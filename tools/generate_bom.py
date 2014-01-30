#!/usr/bin/env python
from __future__ import print_function

import csv
import os
import os.path
import shutil
import subprocess
import sys
import tempfile
import time
try:
    import yaml
except ImportError:
    print('Please install PyYaml')
    sys.exit(1)


def start_xvfb():
    display_num = os.getpid()
    xvfb_process = subprocess.Popen([
        'Xvfb',
        ':%d' % display_num,
        '-screen', '0', '1024x768x24',
        ])
    time.sleep(2)
    print("Spawned Xvfb, display %d, pid %d" %
         (display_num, xvfb_process.pid))
    os.environ["DISPLAY"] = ":%d" % display_num
    return (xvfb_process, display_num)


def kill_xvfb(xvfb_process):
    xvfb_process.kill()
    print("Killed Xvfb")


def setup_tmp_dir():
    tmpdir = tempfile.mkdtemp()
    print("Files in %s" % tmpdir)
    os.chdir(tmpdir)
    return tmpdir


def remove_tmp_dir(tmpdir):
    shutil.rmtree(tmpdir)


def run_eagle_bom_ulp(infile, outfile, ulpfile):
    subprocess.call([
        '/opt/eagle-6.5.0/bin/eagle',
        '-C', 'run %s %s; quit' % (ulpfile, outfile),
        infile,
        ])


def fill_bom_with_db_info(infile, outfile, partDb):
    inF = open(infile, 'r')
    outF = open(outfile, 'w')
    inReader = csv.reader(inF)
    outWriter = csv.writer(outF)

    outWriter.writerow([
        "Reference Designator",
        "Value",
        "Package",
        "Description",
        "Manufacturer",
        "Manufacturer Part Number",
        "Distributor",
        "Distributor Part Number",
        "Notes",
        ])

    for row in inReader:
        refDes, value, package, description, intPartNumber = row
        # print(refDes)
        mfg = ""
        mfgpn = ""
        dist = ""
        distpn = ""
        notes = ""

        if not intPartNumber in partDb:
            print("WARNING: Part %s has unknown internal part number '%s'" % (
                refDes, intPartNumber))
        else:
            partInfo = partDb[intPartNumber]
            packageInfo = None
            if package in partInfo:
                packageInfo = partInfo[package]
            elif '*' in partInfo:
                packageInfo = partInfo['*']
            else:
                print("WARNING: Part %s does not have footprint '%s'" % (
                    refDes, package))

            if packageInfo:
                valueInfo = None
                if value in packageInfo:
                    valueInfo = packageInfo[value]
                elif '*' in packageInfo:
                    valueInfo = packageInfo['*']
                else:
                    print("WARNING: Part %s does not have value '%s'" % (
                        refDes, value))

                if valueInfo:
                    # TODO(rqou): Alternates
                    particularPartInfo = valueInfo['primary_part']

                    mfg = particularPartInfo['mfg']
                    mfgpn = particularPartInfo['mfg_pn']
                    dist = particularPartInfo['distributor']
                    distpn = particularPartInfo['distributor_pn']
                    notes = particularPartInfo['notes']

        outWriter.writerow([
            refDes,
            value,
            package,
            description,
            mfg,
            mfgpn,
            dist,
            distpn,
            notes,
            ])

    inF.close()
    outF.close()


def main():
    if len(sys.argv) < 4:
        print("Usage: %s schematic.sch bom.csv partdb.yaml" % sys.argv[0])
        sys.exit(1)

    ulpFile = os.path.abspath(os.path.dirname(sys.argv[0]) + "/tenshi-bom.ulp")
    schematicPath = os.path.abspath(sys.argv[1])
    outfilePath = os.path.abspath(sys.argv[2])
    partDbFileName = os.path.abspath(sys.argv[3])

    # Load the part database
    partDbFile = open(partDbFileName, 'r')
    partDb = yaml.load(partDbFile)
    partDbFile.close()

    # Start up Xvfb
    xvfb, display_num = start_xvfb()

    # Create temp directory (work around EAGLE running the ULP in BRD instead
    # of in the SCH). Also will chdir into it.
    tmpdir = setup_tmp_dir()
    shutil.copyfile(schematicPath, "input.sch")

    # Run the BOM-generating ULP
    run_eagle_bom_ulp("input.sch", "bom-temp.csv", ulpFile)

    # TODO(rqou): There should probably be some error checking somewhere here

    # Match up stuff with the parts database
    fill_bom_with_db_info('bom-temp.csv', outfilePath, partDb)

    # Cleanup
    remove_tmp_dir(tmpdir)
    kill_xvfb(xvfb)

if __name__ == '__main__':
    main()
