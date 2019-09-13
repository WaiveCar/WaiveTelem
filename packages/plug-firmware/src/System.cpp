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

#define IGNITION_ON 3

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

  String sn = ECCX08.serialNumber();
  strncpy(id, sn.c_str(), sn.length());

  JsonObject can = statusDoc.createNestedObject("canbus");
  can.createNestedObject("lessThanDelta");
  can.createNestedObject("batch");
  return 1;
}

void SystemClass::poll() {
  checkVin();
  checkHeartbeat();
}

void SystemClass::checkHeartbeat() {
  JsonObject heartbeat = Config.get()["heartbeat"];
  int ignition = statusDoc["canbus"]["ignition"] | 0;
  uint32_t interval = ignition == IGNITION_ON ? heartbeat["ignitionOn"] | 30 : heartbeat["ignitionOff"] | 900;
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
  char info[512], remoteLog[2];
  snprintf(remoteLog, 2, "%c", '0' + remoteLogLevel);
  json(info, "remoteLog", remoteLog, "o|system", sysJson);
  report(info);
}

void SystemClass::resetLastHeartbeat() {
  lastHeartbeat = -1;
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
  json(buf, "{|gps", "i|lat", Gps.getLatitude(),
       "i|long", Gps.getLongitude(),
       "i|hdop", Gps.getHdop(),
       "i|speed", Gps.getSpeed() / 869,  // convert to miles per hour
       "i|heading", Gps.getHeading() / 100, "}|",
       "{|system", "i|ble", Bluetooth.getHealth(),
       "i|can", Can.getHealth(),
       "i|uptime", time - bootTime,
       "i|heapFreeMem", freeMemory(),
       "i|statusFreeMem", STATUS_DOC_SIZE - statusDoc.memoryUsage(),
       vinBuf, cellBuf, "}|");
  // system["moreStuff"] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
  report(buf);

  lastHeartbeat = getTime();
}

void SystemClass::sendCanStatus(const char* type) {
  JsonObject can = statusDoc["canbus"];
  JsonObject telemetry = statusDoc[type];
  if (telemetry.size() > 0) {
    const char* canJson = statusDoc[type].as<char*>();
    char buf[512];
    json(buf, "o|canbus", canJson);
    report(buf);
    for (JsonPair kv : telemetry) {
      can[kv.key()] = kv.value();
      telemetry.remove(kv.key());
    }
  }
}

bool SystemClass::setCanStatus(const char* name, uint64_t value, uint32_t delta) {
  JsonObject can = statusDoc["canbus"];
  JsonObject lessThanDelta = can["lessThanDelta"];
  JsonObject batch = can["batch"];
  uint64_t oldValue = can[name];
  bool isBatched = false;
  if (oldValue != value) {
    if (abs(oldValue - value) >= delta) {
      can[name] = value;
      if (delta > 0) {
        lessThanDelta.remove(name);
      }
      batch[name] = value;
      isBatched = true;
      if (strcmp(name, "ignition") == 0) {
        if (oldValue == IGNITION_ON && value != IGNITION_ON) {
          // send all the changed data
          sendCanStatus("lessThanDelta");
          Gps.poll();
          System.sendHeartbeat();
          // put in power-saving mode for canbus, mpu6050
          // change heartbeat interval to 15 minutes, gps will be sleep for the first 14 minutes
        } else if (oldValue != IGNITION_ON && value == IGNITION_ON) {
          // put in normal mode canus, mpu6050
          // change heartbeat interval to 30 secs
        }
      }
    } else {
      lessThanDelta[name] = value;
    }
  }
  return isBatched;
}

void SystemClass::sleep(uint32_t sec) {
  digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
  delay(sec * 1000 - 150);
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

  int len;
  if (desired) {
    len = json(message, "{|state", "o|reported", reported, "o|desired", desired, "}|");
  } else {
    len = json(message, "{|state", "o|reported", reported, "}|");
  }
  logInfo("o|message", message);
  if (Mqtt.isConnected()) {
    Mqtt.updateShadow(message, len);
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

void SystemClass::reportCommandDone(const String& lastCmd, const String& cmdKey, const String& cmdValue) {
  char reported[512], desired[64];
  if (cmdValue.length() == 0) {
    json(reported, "{|system", "lastCmd", lastCmd.c_str(), "}|");
  } else {
    json(reported, "{|system", "lastCmd", lastCmd.c_str(), "}|", cmdKey.c_str(), cmdValue.c_str());
  }
  json(desired, ("o|" + cmdKey).c_str(), "null");
  report(reported, desired);
}

void SystemClass::setRemoteLogLevel(int8_t in) {
  remoteLogLevel = in;
}

int8_t SystemClass::getRemoteLogLevel() {
  return remoteLogLevel;
}

SystemClass System;
