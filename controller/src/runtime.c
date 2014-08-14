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

// Interpreter
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>

// Interpreter runtime
#include <inc/mboxlib.h>
#include <inc/runtime_entry.h>

#include "inc/runtime.h"
#include "inc/runtime_interface.h"

#include "inc/button_driver.h"
#include "inc/led_driver.h"
#include "inc/task.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"

typedef struct {
  RuntimeMessageType type;
  void *info;

  SSState *sensor;
  uint16_t sensorIndex;
  uint8_t isSensor;
} RuntimeMessage;

QueueHandle_t messageQueue = NULL;

volatile int gameMode = RuntimeModeUninitialized;


void sensorUpdateCallback(uint16_t index, SSState *sensor);

void setLedError(int err);

static portTASK_FUNCTION_PROTO(runtimeTask, pvParameters);



BaseType_t runtimeInit() {
  messageQueue = xQueueCreate(100, sizeof(RuntimeMessage));

  // Get updates from smart sensor protocol
  registerSensorUpdateCallback(&sensorUpdateCallback);

  return xTaskCreate(runtimeTask, "Runtime", 2048, NULL, tskIDLE_PRIORITY,
                     NULL);
}

void runtimeSendRadioMsg(RuntimeMessageType type, void* info) {
  RuntimeMessage msg = {
    .type = type,
    .info = info,
    .isSensor = 0
  };
  xQueueSend(messageQueue, &msg, 0);
}

void sensorUpdateCallback(uint16_t index, SSState *sensor) {
  RuntimeMessage msg = {
    .isSensor = 1,
    .sensorIndex = index,
    .sensor = sensor,
    .type = RuntimeMessageNone,
    .info = NULL
  };
  xQueueSend(messageQueue, &msg, 0);
}

void setLedError(int err) {
  led_driver_set_mode(PATTERN_RUNTIME_ERROR);
  led_driver_set_fixed(err, 0);  // Press button 0 to view this error code
}


static portTASK_FUNCTION_PROTO(runtimeTask, pvParameters) {
  (void) pvParameters;

  int ret = RUNTIME_OK, firstErr = RUNTIME_OK;
  TenshiActorState a;
  const char studentcode[] =
    "sensor = get_device('stestsensor')\n"
    "actuator = get_device('atestactuator')\n"
    "sensor_sampled = triggers.sampled(sensor)\n"
    "\n"
    "while true do\n"
    "    local val = sensor_sampled:recv()\n"
    "    print('sensor is ' .. tostring(val))\n"
    "    actuator:send({val})\n"
    "end";

  // Initialization
  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(0b1110, 0b111);

  // Init runtime
  TenshiRuntimeState s = TenshiRuntimeInit();

  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(0b1101, 0b111);

  TenshiRegisterCFunctions(s, runtime_register);

  // Wait for sensor enumeration
  ssBlockUntilActive();  // TODO(cduck): Should respond to radio during enum.

  if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
  if (firstErr != RUNTIME_OK )led_driver_set_fixed(firstErr, 0b111);

  ret = LoadStudentcode(s, studentcode, strlen(studentcode), &a);
  if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
  if (firstErr != RUNTIME_OK )led_driver_set_fixed(firstErr, 0b111);

  ret = ActorSetRunnable(a, 1);
  if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
  if (firstErr != RUNTIME_OK )led_driver_set_fixed(firstErr, 0b111);

  gameMode = RuntimeModeTeleop;

  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(0b1011, 0b111);

  int i = 0;
  while (1) {
    RuntimeMessage msg;
    while (xQueueReceive(messageQueue, &msg, 0) == pdTRUE) {
      if (msg.isSensor) {
        SSState *sensor = msg.sensor;

        led_driver_set_mode(PATTERN_JUST_RED);
        led_driver_set_fixed(0b1001, 0b111);

        for (int c = 0; c < sensor->channelsNum; c++) {
          SSChannel *channel = sensor->channels[c];
          if (!channel->isProtected) {
            TenshiFlagSensor(s, channel);
          }
        }
      } else {
        // TODO(cduck): Handle radio input
        led_driver_set_mode(PATTERN_JUST_RED);
        led_driver_set_fixed(0b1010, 0b111);
      }
    }

    ret = TenshiRunQuanta(s);
    if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
    if (firstErr != RUNTIME_OK )led_driver_set_fixed(firstErr, 0b111);

    led_driver_set_mode(PATTERN_JUST_RED);
    led_driver_set_fixed(0b1100, 0b111);

    i++;
  }

  TenshiRuntimeDeinit(s);
}
