#include <Arduino.h>
#include <String.h>

#include "Config.h"
#include "Console.h"
#include "Gps.h"
#include "Mqtt.h"
#include "Pins.h"

void setup() {
  Console.setup();
  Serial.println("Version: " + String(VERSION));
  Pins.setup();
  Config.load();
  String id = Config.getId();
  log("id: " + id);
}

void loop() {
}