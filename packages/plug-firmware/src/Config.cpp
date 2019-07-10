#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.h>
#else
#include <JsonStreamingParser.h>
#endif
#include <SD.h>

#include "Config.h"
#include "Console.h"
#include "Pins.h"
#include "Status.h"

#define CONFIG_FILE "/config.txt"

void ConfigClass::load() {
  while (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Failed to initialize SD Library"));
    delay(1000);
  }
  File file = SD.open(CONFIG_FILE);
#ifdef USE_ARDUINO_JSON
  // go to https://arduinojson.org/v6/assistant/ to find the size
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) +
                          2 * JSON_OBJECT_SIZE(4) + 13 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(17) + 2180;
  DynamicJsonDocument doc(capacity);
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

  canConfig.make = canDoc["make"].as<char*>();
  canConfig.model = canDoc["model"].as<char*>();
  log("canConfig.make: " + canConfig.make);
  log("canConfig.model: " + canConfig.model);
  JsonArray arr = canDoc["bus"].as<JsonArray>();
  for (JsonObject bus : arr) {
    canConfig.num_bus++;
    canConfig.bus_baud[canConfig.num_bus - 1] = bus["baud"];
  }

  arr = canDoc["telemetry"].as<JsonArray>();
  for (JsonObject obj : arr) {
    canConfig.num_telemetry++;
    CanTelemetry& telemetry = canConfig.telemetry[canConfig.num_telemetry - 1];
    telemetry.name = obj["name"].as<char*>();
    telemetry.can_id = obj["can_id"] | 0;
    telemetry.can_byte_num = obj["byte_num"] | 0;
    telemetry.can_bit_num = obj["bit_num"] | 0;
    telemetry.can_data_len = obj["len"] | 1;
    telemetry.bus_id = obj["bus_id"] | 0;
  }
#else
  JsonStreamingParser parser;
  parser.setListener(this);
  while (file.available()) {
    parser.parse((char)file.read());
  }
#endif

  file.close();
}

#ifndef USE_ARDUINO_JSON
void ConfigClass::value(String v) {
  // log("v: " + v);
  if (currentObject == "/") {
    if (currentKey == "id") {
      id = v;
    } else if (currentKey == "mqttBrokerUrl") {
      mqttBrokerUrl = v;
    } else if (currentKey == "mqttBrokerCert") {
      mqttBrokerCert = v;
    }
  } else if (currentObject == "/nb/") {
    if (currentKey == "simPin") {
      nbSimPin = v;
    }
  } else if (currentObject == "/gps/") {
    if (currentKey == "inRideInterval") {
      gpsInRideInterval = v.toInt();
    } else if (currentKey == "notInRideInterval") {
      gpsNotInRideInterval = v.toInt();
    }
  } else if (currentObject == "/can/") {
    if (currentKey == "make") {
      canConfig.make = (char*)malloc(strlen(v.c_str()) + 1);
      strcpy(canConfig.make, v.c_str());
    } else if (currentKey == "model") {
      canConfig.model = (char*)malloc(strlen(v.c_str()) + 1);
      strcpy(canConfig.model, v.c_str());
    }
  } else if (currentObject == "/can/bus//") {
    canConfig.bus_baud[arrayIndex] = v.toInt();
    canConfig.num_bus = arrayIndex + 1;
  } else if (currentObject == "/can/telemetry//") {
    CanTelemetry& telemetry = canConfig.telemetry[arrayIndex];
    if (currentKey == "name") {
      telemetry.name = (char*)malloc(strlen(v.c_str()) + 1);
      strcpy(telemetry.name, v.c_str());
      canConfig.num_telemetry = arrayIndex + 1;
    } else if (currentKey == "bus_id") {
      telemetry.bus_id = v.toInt();
    } else if (currentKey == "can_id") {
      telemetry.can_id = v.toInt();
    } else if (currentKey == "can_byte_num") {
      telemetry.can_byte_num = v.toInt();
    } else if (currentKey == "can_bit_num") {
      telemetry.can_bit_num = v.toInt();
    } else if (currentKey == "can_data_len") {
      telemetry.can_data_len = v.toInt();
    }
  }
}
#endif

String& ConfigClass::getId() {
  return id;
}

String& ConfigClass::getMqttBrokerUrl() {
  return mqttBrokerUrl;
}

String& ConfigClass::getMqttBrokerCert() {
  return mqttBrokerCert;
}

String& ConfigClass::getNbSimPin() {
  return nbSimPin;
}

int ConfigClass::getGpsInterval() {
  return (Status.getInRide() ? gpsInRideInterval : gpsNotInRideInterval) * 1000 - 500;
}

CanConfig& ConfigClass::getCanConfig() {
  return canConfig;
}

ConfigClass Config;
