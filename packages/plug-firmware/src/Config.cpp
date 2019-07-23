#include <SD.h>

#include "Config.h"
#include "Logger.h"
#include "Pins.h"
#include "System.h"

#define CONFIG_FILE "/config.txt"

void ConfigClass::load() {
  while (!SD.begin(SD_CS_PIN)) {
    logError(F("Failed to initialize SD Library"));
    delay(1000);
  }
  File file = SD.open(CONFIG_FILE);
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(configDoc, file);
  if (error) {
    logError("Failed to read file: " + String(error.c_str()));
    file.close();
    while (true)
      ;
  }
  logDebug("configDoc memory cushion: " + String(CONFIG_DOC_SIZE - configDoc.memoryUsage()));
  // logDebug(configDoc["can"]["model"].as<char*>());

  file.close();
}

JsonDocument& ConfigClass::get() {
  return configDoc;
}

ConfigClass Config;
