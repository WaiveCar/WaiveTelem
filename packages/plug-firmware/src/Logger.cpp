#include <Arduino.h>
#include <cstdarg>

#include "Config.h"
#include "Logger.h"
#include "Pins.h"
#include "System.h"

#define KV(key, value) json += String(",\"") + key + "\":\"" + value + "\""
#define KVNUM(key, value) json += String(",\"") + key + "\":" + String(value).c_str()

const char* LEVELS[5] = {"", "DEBUG", " INFO", " WARN", "ERROR"};

void LoggerClass::begin() {
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logError("LOG.TXT open failed");
    return;
  }
  mqttLevel = Config.get()["logger"]["mqttLevel"] | 1;
}

void LoggerClass::logKeyValueJson(int level, const char* key, ...) {
  String json = (char*)NULL;
  json.reserve(LOG_RESERVE_SIZE);
  va_list arg;
  va_start(arg, key);
  json += String("{\"l\":\"") + LEVELS[level] + "\"";
  KV(ID_KEY, ID_VALUE);
  KV(TIME_KEY, TIME_VALUE);
  while (key) {
    if (key[0] == 'i' && key[1] == '_') {
      key = key + 2;
      long value = va_arg(arg, long);
      KVNUM(key, value);
    } else if (key[0] == 'f' && key[1] == '_') {
      key = key + 2;
      double value = va_arg(arg, double);
      KVNUM(key, value);
    } else {
      bool isNoMore = false;
      const char* value = va_arg(arg, const char*);
      if (!value) {
        value = key;
        key = "m";
        isNoMore = true;
      }
      KV(key, value);
      if (isNoMore) {
        break;
      }
    }
    key = va_arg(arg, const char*);
  }
  va_end(arg);
  json += "}";

  if (level >= mqttLevel) {
    if (Mqtt.isConnected()) {
      Mqtt.logMsg(json);
    }
  }
  if (writeFile) {
    writeFile.println(json);
    writeFile.flush();
  }

#if LOG_EASYREAD_SERIAL
  json.replace("{\"l\":\"", "");

  json.replace("\",\"i\":\"", "");
  json.replace(ID_VALUE, "");

  json.replace("\",\"t\":\"", " ");
  json.replace("\",\"s\":\"", " ");
  json.replace("\",\"f\":\"", " ");

  json.replace("\"", " ");
#endif
  Serial.println(json);
}

LoggerClass Logger;