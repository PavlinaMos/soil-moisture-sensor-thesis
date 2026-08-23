#pragma once

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "arduino_secrets.h"

const char DEVICE_LOGIN_NAME[] = "7d40106f-e389-4d29-b789-afef2033ef1f";
const char DEVICE_KEY[] = SECRET_DEVICE_KEY;

float rp;
float vmag1;

inline void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(rp, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(vmag1, READ, 10 * SECONDS, NULL);
}
