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
  Console.logFreeMemory();
  Watchdog.enable(60 * 1000);
  Pins.setup();
  Config.load();
  Console.logFreeMemory();
  Gps.setup();
  Mqtt.setup();
  Can.setup();
  Console.logFreeMemory();
}

void loop() {
#ifdef ARDUINO_SAMD_MKR1000
  if (!Wifi.isConnected()) {
    Wifi.connect();
    Console.logFreeMemory();
  }
#elif defined(ARDUINO_SAMD_MKRNB1500)
  if (!Cellular.isConnected()) {
    Cellular.connect();
  }
#endif
  if (!Mqtt.isConnected()) {
    Mqtt.connect();
    Console.logFreeMemory();
  }
  Mqtt.poll();
  Gps.poll();
  Watchdog.reset();
  delay(1);
  if (millis() % 1000 == 0) {
    Console.logFreeMemory();
  }
}