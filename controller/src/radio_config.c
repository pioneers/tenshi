// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inc/radio_config.h"
#include "inc/task.h"
#include "inc/queue.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/runtime.h"

typedef struct {
  config_port *port;
  size_t len;
} ConfigMessage;

QueueHandle_t configMessageQueue = NULL;
// void sensorUpdateCallback(uint16_t index, SSState *sensor);

static portTASK_FUNCTION_PROTO(radioConfigTask, pvParameters);

BaseType_t radioConfigInit() {
  configMessageQueue = xQueueCreate(100, sizeof(ConfigMessage));
  /*
  // Get updates from smart sensor protocol
  registerSensorUpdateCallback(&sensorUpdateCallback);
  */
  return xTaskCreate(radioConfigTask, "Radio Config", 256, NULL,
                     tskIDLE_PRIORITY, NULL);
}

void receiveConfigPort(config_port *port, size_t len) {
  ConfigMessage msg = {
    .port = port,
    .len = len,
  };
  xQueueSend(configMessageQueue, &msg, 0);
}

config_port *getDeviceList() {
  config_port *port = pvPortMalloc(sizeof(uint8_t) + sizeof(uint32_t)
                                                   + SMART_ID_LEN*numSensors);
  port->id = ID_DEVICE_GET_LIST;
  port->data.device_list.count = numSensors;
  for (int i = 0; i < numSensors; i++) {
    uint64_t temp = 0;
    for (int j = SMART_ID_LEN-1; j > 0; j--) {
      temp = ((temp << 8) | sensorArr[i]->id[j]);
    }
    port->data.device_list.dids[i] = temp;
  }
  return port;
}

static portTASK_FUNCTION_PROTO(radioConfigTask, pvParameters) {
  (void) pvParameters;
  while (1) {
    ConfigMessage msg;
    while (xQueueReceive(configMessageQueue, &msg, portMAX_DELAY) == pdTRUE) {
      switch (msg.port->id) {  // TODO(vdonato): Implement the rest
        case ID_START_VM:
          break;
        case ID_STOP_VM:
          break;
        case ID_LOAD_MAIN_THREAD:
          break;
        case ID_CONTROL_UNFREEZE:
          // TODO(vdonato): Figure out what to do
          break;
        case ID_CONTROL_STOP:
          setGameMode(RuntimeModePaused);
          break;
        case ID_CONTROL_UNPOWERED:
          setGameMode(RuntimeModeDisabled);
          break;
        case ID_CONTROL_SET_AUTON:
          setGameMode(RuntimeModeAutonomous);
          break;
        case ID_CONTROL_SET_TELEOP:
          setGameMode(RuntimeModeTeleop);
          break;
        case ID_DEVICE_GET_LIST: {
            config_port *deviceList = getDeviceList();
            size_t size = sizeof(uint8_t) + sizeof(uint32_t) +
                          SMART_ID_LEN*numSensors;
            radioPushConfig(deviceList, size);
          }
          break;
        case ID_DEVICE_READ_DESCRIPTOR:
          break;
        case ID_DEVICE_ENABLE_BLINK:
          break;
        default:
          break;
      }
      vPortFree(msg.port);
    }
  }
}
