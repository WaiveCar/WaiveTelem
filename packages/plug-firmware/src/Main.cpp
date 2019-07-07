#include <Arduino.h>

#include "Config.h"
#include "Console.h"
#include "Gps.h"
#include "Mqtt.h"
#include "Pins.h"
#ifdef ARDUINO_SAMD_MKR1000
#include "Wifi.h"
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include "Cellular.h"
#endif

void setup() {
  Console.setup();
  Serial.println("Version: " + String(VERSION));
  Pins.setup();
  Gps.setup();
  Config.load();
  Mqtt.setup();
}

void loop() {
#ifdef ARDUINO_SAMD_MKR1000
  if (!Wifi.isConnected()) {
    Wifi.connect();
  }
#elif defined(ARDUINO_SAMD_MKRNB1500)
  if (!Cellular.isConnected()) {
    Cellular.connect();
  }
#endif
  if (!Mqtt.isConnected()) {
    Mqtt.connect();
  }
  Mqtt.poll();
  Gps.poll();
}