#include <Arduino.h>
#include <ArduinoECCX08.h>
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

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
  statusDoc.createNestedObject("notyetcanbus");
  return 1;
}

void SystemClass::poll() {
  checkVin();
  checkHeartbeat();
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
    vinReads[vinIndex] = analogRead(VIN_SENSE) * VOLTAGE * (RESISTOR_1 + RESISTOR_2) * 10 / (1 << ANALOG_RESOLUTION) / RESISTOR_1;
    vinIndex++;
    if (vinIndex == ARRAY_SIZE(vinReads)) {
      vinAvgValid = true;
      vinIndex = 0;
    }
    if (vinAvgValid) {
      uint32_t total = 0;
      for (int i = 0; i < (int)ARRAY_SIZE(vinReads); i++) {
        total += vinReads[i];
      }
      uint32_t avg = total / ARRAY_SIZE(vinReads);
      uint32_t limit = Config.get()["vin"]["low"];
      if (avg < limit) {
        char info[128];
        json(info, "{|system", "i|vin", avg, "}|");
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
  char vinBuf[32] = "+|";
#ifdef ARDUINO_SAMD_WAIVE1000
  int index = (vinIndex - 1) % ARRAY_SIZE(vinReads);
  json(vinBuf, "-{", "i|vin", vinReads[index]);
#endif
  char cellBuf[64] = "+|";
  if (Internet.isConnected()) {
    json(cellBuf, "-{", "i|signal", Internet.getSignalStrength(), "carrier", Internet.getCarrier().c_str());
  }
  char buf[512];
  json(buf, "{|gps", "i|lat", Gps.getLatitude(), "i|long", Gps.getLongitude(), "i|hdop", Gps.getHdop(),
       "i|speed", Gps.getSpeed(), "i|heading", Gps.getHeading(), "}|",
       "{|system", "i|ble", Bluetooth.getHealth(), "i|can", Can.getHealth(),
       "i|uptime", time - bootTime, "i|heapFreeMem", freeMemory(),
       "i|statusFreeMem", STATUS_DOC_SIZE - statusDoc.memoryUsage(), vinBuf, cellBuf, "}|");
  // system["moreStuff"] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
  report(buf);

  lastHeartbeat = getTime();
}

void SystemClass::sendNotYetCanStatus() {
  JsonObject can = statusDoc["canbus"];
  JsonObject notyet = statusDoc["notyetcanbus"];
  String canJson = statusDoc["notyetcanbus"].as<String>();
  if (canJson != "{}") {
    char buf[512];
    json(buf, "o|canbus", canJson.c_str());
    report(buf);
    for (JsonPair kv : notyet) {
      can[kv.key()] = kv.value();
      notyet.remove(kv.key());
    }
  }
}

void SystemClass::setCanStatus(const String& name, uint64_t value, uint32_t delta) {
  JsonObject can = statusDoc["canbus"];
  JsonObject notyet = statusDoc["notyetcanbus"];
  uint64_t oldValue = can[name];
  if (oldValue != value) {
    if (abs(oldValue - value) >= delta) {
      can[name] = value;
      if (delta > 0) {
        notyet.remove(name);
      }
      char buf[128];
      json(buf, "{|canbus", (String("i|") + name).c_str(), value, "}|");
      report(buf);
    } else {
      notyet[name] = value;
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
  char message[512];

  if (desired) {
    json(message, "{|state", "o|reported", reported, "o|desired", desired, "}|");
  } else {
    json(message, "{|state", "o|reported", reported, "}|");
  }
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
  if (time % 7 == 0 && Internet.isConnected()) {  // don't get time from modem too often
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

void SystemClass::reportCommandDone(const char* json, const char* cmdKey, const char* cmdValue) {
  char reported[512], desired[64];
  if (cmdValue) {
    json(reported, "{|system", "lastCmd", json, "}|", cmdKey, cmdValue);
  } else {
    json(reported, "{|system", "lastCmd", json, "}|");
  }
  json(desired, (String("o|") + cmdKey).c_str(), "null");
  report(reported, desired);
}

uint8_t SystemClass::getRemoteLogLevel() {
  return remoteLogLevel;
}

SystemClass System;
