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

void setup() {
  Watchdog.enable(16 * 1000);
  Console.setup();
  Pins.setup();
  Config.load();
  Gps.setup();
  Mqtt.setup();
  Can.setup();
  Bluetooth.setup();
  System.sendVersion();
}

void loop() {
  Watchdog.reset();
  Mqtt.poll();
  Gps.poll();
  Bluetooth.poll();
  delay(1);
}