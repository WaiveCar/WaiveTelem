#include <Arduino.h>
#include <ArduinoJson.h>

#include "Console.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void SystemClass::setup() {
  inRide = false;
}

void SystemClass::sendVersion() {
  String version = "{\"inRide\": \"false\", \"firmware\": \"" + String(VERSION) + "\"}";
  Mqtt.telemeter(version);
}

void SystemClass::setInRide(bool in) {
  inRide = in;
}

bool SystemClass::getInRide() {
  return inRide;
}

String& SystemClass::getStatus() {
  status = "\"freeMemory\": " + String(freeMemory());
  return status;
}

void SystemClass::processCommand(String& json) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("Failed to read json: " + String(error.c_str()));
    return;
  }

  JsonObject stateDoc = doc["state"];
  if (stateDoc["doors"] == "unlocked") {
    Pins.unlockDoors();
    // CAN-BUS should update
    // Mqtt.telemeter("{\"doors\": \"unlocked\"}");
  } else if (stateDoc["doors"] == "locked") {
    Pins.lockDoors();
    // CAN-BUS should update
    // Mqtt.telemeter("{\"doors\": \"locked\"}");
  } else if (stateDoc["immobilized"] == "true") {
    Pins.immobilize();
    Mqtt.telemeter("{\"immobilized\": \"true\"}");
  } else if (stateDoc["immobilized"] == "false") {
    Pins.unimmobilize();
    Mqtt.telemeter("{\"immobilized\": \"false\"}");
  } else if (stateDoc["inRide"] == "true") {
    System.setInRide(true);
    Mqtt.telemeter("{\"inRide\": \"true\"}");
  } else if (stateDoc["inRide"] == "false") {
    System.setInRide(false);
    Mqtt.telemeter("{\"inRide\": \"false\"}");
  } else if (System.getInRide() && stateDoc["firmware"] != String(VERSION)) {
  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(json);
  }
}

SystemClass System;