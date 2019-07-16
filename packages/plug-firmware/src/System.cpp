#include <Arduino.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>
#include <SD.h>

#include "Config.h"
#include "Console.h"
#include "Gps.h"
#include "Http.h"
#include "Internet.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void SystemClass::setup() {
  lastHeartbeat = 0;
  canStatusChanged = false;
  statusDoc.createNestedObject("can");
  statusDoc.createNestedObject("heartbeat");
  statusDoc["heartbeat"].createNestedObject("gps");
}

void SystemClass::poll() {
  bool inRide = statusDoc["inRide"] == "true";
  int interval = Config.get()["heartbeat"][inRide ? "inRide" : "notInRide"];
  if (Gps.getLatitude() != 0 &&
      (lastHeartbeat == 0 ||
       (interval >= 0 && millis() - lastHeartbeat > (uint32_t)interval * 1000))) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
}

void SystemClass::sendVersion() {
  statusDoc["firmware"] = FIRMWARE_VERSION;
  String version = "{\"inRide\": \"false\", \"firmware\": \"" + String(FIRMWARE_VERSION) + "\"}";
  Mqtt.telemeter(version);
}

void SystemClass::sendHeartbeat() {
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["time"] = Gps.getTime();
  // return "\"freeMemory\": " + String(freeMemory()) + ", \"signalStrength\": \"" + Internet.getSignalStrength() + "\"";
  heartbeat["freeMemory"] = freeMemory();
  heartbeat["signalStrength"] = Internet.getSignalStrength();
  Mqtt.telemeter(statusDoc["heartbeat"].as<String>());
  File writeFile = SD.open("STATUS.TXT", FILE_WRITE);
  if (!writeFile) {
    Serial.println("STATUS.TXT open failed");
    return;
  }
  String json = statusDoc.as<String>();
  logLine("json: " + json);
  writeFile.write(json.c_str(), json.length());
  writeFile.close();
}

void SystemClass::sendCanStatus() {
  if (canStatusChanged) {
    Mqtt.telemeter(statusDoc["can"].as<String>());
    canStatusChanged = false;
  }
}

void SystemClass::setCanStatus(const String& name, const uint64_t value) {
  JsonObject can = statusDoc["can"];
  if (can[name] != value) {
    canStatusChanged = true;
    can[name] = value;
  }
}

void SystemClass::processCommand(const String& json) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("Failed to read json: " + String(error.c_str()));
    return;
  }

  JsonObject desired = doc["state"];
  if (desired["doors"] == "unlocked") {
    Pins.unlockDoors();
    // CAN-BUS should update
    // Mqtt.telemeter("{\"doors\": \"unlocked\"}");
  } else if (desired["doors"] == "locked") {
    Pins.lockDoors();
    // CAN-BUS should update
    // Mqtt.telemeter("{\"doors\": \"locked\"}");
  } else if (desired["immobilized"] == "true") {
    Pins.immobilize();
    statusDoc["immobilized"] = "true";
    Mqtt.telemeter("{\"immobilized\": \"true\"}");
  } else if (desired["immobilized"] == "false") {
    Pins.unimmobilize();
    statusDoc["immobilized"] = "false";
    Mqtt.telemeter("{\"immobilized\": \"false\"}");
  } else if (desired["inRide"] == "true") {
    statusDoc["inRide"] = "true";
    Mqtt.telemeter("{\"inRide\": \"true\"}");
  } else if (desired["inRide"] == "false") {
    statusDoc["inRide"] = "false";
    Mqtt.telemeter("{\"inRide\": \"false\"}");
  } else if (!desired["download"].isNull() && desired["firmware"] != FIRMWARE_VERSION) {
    String host = desired["download"]["host"];
    String file = desired["download"]["file"];
    Http.download(host, file);
  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(json);
  }
}

SystemClass System;
