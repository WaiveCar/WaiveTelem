#include <SD.h>

#include "Config.h"
#include "Console.h"
#include "Pins.h"
#include "System.h"

#define CONFIG_FILE "/config.txt"

void ConfigClass::load() {
  while (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Failed to initialize SD Library"));
    delay(1000);
  }
  File file = SD.open(CONFIG_FILE);
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(jsonDoc, file);
  if (error) {
    Serial.println("Failed to read file: " + String(error.c_str()));
    file.close();
    return;
  }
  logLine("JsonDoc memory usage: " + String(jsonDoc.memoryUsage()));
  logLine(jsonDoc["can"]["model"].as<char*>());

  file.close();
}

JsonDocument& ConfigClass::get() {
  return jsonDoc;
}

ConfigClass Config;
