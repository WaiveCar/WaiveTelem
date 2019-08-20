#include <SD.h>

#include "Config.h"
#include "Logger.h"
#include "System.h"

int ConfigClass::begin() {
  File file = SD.open("CONFIG.TXT");
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(configDoc, file);
  if (error) {
    logError("error", error.c_str());
  }
  configFreeMem = CONFIG_DOC_SIZE - configDoc.memoryUsage();
  file.close();
  return error.code();
}

JsonDocument& ConfigClass::get() {
  return configDoc;
}

int32_t ConfigClass::getConfigFreeMem() {
  return configFreeMem;
}

ConfigClass Config;
