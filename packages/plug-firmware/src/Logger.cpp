#include <Arduino.h>
#include <cstdarg>

#include "Config.h"
#include "Logger.h"
#include "Pins.h"
#include "System.h"

void LoggerClass::begin() {
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logError("LOG.TXT open failed");
    return;
  }
  minLevel = Config.get()["logger"]["minLevel"] | 2;
}

void LoggerClass::logKeyValueJson(const char* key, ...) {
  String json = (char*)NULL;
  json.reserve(512);
  va_list arg;
  va_start(arg, key);
  bool isFirst = true;
  bool isNoMore = false;
  while (key) {
    const char* value = va_arg(arg, const char*);
    if (!value) {
      value = key;
      key = "msg";
      isNoMore = true;
    }
    json += (isFirst ? "{\"" : "\",\"") + String(key) + "\":\"" + value;
    if (isNoMore) {
      break;
    }
    key = va_arg(arg, const char*);
    isFirst = false;
  }
  va_end(arg);
  json += "\"}";

  if (Mqtt.isConnected()) {
    Mqtt.logMsg(json);
  }
  if (writeFile) {
    writeFile.println(json);
    writeFile.flush();
  }

  // #if DEBUG
  // make log more readible for human, but no longer json
  json.replace("{\"t\":\"", "");
  json.replace("\",\"l\":\"", " ");
  json.replace("\",\"s\":\"", " ");
  json.replace("\",\"f\":\"", " ");
  Serial.println(json);
  // #endif
  json.replace("\"", " ");
}

uint8_t LoggerClass::getMinLevel() {
  return minLevel;
}

LoggerClass Logger;