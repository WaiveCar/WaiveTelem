#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

void setup() {
  Serial.begin(115200);
#if DEBUG
  // the following cause cause the firmware to only run if serial-monitored
  delay(5000);
#endif
  logFunc();
  Watchdog.enable(WATCHDOG_TIMEOUT);
  Pins.setup();
  Logger.setup();
  Config.load();
  Mqtt.setup();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#else
  Internet.connect();
#endif
  System.keepTime();
  Gps.setup();
  Bluetooth.setup();
  Can.setup();
  System.setup();
  System.sendInfo();
}

void loop() {
  Watchdog.enable(WATCHDOG_TIMEOUT);
  if (!System.getStayAwake()) {
    System.sleep(1);
  }
  System.keepTime();
#ifdef ARDUINO_SAMD_MKR1000
  Mqtt.poll();
#endif
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
