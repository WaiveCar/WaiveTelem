#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>

#include "Config.h"
#include "Gps.h"
#include "Https.h"
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

  sendInfo();
}

void SystemClass::poll() {
  bool inRide = (statusDoc["inRide"] == "true");
  int interval = Config.get()["heartbeat"][inRide ? "inRide" : "notInRide"];
  if (interval > 0 && Gps.getLatitude() != 0 &&
      (lastHeartbeat == -1 || uptime - lastHeartbeat >= (uint32_t)interval)) {
    sendHeartbeat();
    lastHeartbeat = uptime;
  }
}

void SystemClass::sendInfo() {
  statusDoc["firmware"] = FIRMWARE_VERSION;
  statusDoc["inRide"] = "true";
  String version = "{\"inRide\":\"" + String(statusDoc["inRide"].as<char*>()) + "\", \"system\":{\"firmware\":\"" +
                   FIRMWARE_VERSION + "\",\"configFreeMem\":" + Config.getConfigFreeMem() + "}}";
  telemeter(version);
}

void SystemClass::sendHeartbeat() {
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  JsonObject system = heartbeat["system"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["hdop"] = Gps.getHdop();
  gps["speed"] = Gps.getSpeed();
  gps["dateTime"] = Gps.getDateTime();
  system["uptime"] = uptime;
  system["signalStrength"] = Internet.getSignalStrength();
  system["heapFreeMem"] = freeMemory();
  system["statusFreeMem"] = STATUS_DOC_SIZE - statusDoc.memoryUsage();
  telemeter(statusDoc["heartbeat"].as<String>());
}

void SystemClass::sendCanStatus() {
  if (canStatusChanged) {
    telemeter(statusDoc["can"].as<String>());
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
  String json2 = json;
  json2.replace("\"", "\\\"");
  String lastCmd = "\"system\":{\"lastCmd\":\"" + json2 + "\"}";
  if (desired["reboot"] == "true") {
    telemeter("{" + lastCmd + "}", "{\"reboot\":null}");
    reboot();
  } else if (desired["lock"] == "open") {
    Pins.unlockDoors();
    // CAN-BUS should update
    telemeter("{" + lastCmd + ",\"lock\":\"open\"}", "{\"lock\":null}");
  } else if (desired["lock"] == "close") {
    Pins.lockDoors();
    // CAN-BUS should update
    telemeter("{" + lastCmd + ",\"lock\":\"close\"}", "{\"lock\":null}");
  } else if (desired["immo"] == "lock") {
    Pins.immobilize();
    statusDoc["immo"] = "lock";
    telemeter("{" + lastCmd + ",\"immo\":\"lock\"}", "{\"immo\":null}");
  } else if (desired["immo"] == "unlock") {
    Pins.unimmobilize();
    statusDoc["immo"] = "unlock";
    telemeter("{" + lastCmd + ",\"immo\":\"unlock\"}", "{\"immo\":null}");
  } else if (desired["inRide"] == "true") {
    statusDoc["inRide"] = "true";
    telemeter("{" + lastCmd + ",\"inRide\":\"true\"}", "{\"inRide\":null}");
  } else if (desired["inRide"] == "false") {
    statusDoc["inRide"] = "false";
    telemeter("{" + lastCmd + ",\"inRide\":\"false\"}", "{\"inRide\":null}");
  } else if (!download.isNull()) {
    const char* host = download["host"] | "";
    const char* from = download["from"] | "";
    const char* to = download["to"] | "";
    if (strlen(host) > 0 && strlen(from) > 0 && strlen(to) > 0) {
      Https.download(host, from, to);
      telemeter("{" + lastCmd + "}", "{\"download\":null}");
      reboot();
    } else {
      logError("Error: " + json);
    }
  } else if (!copy.isNull()) {
    const char* from = copy["from"] | "";
    const char* to = copy["to"] | "";
    if (strlen(from) > 0 && strlen(to) > 0) {
      copyFile(from, to);
      telemeter("{" + lastCmd + "}", "{\"copy\":null}");
      reboot();
    } else {
      logError("Error: " + json);
    }
  } else {
    logError("Unknown command: " + json);
  }
}

uint32_t SystemClass::getUptime() {
  return uptime;
}

void SystemClass::kickWatchdogAndSleep() {
  digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
  // don't use Watchdog.sleep as it disconnects USB
  delay(950);
#else
  //sleep most and gps should take the other in 1 second
  Watchdog.sleep(500);
  Watchdog.sleep(250);
  Watchdog.sleep(125);
#endif
  uptime += 1;
  digitalWrite(LED_BUILTIN, HIGH);
  Watchdog.enable(WATCHDOG_TIMEOUT);
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

void SystemClass::telemeter(const String& reported, const String& desired) {
  String message = "{\"state\":{" +
                   (reported != "" ? "\"reported\":" + reported : "") +
                   (reported != "" && desired != "" ? "," : "") +
                   (desired != "" ? "\"desired\":" + desired : "") + "}}";
  Logger.logLine("Debug", message);
  if (Mqtt.isConnected()) {
    Mqtt.send(message);
  }
}

SystemClass System;
