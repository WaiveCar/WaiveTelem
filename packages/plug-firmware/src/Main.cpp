#include <Adafruit_SleepyDog.h>
#include <Arduino.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Console.h"
#include "Gps.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"
#ifdef ARDUINO_SAMD_MKR1000
#include "Wifi.h"
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include "Cellular.h"
#endif

void setup() {
  Watchdog.enable(16 * 1000);
  Console.setup();
  Pins.setup();
  Config.load();
  Gps.setup();
  Mqtt.setup();
  Can.setup();
  Bluetooth.setup();
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
    System.sendVersion();
  }
  Mqtt.poll();
  Gps.poll();
  Bluetooth.poll();
  Watchdog.reset();
}