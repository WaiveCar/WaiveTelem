#include <SD.h>

#include "Config.h"
#include "Logger.h"
#include "System.h"

void ConfigClass::load() {
  log("DEBUG");
  File file = SD.open("CONFIG.TXT");
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(configDoc, file);
  if (error) {
    log("ERROR", error.c_str());
  }
  configFreeMem = CONFIG_DOC_SIZE - configDoc.memoryUsage();
  file.close();
}

JsonDocument& ConfigClass::get() {
  return configDoc;
}

int32_t ConfigClass::getConfigFreeMem() {
  return configFreeMem;
}

ConfigClass Config;
