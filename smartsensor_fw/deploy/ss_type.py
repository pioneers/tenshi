#!/usr/bin/env python

from array import array

allDescriptors = {
  "digital-in": {
    "description": "This is a digital sensor.  It can be used to get the " + \
                   "state of up to four switches.",
    "chunksNumer": 0xFF,  # TODO(cduck): What is this?
    "chunksDenom": 0xFF,
    "channels": [
      { "description": "This is the only channel.",
        "type": 0x00,
        "additional": array('B', [0x01, 0xFF, 0xFF])
      },
    ]
  }
}

def allTypes():
  return allDescriptors.keys()
def descriptor(sensorConfig):
  return allDescriptors[sensorConfig]

