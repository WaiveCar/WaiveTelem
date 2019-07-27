#include <SD.h>

#include "Config.h"
#include "Logger.h"
#include "System.h"

void ConfigClass::load() {
  File file = SD.open("CONFIG.TXT");
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(configDoc, file);
  if (error) {
    logError("Failed to read file: " + String(error.c_str()));
    file.close();
    while (true)
      ;
  }
  logInfo("configDoc memory cushion: " + String(CONFIG_DOC_SIZE - configDoc.memoryUsage()));
  // logDebug(configDoc["can"]["model"].as<char*>());

  file.close();
}

JsonDocument& ConfigClass::get() {
  return configDoc;
}

ConfigClass Config;
