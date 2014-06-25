
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>
#include "ngl_types.h"   // NOLINT(build/include)

#include "inc/lua_interface.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/led_driver.h"
#include "inc/button_driver.h"

#define LUA_REGISTER(FUNC) lua_register(L, #FUNC, lua_ ## FUNC)


// Assuming the sensor is already locked
void allocOutgoingBytes(SSState sensor, uint8_t requiredLen);
int checkIncomingBytes(SSState sensor, uint8_t requiredLen);



void lua_register_all(lua_State *L) {
  LUA_REGISTER(set_led);
  LUA_REGISTER(get_button);
  LUA_REGISTER(get_digital);
  LUA_REGISTER(set_digital);
}

int lua_set_led(lua_State *L) {
  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(lua_tonumber(L, 1), 0b111);

  int ret = 1;  // 1 means ok.
  lua_pushnumber(L, ret);
  return 1;
}

int lua_get_button(lua_State *L) {
  int button = button_driver_get_button_state(0);

  lua_pushnumber(L, button);
  return 1;
}

int lua_get_digital(lua_State *L) {
  int sensorNum = lua_tonumber(L, 1);

  int result = 0;
  if (numSensors > sensorNum) {
    SSState sensor = sensorArr[sensorNum];
    if (xSemaphoreTake(sensor.lock, SENSOR_WAIT_TIME) == pdTRUE) {
      if (checkIncomingBytes(sensor, 1)) {
        result = sensor.incomingBytes[0];
      }

      xSemaphoreGive(sensor.lock);
    }
  }

  lua_pushnumber(L, result);
  return 1;
}
int lua_set_digital(lua_State *L) {
  int sensorNum = lua_tonumber(L, 1);
  int sensorVal = lua_tonumber(L, 2);

  if (numSensors > sensorNum) {
    SSState sensor = sensorArr[sensorNum];
    if (xSemaphoreTake(sensor.lock, SENSOR_WAIT_TIME) == pdTRUE) {
      allocOutgoingBytes(sensor, 1);
      sensor.outgoingBytes[0] = sensorVal;

      xSemaphoreGive(sensor.lock);
    }
  }

  int ret = 1;  // 1 means ok.
  lua_pushnumber(L, ret);
  return 1;
}




// Assuming the sensor is already locked
void allocOutgoingBytes(SSState sensor, uint8_t requiredLen) {
  if (sensor.outgoingBytes == NULL) {
    sensor.outgoingLen = requiredLen;
    sensor.outgoingBytes = pvPortMalloc(requiredLen);
  }
  if (sensor.outgoingLen != requiredLen) {
    vPortFree(sensor.outgoingBytes);
    sensor.outgoingLen = requiredLen;
    sensor.outgoingBytes = pvPortMalloc(requiredLen);
  }
}
// Assuming the sensor is already locked
int checkIncomingBytes(SSState sensor, uint8_t requiredLen) {
  return (sensor.incomingBytes != NULL) && (sensor.incomingLen >= requiredLen);
}
