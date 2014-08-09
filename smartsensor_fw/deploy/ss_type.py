#!/usr/bin/env python

from array import array

gameModeChannel = { "description": "The game mode channel.",
        "type": 0xFE,
        "additional": array('B', [])
      }

allDescriptors = {
  "digital-in": {
    "description": "This is a digital sensor.  It can be used to get the " + \
                   "state of up to four switches.",
    "chunksNumer": 0xFF,  # TODO(cduck): What is this?
    "chunksDenom": 0xFF,
    "channels": [
      gameModeChannel,
      { "description": "This is the only digital channel.",
        "type": 0x00,
        "additional": array('B', [0x01, 0xFF, 0xFF])
      },
    ]
  },

  "analog-in": {
    "description": "This is a digital sensor.  It can be used to get the " + \
                   "state of up to four switches.",
    "chunksNumer": 0xFF,
    "chunksDenom": 0xFF,
    "channels": [
      gameModeChannel,
      { "description": "This is the first analog channel.",
        "type"  : 0x01,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF])
        # "additional" value should also contain n bytes of calibration data
      },
      { "description": "This is the second analog channel.",
        "type"  : 0x01,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF])
        # "additional" value should also contain n bytes of calibration data
      },
      { "description": "This is the third analog channel.",
        "type"  : 0x01,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF])
        # "additional" value should also contain n bytes of calibration data
      },
      { "description": "This is the fourth analog channel.",
        "type"  : 0x01,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF])
        # "additional" value should also contain n bytes of calibration data
      },
    ]
  },

  "grizzly": {
    "description": "This is a digital sensor.  It can be used to get the " + \
                  "state of up to four switches.",
    "chunksNumer": 0xFF,
    "chunksDenom": 0xFF,
    "channels": [
      # TODO (tobinsarah): get this from Casey's grizzly protocol
      gameModeChannel,
      { "description": "This is the only grizzly channel.",
        "type": 0x80,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF, 0xFF])
      },
    ]
  },

  "buzzer": {
    "description": "This is a battery buzzer. For now its channel has the " + \
                   "same format as analog in.",
    "chunksNumer": 0xFF,
    "chunksDenom": 0xFF,
    "channels": [
      gameModeChannel,
      { "description": "This is the only buzzer channel.",
        "type": 0x81,
        "additional": array('B', [0xFF, 0xFF, 0xFF, 0xFF])
        # "additional" value should also contain n bytes of calibration data
      },
    ]
  },

  "flag": {
    "description": "This is a team flag. For now its channel has the same " + \
                   "format as digital.",
    "chunksNumer": 0xFF,  # TODO(cduck): What is this?
    "chunksDenom": 0xFF,
    "channels": [
      gameModeChannel,
      { "description": "This is the only digital channel.",
        "type": 0x82,
        "additional": array('B', [0x01, 0xFF, 0xFF])
      },
    ]
  }
}

def allTypes():
  return allDescriptors.keys()
def descriptor(sensorConfig):
  return allDescriptors[sensorConfig]

