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
  Can.setup();
  Bluetooth.setup();
  Mqtt.setup();

  System.sendVersion();
}

void loop() {
  Watchdog.reset();
  Gps.poll();
  Mqtt.poll();
  Bluetooth.poll();
  System.poll();
  delay(1);
}
