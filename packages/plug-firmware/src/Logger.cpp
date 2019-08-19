#include <Arduino.h>
#include <cstdarg>

#include "Config.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#ifndef LOG_ID_VALUE
#error You must define LOG_ID_VALUE in build_flags (e.g. -D LOG_ID_VALUE=System.getId())which is used by logger.cpp to log device id
#endif

#ifndef LOG_TIME_VALUE
#error You must define LOG_ID_VALUEin build_flags (e.g. -D LOG_ID_VALUE=System.getDateTime()) which is used by logger.cpp to log time string
#endif

// #ifndef LOG_EASYREAD_SERIAL
// #pragma message "define LOG_EASYREAD_SERIAL in build_flags (e.g. -D LOG_EASYREAD_SERIAL) if you want more readable SERIAL output (but no longer JSON)"
// #endif

// can still generate json > LOG_RESERVE_SIZE, just takes slighly longer
#ifndef LOG_RESERVE_SIZE
#define LOG_RESERVE_SIZE 256
#endif

#define KV_STR(key, value) json += String(",\"") + key + "\":\"" + value + "\""
#define KV_NUM(key, value) json += String(",\"") + key + "\":" + String(value).c_str()
#define KV_OTHER(key, value) json += String(",\"") + key + "\":" + value

const char* LEVELS[5] = {"", "DEBUG", "INFO", "WARN", "ERROR"};

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
  json += String("{\"" LOG_ID_KEY "\":\"") + LOG_ID_VALUE + "\"";
  KV_STR(LOG_TIME_KEY, LOG_TIME_VALUE);
  KV_STR(LOG_LEVEL_KEY, LEVELS[level]);
  while (key) {
    if (key[0] == 'i' && key[1] == '_') {
      key = key + 2;
      long value = va_arg(arg, long);
      KV_NUM(key, value);
    } else if (key[0] == 'f' && key[1] == '_') {
      key = key + 2;
      double value = va_arg(arg, double);
      KV_NUM(key, value);
    } else if (key[0] == 'b' && key[1] == '_') {
      key = key + 2;
      bool value = va_arg(arg, int);
      KV_NUM(key, value);
    } else if (key[0] == 'o' && key[1] == '_') {
      key = key + 2;
      const char* value = va_arg(arg, const char*);
      KV_OTHER(key, value);
    } else {
      bool isNoMore = false;
      const char* value = va_arg(arg, const char*);
      if (!value) {
        value = key;
        key = LOG_MESSAGE_KEY;
        isNoMore = true;
      }
      KV_STR(key, value);
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
      if (level == 4) {
        String escapedJson = json;
        escapedJson.replace("\"", "\\\"");
        System.report("{\"system\":{\"lastError\":\"" + escapedJson + "\"}}");
      }
    }
  }
  if (writeFile) {
    writeFile.println(json);
    writeFile.flush();
  }

#if LOG_EASYREAD_SERIAL
  json.replace(String("{\"" LOG_ID_KEY "\":\""), "");
  json.replace(LOG_ID_VALUE, "");

  json.replace(String("\",\"" LOG_TIME_KEY "\":\""), "");

  json.replace(String("\",\"" LOG_LEVEL_KEY "\":\""), strlen(LEVELS[level]) == 5 ? " " : "  ");
  json.replace(String("\",\"" LOG_SOURCE_KEY "\":\""), " ");
  json.replace(String("\",\"" LOG_FUNC_KEY "\":\""), " ");

  json.replace("\"", " ");
#endif
  Serial.println(json);
}

LoggerClass Logger;
