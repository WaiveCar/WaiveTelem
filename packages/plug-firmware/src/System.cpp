#include <Adafruit_SleepyDog.h>
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
  bool inRide = (statusDoc["inRide"] == "true");
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
  statusDoc["inRide"] = "false";
  String version = "{\"inRide\": \"false\", \"firmware\": \"" + String(FIRMWARE_VERSION) + "\"}";
  Mqtt.telemeter(version);
}

void SystemClass::sendHeartbeat() {
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["time"] = Gps.getTime();
  // heartbeat["freeMemory"] = freeMemory();
  heartbeat["signalStrength"] = Internet.getSignalStrength();
  Mqtt.telemeter(statusDoc["heartbeat"].as<String>());
  File writeFile = SD.open("STATUS.TXT", FILE_WRITE);
  if (!writeFile) {
    Serial.println("STATUS.TXT open failed");
    return;
  }
  writeFile.seek(0);  // workaround BUG in SD to default to append
  String json = statusDoc.as<String>();
  // logLine("json: " + json);
  writeFile.write(json.c_str(), json.length());
  writeFile.close();
  logLine("statusDoc memory cushion: " + String(1024 - statusDoc.memoryUsage()));
  logLine("Free memory: " + String(freeMemory()));
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
  StaticJsonDocument<512> cmdDoc;
  DeserializationError error = deserializeJson(cmdDoc, json);
  if (error) {
    Serial.println("Failed to read json: " + String(error.c_str()));
    return;
  }
  logLine("cmdDoc memory cushion: " + String(512 - cmdDoc.memoryUsage()));

  JsonObject desired = cmdDoc["state"];
  JsonObject download = desired["download"];
  JsonObject copy = desired["copy"];
  if (desired["reboot"] == "true") {
    Mqtt.telemeter("", "{\"reboot\": null}");
    reboot();
  } else if (desired["doors"] == "unlocked") {
    Pins.unlockDoors();
    // CAN-BUS should update
    Mqtt.telemeter("{\"doors\": \"unlocked\"}");
  } else if (desired["doors"] == "locked") {
    Pins.lockDoors();
    // CAN-BUS should update
    Mqtt.telemeter("{\"doors\": \"locked\"}");
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
  } else if (!download.isNull()) {
    const char* host = download["host"] | "";
    const char* from = download["from"] | "";
    const char* to = download["to"] | "";
    if (strlen(host) > 0 && strlen(from) > 0 && strlen(to) > 0) {
      Http.download(host, from, to);
    } else {
      Serial.print(F("Error: "));
      Serial.println(json);
    }
  } else if (!copy.isNull()) {
    const char* from = copy["from"] | "";
    const char* to = copy["to"] | "";
    if (strlen(from) > 0 && strlen(to) > 0) {
      copyFile(from, to);
      Mqtt.telemeter("", "{\"copy\": null}");
      reboot();
    } else {
      Serial.print(F("Error: "));
      Serial.println(json);
    }
  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(json);
  }
}

void SystemClass::reboot() {
  Serial.println(F("Rebooting now"));
  delay(1000);
  Watchdog.enable(1);
  while (true)
    ;
}

int32_t SystemClass::moveFile(const char* from, const char* to) {
  int32_t error = copyFile(from, to);
  if (!error) {
    SD.remove((char*)from);
  }
  return error;
}

int32_t SystemClass::copyFile(const char* from, const char* to) {
  File readFile = SD.open(from, FILE_READ);
  if (!readFile) {
    Serial.println("readFile open failed");
    return -1;
  }
  File writeFile = SD.open(to, FILE_WRITE);
  if (!writeFile) {
    Serial.println("writeFile open failed");
    return -1;
  }
  writeFile.seek(0);  // workaround BUG in SD to default to append
  uint8_t buf[BUFFER_SIZE];
  while (readFile.available()) {
    int bytesRead = readFile.read(buf, sizeof(buf));
    writeFile.write(buf, bytesRead);
    // logLine("write " + String(bytesRead));
    Watchdog.reset();
  }
  readFile.close();
  writeFile.close();
  return 0;
}

SystemClass System;
