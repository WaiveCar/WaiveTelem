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
#include "Motion.h"
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
#ifndef DEBUG
  rtc.begin();
  rtc.setAlarmSeconds(59);
  rtc.enableAlarm(rtc.MATCH_SS);
#endif

  String sn = ECCX08.serialNumber();
  strncpy(id, sn.c_str(), sn.length());

  char state = Config.loadImmoState();
  logDebug("i|immo", (int32_t)state - 48);
  if (state == '1') {
    Pins.immobilize();
    Bluetooth.setSystemStatus(STATUS_IMMOBILIZER, 1);
  } else if (state == '2') {
    Pins.unimmobilize();
    Bluetooth.setSystemStatus(STATUS_IMMOBILIZER, 2);
  }

  return 1;
}

void SystemClass::poll() {
  checkVin();
  checkHeartbeat();
}

void SystemClass::checkHeartbeat() {
  JsonObject heartbeat = Config.get()["heartbeat"];
  uint32_t interval = canBus[ignitionKey] == IGNITION_ON ? heartbeat["ignitionOn"] | 30 : heartbeat["ignitionOff"] | 900;
  logTrace("i|time", time, "i|lastHeartbeat", lastHeartbeat, "i|interval", interval, "i|canBus[ignition]", (int32_t)canBus[ignitionKey]);
  uint32_t elapsedTime = getTime() - lastHeartbeat;
  if (interval > 60 && elapsedTime == interval - 60) {
#ifndef DEBUG
    // Gps.wakeup();
#endif
  } else if (lastHeartbeat == -1 || elapsedTime >= interval) {
    if (Gps.poll()) {
      sendHeartbeat();
      if (interval > 60) {
#ifndef DEBUG
        // Gps.sleep();
#endif
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
      int32_t avg = total / ARRAY_SIZE(vinReads);
      int32_t limit = Config.get()["vin"]["low"] | 800;
      // logDebug("i|limit", limit);
      if (avg < limit) {
        char info[64];
        json(info, "i|vin", avg);
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
  json(info, "remoteLog", remoteLog, "o|init", sysJson);
  report(info);
}

void SystemClass::resetLastHeartbeat() {
  lastHeartbeat = -1;
}

void SystemClass::sendHeartbeat() {
  char vinBuf[32] = "+|";
#ifdef ARDUINO_SAMD_WAIVE1000
  int index = (vinIndex - 1) % ARRAY_SIZE(vinReads);
  json(vinBuf, "-{", "i|lastVin", vinReads[index]);
#endif
  char cellBuf[64] = "+|";
  if (Internet.isConnected()) {
    json(cellBuf, "-{", "i|signal", Internet.getSignalStrength(), "carrier", Internet.getCarrier().c_str());
  }
  char buf[512];
  json(buf, "{|heartbeat", "datetime", System.getDateTime(),
       "fa|lat", (double)Gps.getLatitude() / 1e7,
       "fa|long", (double)Gps.getLongitude() / 1e7,
       "i|hdop", Gps.getHdop(),
       "i|speed", Gps.getSpeed() / 869,  // convert to miles per hour
       "i|heading", Gps.getHeading() / 100,
       "i|uptime", time - bootTime,
       "f3|temp", Motion.getTemp(),
       //  "i|ble", Bluetooth.getHealth(),
       //  "i|can", Can.getHealth(),
       "i|freeMem", freeMemory(),
       vinBuf,
       cellBuf, "}|");
  // system["moreStuff"] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
  report(buf);

  lastHeartbeat = getTime();
}

void SystemClass::sendCanStatus(const char* type) {
  HashMap<void*, int64_t, 10>& telemetry = (strcmp(type, "batch") == 0) ? canBusBatch : canBusLessThanDelta;
  logTrace("i|telemetry.size(): ", telemetry.size());
  if (telemetry.size() > 0) {
    char canFragment[512] = "+|";

    while (telemetry.size() > 0) {
      void* key = telemetry.keyAt(0);
      int64_t value = telemetry.valueAt(0);
      canBus[key] = value;
      logTrace("key", key, "i|value", (int32_t)value, "i|canBus[key]", (int32_t)canBus[key]);
      telemetry.remove(key);

      // augment json with another key value pair
      char jsonKey[64];
      sprintf(jsonKey, "i|%s", (const char*)key);
      char fragmentCopy[512];
      strcpy(fragmentCopy, canFragment);
      json(canFragment, "-{", fragmentCopy, jsonKey, (int32_t)value);
    }
    char buf[512];
    json(buf, "{|canbus", canFragment, "}|");
    report(buf);
  }
  // logTrace("i|telemetry.size(): ", telemetry.size());
}

void SystemClass::setCanStatus(const char* name, int64_t value, uint32_t delta) {
  int64_t oldValue = canBus[(void*)name];
  if (oldValue != value) {
    logTrace("name", name, "i|value", (int32_t)value, "i|delta", delta, "i|oldValue", (int32_t)oldValue);
    if (abs(oldValue - value) >= delta) {
      canBus[(void*)name] = value;
      if (delta > 0) {
        canBusLessThanDelta.remove((void*)name);
      }
      canBusBatch[(void*)name] = value;
      if (strcmp(name, "ignition") == 0) {
        if (oldValue == IGNITION_ON && value != IGNITION_ON) {
          handleIgnitionOff();
        } else if (oldValue != IGNITION_ON && value == IGNITION_ON) {
          handleIgnitionOn();
        }
      }
    } else {
      canBusLessThanDelta[(void*)name] = value;
    }
  }
}

void SystemClass::sleep() {
  digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
  delay(840);
#else
  rtc.setSeconds(59);
  // TODO: it seems can bus interrupt is waking MCU up
  rtc.standbyMode();
  _ulTickCount = _ulTickCount + 250;  // 250 because can bus 1 seems to wake up MCU 4 times every second
#endif
  digitalWrite(LED_BUILTIN, HIGH);
}

void SystemClass::setTimes(uint32_t in) {
  logTrace("i|in", in);
  // sometime cellular will report 0 time, might be ISR modem AT cmd race condition
  if (in == 0) {
    return;
  }
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

void SystemClass::keepTimeWithMillis() {
  int32_t elapsed = millis() - lastMillis;
  logTrace("i|elapsed", elapsed, "i|lastMillis", lastMillis);
  if (elapsed >= 1000) {
    int32_t remainder = elapsed % 1000;
    setTimes(time + elapsed / 1000);
    lastMillis -= remainder;
  }
  logTrace("i|lastMillis", lastMillis);
}

void SystemClass::keepTime() {
  if (time % 10 != 0) {
    keepTimeWithMillis();
  } else if (Internet.isConnected()) {
    setTimes(Internet.getTime());
  } else if (!Gps.poll()) {
    keepTimeWithMillis();
  }
}

void SystemClass::reportCommandDone(const String& lastCmd, const String& cmdKey, const String& cmdValue) {
  char reported[512], desired[64];
  if (cmdValue.length() == 0) {
    json(reported, "lastCmd", lastCmd.c_str(), "lastCmdDatetime", System.getDateTime());
  } else {
    json(reported, "lastCmd", lastCmd.c_str(), cmdKey.c_str(), cmdValue.c_str(), "lastCmdDatetime", System.getDateTime());
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

void SystemClass::handleIgnitionOn() {
  // put in normal mode mpu6050
  Motion.setSleepEnabled(false);

  Bluetooth.setSystemStatus(STATUS_IGNITION, 2);
}

void SystemClass::handleIgnitionOff() {
  // send all changed data
  sendCanStatus("lessThanDelta");
  Gps.poll();
  System.sendHeartbeat();
  // put in power-saving mode for canbus, mpu6050
  // Can.sleep(); doesn't work
  Motion.setSleepEnabled(true);

  Bluetooth.setSystemStatus(STATUS_IGNITION, 1);
}

void SystemClass::simulateIgnition(const String& cmdValue) {
  if (cmdValue == "on") {
    canBus[ignitionKey] = 3;
    handleIgnitionOn();
  } else if (cmdValue == "off") {
    canBus[ignitionKey] = 0;
    handleIgnitionOff();
  }
}

void SystemClass::setIgnitionKey(void* key) {
  ignitionKey = key;
}

HashMap<void*, int64_t, 20>& SystemClass::getCanBusStatus() {
  return canBus;
}

SystemClass System;
