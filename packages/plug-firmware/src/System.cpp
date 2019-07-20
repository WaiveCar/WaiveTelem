#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>

#include "Config.h"
#include "Gps.h"
#include "Http.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#define COMMAND_DOC_SIZE 512

extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void SystemClass::setup() {
  statusDoc.createNestedObject("can");
  statusDoc.createNestedObject("heartbeat");
  statusDoc["heartbeat"].createNestedObject("gps");
  statusDoc["heartbeat"].createNestedObject("system");

  sendVersion();
}

void SystemClass::poll() {
  bool inRide = (statusDoc["inRide"] == "true");
  int interval = Config.get()["heartbeat"][inRide ? "inRide" : "notInRide"];
  if (interval > 0 && Gps.getLatitude() != 0 &&
      (lastHeartbeat == 0 || getMillis() - lastHeartbeat > (uint32_t)interval * 1000)) {
    sendHeartbeat();
    lastHeartbeat = getMillis();
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
  JsonObject system = heartbeat["system"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["time"] = Gps.getTime();
  system["signalStrength"] = Internet.getSignalStrength();
  system["heapFreeMem"] = freeMemory();
  system["statusFreeMem"] = STATUS_DOC_SIZE - statusDoc.memoryUsage();
  Mqtt.telemeter(statusDoc["heartbeat"].as<String>());
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
  StaticJsonDocument<COMMAND_DOC_SIZE> cmdDoc;
  DeserializationError error = deserializeJson(cmdDoc, json);
  if (error) {
    logError("Failed to read json: " + String(error.c_str()));
    return;
  }
  JsonObject desired = cmdDoc["state"] | cmdDoc.as<JsonObject>();  // mqtt form | ble form
  JsonObject download = desired["download"];
  JsonObject copy = desired["copy"];
  if (desired["reboot"] == "true") {
    Mqtt.telemeter("", "{\"reboot\": null}");
    reboot();
  } else if (desired["lock"] == "open") {
    Pins.unlockDoors();
    // CAN-BUS should update
    Mqtt.telemeter("{\"lock\": \"open\"}");
  } else if (desired["lock"] == "close") {
    Pins.lockDoors();
    // CAN-BUS should update
    Mqtt.telemeter("{\"lock\": \"close\"}");
  } else if (desired["immo"] == "lock") {
    Pins.immobilize();
    statusDoc["immo"] = "lock";
    Mqtt.telemeter("{\"immo\": \"lock\"}");
  } else if (desired["immo"] == "unlock") {
    Pins.unimmobilize();
    statusDoc["immo"] = "unlock";
    Mqtt.telemeter("{\"immo\": \"unlock\"}");
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
      logError("Error: " + json);
    }
  } else if (!copy.isNull()) {
    const char* from = copy["from"] | "";
    const char* to = copy["to"] | "";
    if (strlen(from) > 0 && strlen(to) > 0) {
      copyFile(from, to);
      Mqtt.telemeter("", "{\"copy\": null}");
      reboot();
    } else {
      logError("Error: " + json);
    }
  } else {
    logError("Unknown command: " + json);
  }
}

uint64_t SystemClass::getMillis() {
  return millis;
}

void SystemClass::kickWatchdogAndSleep(int msec) {
  digitalWrite(PIN_A1, HIGH);
#if DEBUG
  // don't use Watchdog.sleep as it disconnects USB
  Watchdog.reset();
  delay(msec);
#else
  Watchdog.sleep(msec);  // might return early because of external interrupt
#endif
  millis += msec;
  digitalWrite(PIN_A1, LOW);
  Watchdog.enable(16 * 1000);
}

void SystemClass::reboot() {
  logInfo(F("Rebooting now"));
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
    logError("readFile open failed");
    return -1;
  }
  File writeFile = SD.open(to, FILE_WRITE);
  if (!writeFile) {
    logError("writeFile open failed");
    return -1;
  }
  writeFile.seek(0);  // workaround BUG in SD to default to append
  uint8_t buf[BUFFER_SIZE];
  while (readFile.available()) {
    int bytesRead = readFile.read(buf, sizeof(buf));
    writeFile.write(buf, bytesRead);
    // logDebug("write " + String(bytesRead));
    Watchdog.reset();
  }
  readFile.close();
  writeFile.close();
  return 0;
}

SystemClass System;
