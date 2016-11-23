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
#include "inc/queue.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"

typedef struct {
  RuntimeMessageType type;
  void *info;
  size_t infoLen;

  SSState *sensor;
  uint16_t sensorIndex;
  uint8_t isSensor;
} RuntimeMessage;

QueueHandle_t messageQueue = NULL;

volatile int gameMode = RuntimeModeUninitialized;
volatile size_t lastUbjsonLen = 0;
volatile char *lastUbjson = NULL;


void sensorUpdateCallback(uint16_t index, SSState *sensor);

void setLedError(int err);
int restartTenshiRuntime(TenshiRuntimeState *sp, TenshiActorState *ap,
  const char *newCode, size_t newCodeLen);

static portTASK_FUNCTION_PROTO(runtimeTask, pvParameters);



BaseType_t runtimeInit() {
  messageQueue = xQueueCreate(100, sizeof(RuntimeMessage));

  // Get updates from smart sensor protocol
  registerSensorUpdateCallback(&sensorUpdateCallback);

  return xTaskCreate(runtimeTask, "Runtime", 512, NULL, tskIDLE_PRIORITY,
                     NULL);
}

void setGameMode(RuntimeMode mode) {
  gameMode = mode;
  setAllSmartSensorGameMode(mode);

  switch (mode) {
    case RuntimeModeDisabled:
    case RuntimeModePaused:
      led_driver_set_mode(PATTERN_RUNTIME_DISABLED);
      led_driver_set_fixed(led_driver_get_fixed_pattern(), 0);
      break;
    case RuntimeModeAutonomous:
      led_driver_set_mode(PATTERN_RUNTIME_AUTONOMOUS);
      led_driver_set_fixed(led_driver_get_fixed_pattern(), 0);
      break;
    case RuntimeModeTeleop:
      led_driver_set_mode(PATTERN_RUNTIME_TELEOP);
      led_driver_set_fixed(led_driver_get_fixed_pattern(), 0);
      break;
    default: break;
  }
}

void runtimeSendRadioMsg(RuntimeMessageType type, void* info, size_t infoLen) {
  RuntimeMessage msg = {
    .type = type,
    .info = info,
    .infoLen = infoLen,
    .isSensor = 0,
  };
  xQueueSend(messageQueue, &msg, 0);
}
// Takes responsibility for freeing
void runtimeRecieveUbjson(char *ubjson, size_t len) {
  runtimeSendRadioMsg(RuntimeMessageUbjson, ubjson, len);
}
// Takes responsibility for freeing
void runtimeRecieveCode(char *code, size_t len) {
  runtimeSendRadioMsg(RuntimeMessageNewCode, code, len);
}
char *readLastUbjson(size_t *len) {
  *len = lastUbjsonLen;
  return lastUbjson;
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
  setGameMode(RuntimeModeDisabled);
  led_driver_set_mode(PATTERN_RUNTIME_ERROR);
  led_driver_set_fixed(err, 0);  // Press button 0 to view this error code
}

int restartTenshiRuntime(TenshiRuntimeState *sp, TenshiActorState *ap,
  const char *newCode, size_t newCodeLen) {
  int ret = RUNTIME_OK, firstErr = RUNTIME_OK;

  if (*sp) TenshiRuntimeDeinit(*sp);

  // Init runtime
  TenshiRuntimeState s = TenshiRuntimeInit();
  TenshiActorState a;

  runtime_register(s);

  ret = LoadStudentcode(s, newCode, newCodeLen, &a);
  if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;

  if (a) {
    ret = ActorSetRunnable(a, 1);
    if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
  }

  *sp = s;
  *ap = a;
  return firstErr;
}

/*
print('Robot on...')
motor1=get_device('00000000008041DD')
print(motor1)
*/

const char studentCodeTemplate[] =
  "print('Robot on.')\n"
  // "motor1=get_device('00000000008041DD')\n"
  // "print(motor1)\n"
  // "print(send)\n"
  // "send(motor1, 50)\n"
  // "print('Sent.')\n"
  "while true do\n"
  // "  recv({timeout=10})\n"
  // "  send(motor1, 50)\n"
  // "  print('While.')\n"
  "end\n";
//  "while true do\n"
//  "  recv({timeout=10})\n"
//  "  send(motor1, 50)\n"
//  "  print('While.')\n"
//  "end\n";
//  "sensor = get_device('stestsensor')\n"
//  "actuator = get_device('atestactuator')\n"
//  "sensor_sampled = triggers.sampled(sensor)\n"
//  "\n"
//  "while true do\n"
//  "    local val = sensor_sampled:recv()\n"
//  "    print('sensor is ' .. tostring(val))\n"
//  "    actuator:send({val})\n"
//  "end";

static portTASK_FUNCTION_PROTO(runtimeTask, pvParameters) {
  (void) pvParameters;

  int ret = RUNTIME_OK, firstErr = RUNTIME_OK;
  TenshiRuntimeState s = NULL;
  TenshiActorState a = NULL;
  char *studentCode = malloc(sizeof(studentCodeTemplate));
  memcpy(studentCode, studentCodeTemplate, sizeof(studentCodeTemplate));
  size_t studentCodeLen = sizeof(studentCodeTemplate)-1;

  // Wait for sensor enumeration
  ssBlockUntilActive();  // TODO(cduck): Should respond to radio during enum.

  ret = restartTenshiRuntime(&s, &a, studentCode, studentCodeLen);
  if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
  if (firstErr != RUNTIME_OK) {
    setLedError(firstErr);
  }

  setGameMode(RuntimeModeDisabled);

  int i = 0;
  while (1) {
    RuntimeMessage msg;
    while (xQueueReceive(messageQueue, &msg, 0) == pdTRUE) {
      if (msg.isSensor) {
        SSState *sensor = msg.sensor;

        for (int c = 0; c < sensor->channelsNum; c++) {
          SSChannel *channel = sensor->channels[c];
          if (!channel->isProtected && a != NULL && s != NULL) {
            TenshiFlagSensor(s, channel);
          }
        }
      } else {
        switch (msg.type) {
          case RuntimeMessageUbjson:
            free(lastUbjson);
            lastUbjson = msg.info;
            lastUbjsonLen = msg.infoLen;
            break;
          case RuntimeMessageNewCode:
            free(studentCode);
            studentCode = msg.info;
            studentCodeLen = msg.infoLen;

            // Restart tenshi runtime and reset error code
            firstErr = RUNTIME_OK;
            led_driver_set_fixed(0b0000, 0);

            printf("Reloading code.\n");
            restartTenshiRuntime(&s, &a, studentCode, studentCodeLen);
            if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
            if (firstErr != RUNTIME_OK) {
              setLedError(firstErr);
            }

            firstErr = RUNTIME_OK;
            ret = RUNTIME_OK;
            led_driver_set_fixed(0b0000, 0);

            led_driver_set_mode(PATTERN_RUNTIME_ERROR);
            vTaskDelay(500 / portTICK_RATE_MS);

            setGameMode(RuntimeModeDisabled);
            break;
          default:
            free(msg.info);
            break;
        }
      }
    }

    if (firstErr == RUNTIME_OK && a != NULL && s != NULL) {
      ret = TenshiRunQuanta(s);
      if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
      if (firstErr != RUNTIME_OK) {
        setLedError(firstErr);
      }
    }
    if (button_driver_get_button_state(1)) {
      RuntimeMode newMode;
      int shouldRestart = 0;
      if (gameMode >= RuntimeModeTeleop) {
        newMode = RuntimeModeDisabled;
        shouldRestart = 1;
      } else {
        newMode = gameMode + 1;
      }
      if (firstErr != RUNTIME_OK) shouldRestart = 1;

      // Restart tenshi runtime and reset error code
      firstErr = RUNTIME_OK;
      led_driver_set_fixed(0b0000, 0b111);

      setGameMode(newMode);

      if (shouldRestart) {
        ret = restartTenshiRuntime(&s, &a, studentCode, studentCodeLen);
        if (ret != RUNTIME_OK && firstErr == RUNTIME_OK) firstErr = ret;
        if (firstErr != RUNTIME_OK) {
          setLedError(firstErr);
        }

        printf("Restarting code.\nNew mode: %d\n", gameMode);
      } else {
        printf("New mode: %d\n", gameMode);
      }

      while (button_driver_get_button_state(1)) {}
    }

    i++;
  }

  if (s) TenshiRuntimeDeinit(s);
}
