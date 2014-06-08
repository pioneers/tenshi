#!/usr/bin/env python

from __future__ import print_function
import eagle_util_funcs
import os.path
import subprocess
import sys
try:
    import yaml
except ImportError:
    print('Please install PyYaml')
    sys.exit(1)


def generate_gerber(brdFile, outFile, layers):
    ret = eagle_util_funcs.run_eagle([
        '-X',               # Run the command line CAM processor
        '-N',               # No output messages
        '-dGERBER_RS274X',  # Output in gerber format
        '-o' + outFile,     # Output path
        brdFile,            # Input board
        ] + layers          # Layers to process
        )

    if ret != 0:
        print("Eagle returned error!")
        sys.exit(ret)


def generate_drill(brdFile, outFile, layers):
    ret = eagle_util_funcs.run_eagle([
        '-X',               # Run the command line CAM processor
        '-N',               # No output messages
        '-dEXCELLON',       # Output in gerber format
        '-o' + outFile,     # Output path
        brdFile,            # Input board
        ] + layers          # Layers to process
        )

    if ret != 0:
        print("Eagle returned error!")
        sys.exit(ret)


def zip_up_results(outfile):
    subprocess.call("zip %s *" % outfile, shell=True)
    print("Made zip!")


def main():
    if len(sys.argv) < 4:
        print("Usage: %s config.yaml in.brd out.zip" % (sys.argv[0]))
        sys.exit(1)

    configFileName = os.path.abspath(sys.argv[1])
    inputFileName = os.path.abspath(sys.argv[2])
    outputFileName = os.path.abspath(sys.argv[3])

    # Get the "base" (no path, no extension) part of the input name
    inputBaseName = os.path.splitext(os.path.basename(inputFileName))[0]

    # Read configuration
    configFile = open(configFileName, 'r')
    configData = yaml.load(configFile)
    configFile.close()

    # Create temporary directory
    tempdir = eagle_util_funcs.setup_tmp_dir()

    # Process each section
    for configSection in configData:
        print("Running section %s..." % configSection['description'])
        sectionOutputFilePath = ("%s/%s.%s" %
                                 (tempdir, inputBaseName,
                                  configSection['output_extension']))

        layers = configSection['layers']
        if type(layers) == int:
            layers = str(layers)
        layers = layers.split()

        if configSection['type'] == 'gerber':
            generate_gerber(inputFileName, sectionOutputFilePath, layers)
        elif configSection['type'] == 'excellon':
            generate_drill(inputFileName, sectionOutputFilePath, layers)
        else:
            print("Section ignored, unknown type %s" % configSection['type'])

    # Zip outputs
    zip_up_results(outputFileName)

    # Clean up
    eagle_util_funcs.remove_tmp_dir(tempdir)


if __name__ == '__main__':
    main()
