#include <Arduino.h>
#include <cstdarg>

#include "Logger.h"
#include "Pins.h"
#include "System.h"

void LoggerClass::setup() {
  log("DEBUG");
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    log("ERROR", "LOG.TXT open failed");
    return;
  }
}

void LoggerClass::logKeyValueJson(const char* key, ...) {
  String json = (char*)NULL;
  json.reserve(256);
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

  // TODO log to mqtt
  if (writeFile) {
    writeFile.println(json);
    writeFile.flush();
  }

#if DEBUG
  json.replace("{\"t\":\"", "");
  json.replace("\",\"l\":\"", " ");
  json.replace("\",\"s\":\"", " ");
  json.replace("\",\"f\":\"", " ");
  json.replace("\"", " ");
#endif

  Serial.println(json);
}

LoggerClass Logger;