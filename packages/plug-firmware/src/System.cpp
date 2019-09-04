#include <Arduino.h>
#include <ArduinoECCX08.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>
#include <JsonLogger.h>
#include <NMEAGPS.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
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

int SystemClass::begin() {
  rtc.begin();
  rtc.setAlarmSeconds(59);
  rtc.enableAlarm(rtc.MATCH_SS);

  sprintf(id, "%s", ECCX08.serialNumber().c_str());

  statusDoc.createNestedObject("canbus");
  statusDoc.createNestedObject("heartbeat");
  statusDoc["heartbeat"].createNestedObject("gps");
  statusDoc["heartbeat"].createNestedObject("system");
  return 1;
}

void SystemClass::poll() {
  checkHeartbeat();
  checkVin();
}

void SystemClass::checkHeartbeat() {
  // logDebug("b|inRide", inRide, "statusDoc['inRide']", statusDoc["inRide"].as<char*>());
  JsonObject heartbeat = Config.get()["heartbeat"];
  uint32_t interval = inRide ? heartbeat["inRide"] | 30 : heartbeat["notInRide"] | 900;
  // logDebug("i|time", time, "i|lastHeartbeat", lastHeartbeat, "i|interval", interval);
  uint32_t elapsedTime = getTime() - lastHeartbeat;
  if (interval > 60 && elapsedTime == interval - 60) {
    Gps.wakeup();
  } else if (lastHeartbeat == -1 || elapsedTime >= interval) {
    if (Gps.poll()) {
      sendHeartbeat();
      lastHeartbeat = getTime();
      if (interval > 60) {
        Gps.sleep();
      }
    }
  }
}

void SystemClass::checkVin() {
#ifdef ARDUINO_SAMD_WAIVE1000
  uint32_t elapsedTime = getTime() - lastVinRead;
  if (lastVinRead == -1 || elapsedTime >= 10) {
    vinReads[vinIndex] = (float)analogRead(VIN_SENSE) / (1 << ANALOG_RESOLUTION) * VOLTAGE * (RESISTOR_1 + RESISTOR_2) / RESISTOR_1;
    vinIndex++;
    if (vinIndex == 5) {
      vinAvgValid = true;
      vinIndex = 0;
    }
    if (vinAvgValid) {
      float total = 0;
      for (int i = 0; i < 5; i++) {
        total += vinReads[i];
      }
      float avg = total / 5;
      // const char* limitStr = Config.get()["vin"]["low"] | "12.4";
      //TODO strtof takes 2% ROM, maybe we should just code the limit
      // float limit = strtof(limitStr, NULL);
      // logTrace("f|5limit", limit);
      if (avg < 10) {
        char sysJson[64], info[128];
        json(sysJson, "f|5vin", avg);
        json(info, "o|system", sysJson);
        report(info);
      }
    }
    lastVinRead = getTime();
  }
#endif
}

uint32_t SystemClass::getTime() {
  return time;
}

const char* SystemClass::getDateTime() {
  NeoGPS::time_t dt = time;
  sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
  return dateTime;
}

void SystemClass::sendInfo(const char* sysJson) {
  char info[512];
  json(info, "inRide", inRide ? "true" : "false", "o|system", sysJson);
  report(info);
}

void SystemClass::sendHeartbeat() {
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  JsonObject system = heartbeat["system"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["hdop"] = Gps.getHdop();
  gps["speed"] = Gps.getSpeed();
  gps["heading"] = Gps.getHeading();
  // system["time"] = System.getDateTime();
  system["ble"] = Bluetooth.getHealth();
  system["can"] = Can.getHealth();
  system["uptime"] = time - bootTime;
  system["signal"] = Internet.getSignalStrength();
  system["heapFreeMem"] = freeMemory();
  system["statusFreeMem"] = STATUS_DOC_SIZE - statusDoc.memoryUsage();
  // system["moreStuff"] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
  report(statusDoc["heartbeat"].as<String>().c_str());
}

void SystemClass::sendCanStatus() {
  String canJson = statusDoc["canbus"].as<String>();
  if (canStatusChanged) {
    if (canJson != "{}") {
      char buf[512];
      json(buf, "o|canbus", canJson.c_str());
      report(buf);
    }
    canStatusChanged = false;
  }
}

void SystemClass::setCanStatus(const String& name, uint64_t value, uint32_t delta) {
  JsonObject can = statusDoc["canbus"];
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
  delay(sec * 1000);  // don't use rtc.standbyMode as it disconnects USB
#else
  rtc.setSeconds(60 - sec);
  rtc.standbyMode();
  _ulTickCount = _ulTickCount + sec * 1000;
#endif
  digitalWrite(LED_BUILTIN, HIGH);
}

void SystemClass::setTimes(uint32_t in) {
  time = in;
  if (bootTime == 0) {
    bootTime = in - millis() / 1000;
  }
  lastMillis = millis();
}

void SystemClass::report(const char* reported, const char* desired) {
  char state[512];
  char message[512];

  if (desired) {
    json(state, "o|reported", reported, "o|desired", desired);
  } else {
    json(state, "o|reported", reported);
  }
  json(message, "o|state", state);
  logInfo("o|message", message);
  if (Mqtt.isConnected()) {
    Mqtt.updateShadow(message);
  }
}

bool SystemClass::stayResponsive() {
  return stayresponsive;
}

void SystemClass::setStayResponsive(bool resp) {
  stayresponsive = resp;
}

void SystemClass::keepTime() {
  if (time % 60 == 0 && Internet.isConnected()) {  // don't get time from modem too often
    setTimes(Internet.getTime());
  } else {
    int32_t elapsed = millis() - lastMillis;
    if (elapsed >= 1000) {
      setTimes(time + elapsed / 1000);
    }
  }
}

void SystemClass::setInRide(bool in) {
  inRide = in;
}

void SystemClass::setCanStatusChanged() {
  canStatusChanged = true;
}

void SystemClass::reportCommandDone(const char* json, const char* cmdKey, const char* cmdValue) {
  char lastCmd[256], reported[512], desired[64];
  json(lastCmd, "lastCmd", json);
  if (cmdValue) {
    json(reported, "o|system", lastCmd, cmdKey, cmdValue);
  } else {
    json(reported, "o|system", lastCmd);
  }
  json(desired, (String("o|") + cmdKey).c_str(), "null");
  report(reported, desired);
}

uint8_t SystemClass::getRemoteLogLevel() {
  return remoteLogLevel;
}

SystemClass System;
