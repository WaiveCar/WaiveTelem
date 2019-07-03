#include <ArduinoJson.h>
#include <SD.h>

#include "Config.h"
#include "Pins.h"

#define CONFIG_FILE "/config.txt"

void ConfigClass::load() {
  while (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Failed to initialize SD Library"));
    delay(1000);
  }
  File file = SD.open(CONFIG_FILE);
  // go to https://arduinojson.org/v6/assistant/ to find the size
  const int capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 1200;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read file: " + String(error.c_str()));
  }

  id = doc["id"].as<char*>();
  mqttBrokerUrl = doc["mqttBrokerUrl"].as<char*>();
  mqttBrokerCert = doc["mqttBrokerCert"].as<char*>();

  file.close();
}

String ConfigClass::getId() {
  return id;
}

String ConfigClass::getMqttBrokerUrl() {
  return mqttBrokerUrl;
}

String ConfigClass::getMqttBrokerCert() {
  return mqttBrokerCert;
}

ConfigClass Config;