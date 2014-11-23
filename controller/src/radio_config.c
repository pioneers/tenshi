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
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/runtime.h"

size_t data_size = sizeof(uint8_t) + sizeof(TickType_t) + sizeof(uint32_t);
size_t device_list_data_size = sizeof(uint8_t) + sizeof(uint32_t);
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
  if (ssIsActive() && device_list_data_size == sizeof(uint8_t) + sizeof(uint32_t)){
    device_list_data_size += SMART_ID_LEN*numSensors;
  }
  config_port *port = pvPortMalloc(device_list_data_size);
  port->id = ID_DEVICE_GET_LIST;
  if (!ssIsActive()) {
    port->data.device_list.count = 0;
    return port;
  }

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

config_port *getValueUpdate() {
  config_port *port;
  if (ssIsActive()) {
    uint8_t total_number_of_channels = 0;
    for (int i = 0; i < sizeof(sensorArr); i++) {
      total_number_of_channels += sizeof(sensorArr[i]->channels);
    }
    
    if (data_size == sizeof(uint8_t) + sizeof(TickType_t) + sizeof(uint32_t)){
      data_size = sizeof(uint8_t) + sizeof(TickType_t) 
                                  + sizeof(uint32_t) 
                                  + numSensors*SMART_ID_LEN + numSensors*sizeof(uint32_t) 
                                  + total_number_of_channels*sizeof(channel_value);
    }
    port = pvPortMalloc(data_size);
    port->id = ID_DEVICE_VALUE_UPDATE;
    port->data.device_value_update.timestamp = xTaskGetTickCount();
    port->data.device_value_update.count = numSensors;
    for (int i = 0; i < numSensors; i++) {
      uint64_t temp = 0;
      for (int j = SMART_ID_LEN-1; j > 0; j--) {
        temp = ((temp << 8) | sensorArr[i]->id[j]);
      }
      port->data.device_value_update.devices[i].did = temp;
      port->data.device_value_update.devices[i].count =  sensorArr[i]->channelsNum;
      for (int k = 0; k < sensorArr[i]->channelsNum; k++){ 
                 port->data.device_value_update.devices[i].values[k].value = ss_get_generic_value(sensorArr[i]->channels[k]) ;
      }
  }}
  else {
    port = pvPortMalloc(data_size);
    port->id = ID_DEVICE_VALUE_UPDATE;
    port->data.device_value_update.timestamp = xTaskGetTickCount();
    port->data.device_value_update.count = 0;
  }
  return port;
}

static portTASK_FUNCTION_PROTO(radioConfigTask, pvParameters) {
  (void) pvParameters;
  int send_messages_toggle = 0;
  TickType_t time_offset = 100;
  TickType_t time_difference = 0;
  TickType_t next_time_stamp = 0;
  TickType_t wait_time = portMAX_DELAY;

  while (1) {
    ConfigMessage msg;
    if (send_messages_toggle == 1){
      TickType_t current_time_stamp = xTaskGetTickCount();
      time_difference = current_time_stamp - next_time_stamp;
      if (time_difference > time_offset) // if time_difference (unsigned) is negative, hence, super big
      {
        wait_time = next_time_stamp - current_time_stamp;
        next_time_stamp = current_time_stamp + time_offset;
      }
      else {wait_time = 1;} // Just a really small wait time so that the signal can be sent almost immediately
    }
    while (xQueueReceive(configMessageQueue, &msg, wait_time) == pdTRUE) {
      switch (msg.port->id) { 
        case ID_START_VM:
          break;
        case ID_STOP_VM:
          break;
        case ID_LOAD_MAIN_THREAD:
          break;
        case ID_CONTROL_UNFREEZE:
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
        case ID_DEVICE_READ_DESCRIPTOR: {
          uint64_t sensor_id = msg.port->data.device_read_descriptor_req.did;

          for(int i=0; i < numSensors; i++){
            uint64_t temp_id = 0;
            for (int j = SMART_ID_LEN-1; j > 0; j--) {
              temp_id = ((temp_id << 8) | sensorArr[i]->id[j]);
            }
            if(temp_id == sensor_id){
              uint8_t *sensor_descriptor = malloc(sensorArr[i]->totalDescriptorLength);
              ss_make_descriptor(sensorArr[i], sensor_descriptor);

              size_t response_size = sizeof(uint8_t) + SMART_ID_LEN + sensorArr[i]->totalDescriptorLength;
              config_port *response = malloc(response_size);

              response->id = ID_DEVICE_READ_DESCRIPTOR;
              response->data.device_read_descriptor_resp.did = sensor_id;
              memcpy(response->data.device_read_descriptor_resp.data, sensor_descriptor, sensorArr[i]->totalDescriptorLength);

              radioPushConfig(response, response_size);
              break;
            }
          }
          break;
        }
        case ID_DEVICE_ENABLE_BLINK:
          break;

        case ID_DEVICE_START_UPDATES:
            send_messages_toggle = 1;
	    TickType_t current_time_stamp = xTaskGetTickCount();
            TickType_t next_time_stamp = time_offset + current_time_stamp;
            break;
        case ID_DEVICE_STOP_UPDATES:
            send_messages_toggle = 0;
          break;
        case ID_DEVICE_VALUE_UPDATE: {
          config_port *device_value_update = getValueUpdate();
          radioPushConfig(device_value_update, data_size);
          }
          break;

        default:
          if (send_messages_toggle == 1) {
            config_port *device_value_update = getValueUpdate();
            radioPushConfig(device_value_update, data_size);
            break;
          }
      }
      vPortFree(msg.port);
    }
  }
}

