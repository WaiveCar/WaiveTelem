#include <Arduino.h>
#include <ArduinoECCX08.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>
#include <NMEAGPS.h>

#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "System.h"

extern "C" char* sbrk(int incr);
extern volatile uint32_t _ulTickCount;

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

const char* SystemClass::getId() {
  return id;
}

void SystemClass::setup() {
#ifndef DEBUG
  rtc.begin();
  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_SS);
#endif

  sprintf(id, "%s", ECCX08.serialNumber().c_str());
  log("DEBUG", "id", id);

  statusDoc.createNestedObject("can");
  statusDoc.createNestedObject("heartbeat");
  statusDoc["heartbeat"].createNestedObject("gps");
  statusDoc["heartbeat"].createNestedObject("system");
}

void SystemClass::poll() {
  bool inRide = (statusDoc["inRide"] == "true");
  JsonObject heartbeat = Config.get()["heartbeat"];
  uint32_t interval = inRide ? heartbeat["inRide"] | 30 : heartbeat["notInRide"] | 900;
  // log("INFO_", "time: " + String(time));
  // log("INFO_", "lastHeartbeat: " + String(lastHeartbeat));
  if (time - lastHeartbeat == interval * 29 / 30 - 15) {
    Gps.wakeup();
  } else if (lastHeartbeat == -1 || time - lastHeartbeat >= interval) {
    if (Gps.poll()) {
      sendHeartbeat();
      lastHeartbeat = getTime();
      Gps.sleep();
    }
  }
}

uint32_t SystemClass::getTime() {
  return time;
}

const char* SystemClass::getDateTime() {
  NeoGPS::time_t dt = time;
  sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
  return dateTime;
}

void SystemClass::sendInfo() {
  statusDoc["firmware"] = FIRMWARE_VERSION;
#ifdef DEBUG
  statusDoc["inRide"] = "true";
#else
  statusDoc["inRide"] = "false";
#endif
  String version = "{\"inRide\":\"" + String(statusDoc["inRide"].as<char*>()) + "\", \"system\":{\"firmware\":\"" +
                   FIRMWARE_VERSION + "\",\"configFreeMem\":" + Config.getConfigFreeMem() + "}}";
  report(version);
}

void SystemClass::sendHeartbeat() {
  // log("DEBUG");
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  JsonObject system = heartbeat["system"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["hdop"] = Gps.getHdop();
  gps["speed"] = Gps.getSpeed();
  gps["heading"] = Gps.getHeading();
  system["dateTime"] = System.getDateTime();
  system["uptime"] = time - bootTime;
  system["signalStrength"] = Internet.getSignalStrength();
  system["heapFreeMem"] = freeMemory();
  system["statusFreeMem"] = STATUS_DOC_SIZE - statusDoc.memoryUsage();
  report(statusDoc["heartbeat"].as<String>());
}

void SystemClass::sendCanStatus() {
  log("DEBUG");
  if (canStatusChanged) {
    report("{\"can\":" + statusDoc["can"].as<String>() + "}");
    canStatusChanged = false;
  }
}

void SystemClass::setCanStatus(const String& name, uint64_t value, uint32_t delta) {
  log("DEBUG");
  JsonObject can = statusDoc["can"];
  uint64_t oldValue = can[name];
  if (oldValue != value) {
    can[name] = value;
    if (abs(oldValue - value) >= delta) {
      canStatusChanged = true;
    }
  }
}

void SystemClass::sleep(uint32_t sec) {
  digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
  // don't use Watchdog.sleep as it disconnects USB
  delay(sec * 1000);
#else
  rtc.setSeconds(0);
  rtc.standbyMode();
  _ulTickCount = _ulTickCount + sec * 1000;
#endif
  digitalWrite(LED_BUILTIN, HIGH);
}

void SystemClass::setTimes(uint32_t in) {
  time = in;
  if (bootTime == 0) {
    bootTime = in;
  }
  lastMillis = millis();
}

void SystemClass::report(const String& reported, const String& desired) {
  String message = "{\"state\":{" +
                   (reported != "" ? "\"reported\":" + reported : "") +
                   (reported != "" && desired != "" ? "," : "") +
                   (desired != "" ? "\"desired\":" + desired : "") + "}}";
  log("DEBUG", "report", message.c_str());
  if (Mqtt.isConnected()) {
    Mqtt.send(message);
  }
}

bool SystemClass::getStayAwake() {
  return stayAwake;
}

void SystemClass::setStayAwake(bool stay) {
  stayAwake = stay;
}

void SystemClass::keepTime() {
  if (time % 20 == 0 && Internet.isConnected()) {  // don't get time from modem too often; only every 20 secs
    setTimes(Internet.getTime());
  } else {
    int32_t elapsed = millis() - lastMillis;
    if (elapsed >= 1000) {
      setTimes(time + elapsed / 1000);
    }
  }
}

void SystemClass::setCanStatusChanged() {
  canStatusChanged = true;
}

void SystemClass::reportCommandDone(const String& json, String& cmdKey, String& cmdValue) {
  // log("DEBUG");
  statusDoc[cmdKey] = cmdValue;
  String escapedJson = json;
  escapedJson.replace("\"", "\\\"");
  String lastCmd = "\"system\":{\"lastCmd\":\"" + escapedJson + "\"}";
  System.report("{" + lastCmd + +",\"" + cmdKey + "\":\"" + cmdValue + "\"}", "{\"" + cmdKey + "\":null}");
}

SystemClass System;
