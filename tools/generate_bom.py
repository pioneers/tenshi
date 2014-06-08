#!/usr/bin/env python
from __future__ import print_function

import csv
import eagle_util_funcs
import os
import os.path
import shutil
import sys
try:
    import yaml
except ImportError:
    print('Please install PyYaml')
    sys.exit(1)


def run_eagle_bom_ulp(infile, outfile, ulpfile):
    eagle_util_funcs.run_eagle([
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

        if intPartNumber not in partDb:
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

    ulpFile = os.path.abspath(os.path.dirname(__file__) + "/tenshi-bom.ulp")
    schematicPath = os.path.abspath(sys.argv[1])
    outfilePath = os.path.abspath(sys.argv[2])
    partDbFileName = os.path.abspath(sys.argv[3])

    # Load the part database
    partDbFile = open(partDbFileName, 'r')
    partDb = yaml.load(partDbFile)
    partDbFile.close()

    # Start up Xvfb
    xvfb, display_num = eagle_util_funcs.start_xvfb()

    # Create temp directory (work around EAGLE running the ULP in BRD instead
    # of in the SCH). Also will chdir into it.
    tmpdir = eagle_util_funcs.setup_tmp_dir()
    shutil.copyfile(schematicPath, "input.sch")

    # Run the BOM-generating ULP
    run_eagle_bom_ulp("input.sch", "bom-temp.csv", ulpFile)

    # TODO(rqou): There should probably be some error checking somewhere here

    # Match up stuff with the parts database
    fill_bom_with_db_info('bom-temp.csv', outfilePath, partDb)

    # Cleanup
    eagle_util_funcs.remove_tmp_dir(tmpdir)
    eagle_util_funcs.kill_xvfb(xvfb)

if __name__ == '__main__':
    main()
