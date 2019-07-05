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
  // go to https://arduinojson.org/v6/assistant/ to find the size, or just use a large enough number
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to read file: " + String(error.c_str()));
    file.close();
    return;
  }
  id = doc["id"].as<String>();
  mqttBrokerUrl = doc["mqttBrokerUrl"].as<String>();
  mqttBrokerCert = doc["mqttBrokerCert"].as<String>();
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
