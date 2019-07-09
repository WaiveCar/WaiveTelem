#include <Adafruit_SleepyDog.h>
#include <Arduino.h>

#include "Can.h"
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
  Watchdog.enable(60 * 1000);
  Pins.setup();
  Config.load();
  Gps.setup();
  Mqtt.setup();
  Can.setup();
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
  Watchdog.reset();
  delay(1);
  if (millis() % 1000 == 0) {
    Console.logFreeMemory();
  }
}