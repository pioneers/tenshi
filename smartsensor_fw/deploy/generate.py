#!/usr/bin/env python

import sys, getopt, random, struct
from array import array
from intelhex import IntelHex
import ss_type
from crc8 import crc8






def main(argv):
  # Interpret parameters
  sensorConfig = ""
  hexFile = "../../build/artifacts/smartsensor_fw/opt/smartsensor_fw.hex"

  helpText = (argv[0] if len(argv)>0 else "generate.py") + \
    " [-d] [-i <hex-file>] <sensor-type>"
  helpMore = "Supported sensor types:\n\t" + ", ".join(ss_type.allTypes())
  try:
    opts, args = getopt.getopt(argv[1:],"hdi:")
  except getopt.GetoptError:
    print helpText
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print helpText
      print helpMore
      sys.exit()
    elif opt == "-i":
      hexFile = arg
    elif opt == "-d":
      if len(args)>0:
        try:
          ss_type.descriptor(args[0])
          sys.exit(0)
        except KeyError:
          sys.exit(args[0]+" is not a valid sensor type.\n"+helpMore)
      else:
        sys.exit("Enter a sensor type\n"+helpMore)
  if len(args)==0:
    print helpText
    print helpMore
    sys.exit(2)
  else:
    sensorConfig = args[0]

  try:
    descriptor = ss_type.descriptor(sensorConfig)
  except KeyError:
    sys.exit(args[0]+" is not a valid sensor type.\n"+helpMore)



  # Decide the new sensor's ID
  try:
    firstType = descriptor["channels"][0]["type"]
  except IndexError:
    firstType = 0xFF
  # TODO(nikita): Proper ID generation
  idNew = array('B', [0, 0, 0, 0, 0, firstType, random.randint(0,255),
                      random.randint(0,255), firstType])



  # Create the new sensor's descriptor
  channelsStr = b""
  for channel in descriptor["channels"]:
    # Pack each channel
    packer = struct.Struct(
      "<B%upB%us"%(len(channel["description"])+1,len(channel["additional"])))
    channelsStr += packer.pack(
      packer.size, channel["description"], channel["type"],
      channel["additional"].tostring())
  packer = struct.Struct(
    "<H%upBBB%usB"%(len(descriptor["description"])+1,len(channelsStr)))
  descriptorStr = packer.pack(
    packer.size, descriptor["description"], descriptor["chunksNumer"],
    descriptor["chunksDenom"], len(descriptor["channels"]), channelsStr, 0xC8)
  crc = crc8(0, array('B', descriptorStr))
  descriptorStr = packer.pack(
    packer.size, descriptor["description"], descriptor["chunksNumer"],
    descriptor["chunksDenom"], len(descriptor["channels"]), channelsStr, crc)

  descriptorNew = array('B', descriptorStr)




  # Write the ID and descriptor to the hex file

  descriptorPlaceholder = \
  b"Do not change this string.  It is a placeholder that is replaced in the " + \
  b"compiled hex file when the deploy script is run."
  idPlaceholder = "SENSORID"

  ih = IntelHex(hexFile)
  ihDict = ih.todict()
  ihArr = array('B', [ihDict[key] for key in sorted(ihDict)])
  ihString = ihArr.tostring()

  try:
    descriptorIndex = ihArr.tostring().index(descriptorPlaceholder)
  except ValueError:
    sys.exit('Error: Descriptor placeholder not found in hex file.')
  try:
    idIndex = ihArr.tostring().index(idPlaceholder)
  except ValueError:
    sys.exit('Error: ID placeholder not found in hex file.')

  descriptorDict = {descriptorIndex+i: descriptorNew[i] for i in
    range(len(descriptorNew))}
  idDict = {idIndex+i: idNew[i] for i in range(len(idNew))}
  ihDict = dict(ihDict.items()+descriptorDict.items()+idDict.items())

  newIh = IntelHex()
  newIh.fromdict(ihDict)

  newIh.tofile(sys.stdout, format='hex')  # Print new hex file to stdout




if __name__ == "__main__":
  try:
    main(sys.argv)
  except IOError as e:
    exit("IO Error: "+str(e))
