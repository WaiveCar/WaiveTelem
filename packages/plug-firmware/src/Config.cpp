#include <ArduinoJson.h>
#include <SD.h>

#include "Config.h"
#include "Console.h"
#include "Pins.h"

#define CONFIG_FILE "/config.txt"

static const char* can_labels[] = {"ignition", "mileage", "fuel_level", "charging", "can_speed", "central_lock", "door_front_left",
                                   "door_front_right", "door_back_left", "door_back_right", "window_front_left", "window_front_right",
                                   "window_back_left", "window_back_right"};

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

  gpsTelemetry = doc["gps"]["telemetry"].as<bool>();
  gpsInRideInterval = doc["gps"]["inRideInterval"].as<int>();
  gpsNotInRideInterval = doc["gps"]["notInRideInterval"].as<int>();

  JsonObject canDoc = doc["can"];

  strncpy(canConfig.make, canDoc["make"], sizeof(canConfig.make));
  strncpy(canConfig.model, canDoc["model"], sizeof(canConfig.model));
  log("canConfig.make: " + String(canConfig.make));
  log("canConfig.model: " + String(canConfig.model));
  if (canDoc["canbus"][0] != nullptr) {
    canConfig.bus_baud[0] = canDoc["canbus"][0]["baud"];
    canConfig.num_can++;
  }
  if (canDoc["canbus"][1] != nullptr) {
    canConfig.bus_baud[1] = canDoc["canbus"][1]["baud"];
    canConfig.num_can++;
  }
  if (canDoc["canbus"][2] != nullptr) {
    canConfig.bus_baud[2] = canDoc["canbus"][2]["baud"];
    canConfig.num_can++;
  }

  for (int i = 0; i < num_can_items; i++) {
    canConfig.can_id[i] = doc[can_labels[i]]["can_id"] | 0;
    canConfig.can_byte_num[i] = doc[can_labels[i]]["byte_num"] | 0;
    canConfig.can_bit_num[i] = doc[can_labels[i]]["bit_num"] | 0;
    canConfig.can_data_len[i] = doc[can_labels[i]]["len"] | 1;
    canConfig.bus_id[i] = doc[can_labels[i]]["bus_id"] | 0;
  }

  file.close();
}

String& ConfigClass::getId() {
  return id;
}

String& ConfigClass::getMqttBrokerUrl() {
  return mqttBrokerUrl;
}

String& ConfigClass::getMqttBrokerCert() {
  return mqttBrokerCert;
}

int ConfigClass::getGpsInterval() {
  if (!gpsTelemetry) {
    return -1;
  }
  return (true ? gpsInRideInterval : gpsNotInRideInterval) * 1000 - 500;
}

CanConfig& ConfigClass::getCanConfig() {
  return canConfig;
}

ConfigClass Config;
