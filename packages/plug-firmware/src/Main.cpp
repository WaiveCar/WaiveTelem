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
  Config.load();
  Pins.setup();
  Gps.setup();
  Mqtt.setup();
}

void loop() {
#ifdef ARDUINO_SAMD_MKR1000
  if (!Wifi.isConnected()) {
    Wifi.connect();
  }
#endif
  if (!Mqtt.isConnected()) {
    Mqtt.connect();
  }
  Mqtt.poll();
  delay(5000);
  Mqtt.publish("arduino/outgoing", "{}");
}