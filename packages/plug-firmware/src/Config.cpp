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

  // JsonObject mqttDoc = doc["mqtt"];
  // id = mqttDoc["id"];
  // mqttBrokerUrl = mqttDoc["mqttBrokerUrl"];
  // mqttBrokerCert = mqttDoc["mqttBrokerCert"];
  // nbSimPin = doc["nb"]["simPin"];

  // JsonObject gpsDoc = doc["gps"];
  // gpsInRideInterval = gpsDoc["inRideInterval"] | 0;
  // gpsNotInRideInterval = gpsDoc["notInRideInterval"] | 0;

  // JsonObject canDoc = doc["can"];
  // canConfig.make = strdup(canDoc["make"] | "");
  // canConfig.model = strdup(canDoc["model"] | "");

  // JsonArray arr = canDoc["bus"];
  // for (JsonObject bus : arr) {
  //   canConfig.num_bus++;
  //   canConfig.bus_baud[canConfig.num_bus - 1] = bus["baud"];
  // }

  // arr = canDoc["telemetry"];
  // for (JsonObject obj : arr) {
  //   canConfig.num_telemetry++;
  //   CanTelemetry& telemetry = canConfig.telemetry[canConfig.num_telemetry - 1];
  //   telemetry.name = strdup(obj["name"]);
  //   telemetry.can_id = obj["can_id"];
  //   telemetry.can_byte_num = obj["byte_num"];
  //   telemetry.can_bit_num = obj["bit_num"];
  //   telemetry.can_data_len = obj["len"];
  //   telemetry.bus_id = obj["bus_id"] | 0xff;
  // }

  file.close();
}

JsonDocument& ConfigClass::get() {
  return jsonDoc;
}

// char* ConfigClass::getId() {
//   return id;
// }

// char* ConfigClass::getMqttBrokerUrl() {
//   return mqttBrokerUrl;
// }

// char* ConfigClass::getMqttBrokerCert() {
//   return mqttBrokerCert;
// }

// char* ConfigClass::getNbSimPin() {
//   return nbSimPin;
// }

// int ConfigClass::getGpsInterval() {
//   return (System.getInRide() ? gpsInRideInterval : gpsNotInRideInterval) * 1000 - 500;
// }

// CanConfig& ConfigClass::getCanConfig() {
//   return canConfig;
// }

ConfigClass Config;
