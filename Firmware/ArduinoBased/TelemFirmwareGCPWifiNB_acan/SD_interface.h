
// including the SDU library will look for a file on the SD card called UPDATE.bin,
// and if so loads it as the new firmware and deletes the file. Ensure this library is
// included in all updates so that firmware can be perpetually upgraded.
#include <SDU.h>
#include <SD.h>
#include <SerialFlash.h>
#include <ArduinoJson.h>
#include "can_bus_interface.h"


//Config structure, loaded via loadConfiguration(), updated via updateConfiguration(), stored via saveConfiguration()
struct Config {
  char make[32];
  char model[32];
  int num_can = 0;
  int bus_baud[3];
  int can_id[num_can_items];
  int can_byte_num[num_can_items];
  int can_bit_num[num_can_items];
  int can_data_len[num_can_items];
  int bus_id[num_can_items];

};



void loadImmobilizer(const char *immofile, bool &immobilized) {
  File file = SD.open(immofile);

  //Allocate a temporary JsonDocument
  const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  immobilized = doc["immobilized"] | true;
  file.close();
}

void saveImmobilizer(const char *immofile, bool &immobilized) {
  // Delete existing file, otherwise the immostate is appended to the file
  Serial.println("Saving immobilizer state...");
  SD.remove(immofile);

  // Open file for writing
  File file = SD.open(immofile, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  //Allocate a temporary JsonDocument
  const size_t capacity = JSON_OBJECT_SIZE(1);
  DynamicJsonDocument doc(capacity);

  doc["immobilized"] = immobilized;
  size_t file_size = sizeof(file);
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File file = SD.open(filename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + 14 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(17) + 900;
  DynamicJsonDocument doc(capacity);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print(F("Failed to read file, trying again"));
    file.close();
    file = SD.open(filename);
    error = deserializeJson(doc, file);
    if (error)
      Serial.println(F("Failed to read file twice, using default configuration"));
  }
  // Copy values from the JsonDocument to the Config

  strlcpy(config.make,                  // <- destination
          doc["make"],  // <- source
          sizeof(config.make));         // <- destination's capacity
  strlcpy(config.model,                 // <- destination
          doc["model"],  // <- source
          sizeof(config.model));         // <- destination's capacity
//  if (doc["canbus"][0]) {
    JsonObject canbus_0 = doc["canbus"][0];
    config.bus_baud[0] = canbus_0["baud"]; // 500
    config.num_can++;
//  }
  if (doc["canbus"][1]) {
    JsonObject canbus_1 = doc["canbus"][1];
    config.bus_baud[1] = canbus_1["baud"]; // 500
    config.num_can++;
  }
  if (doc["canbus"][2]) {
    JsonObject canbus_2 = doc["canbus"][2];
    config.bus_baud[2] = canbus_2["baud"]; // 500
    config.num_can++;
  }

  for (int i = 0; i < num_can_items; i++) {
    config.can_id[i] = doc[can_labels[i]]["can_id"] | 0;
    config.can_byte_num[i] = doc[can_labels[i]]["byte_num"] | 0;
    config.can_bit_num[i] = doc[can_labels[i]]["bit_num"] | 0;
    config.can_data_len[i] = doc[can_labels[i]]["len"] | 1;
    config.bus_id[i] = doc[can_labels[i]]["bus_id"] | 0;
  }
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, const Config &config) {
  // Delete existing file, otherwise the configuration is appended to the file
  SD.remove(filename);

  // Open file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + 14 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(17);
  DynamicJsonDocument doc(capacity);

  // Set the values in the document
  doc["make"] = config.make;
  doc["model"] = config.model;

  JsonArray canbus = doc.createNestedArray("canbus");

  JsonObject canbus_0 = canbus.createNestedObject();
  canbus_0["id"] = "can0";
  canbus_0["baud"] = config.bus_baud[0];

  if (config.num_can > 1) {
    JsonObject canbus_1 = canbus.createNestedObject();
    canbus_1["id"] = "can1";
    canbus_1["baud"] = config.bus_baud[1];
  }
  if (config.num_can > 2) {
    JsonObject canbus_2 = canbus.createNestedObject();
    canbus_2["id"] = "can2";
    canbus_2["baud"] = config.bus_baud[2];
  }
  for (int i = 0; i < num_can_items; i++) {
    doc.createNestedObject(can_labels[i]);
    doc[can_labels[i]]["can_id"] = config.can_id[i];
    doc[can_labels[i]]["byte_num"] = config.can_byte_num[i];
    doc[can_labels[i]]["bit_num"] = config.can_bit_num[i];
    doc[can_labels[i]]["len"] = config.can_data_len[i];
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

// Prints the content of a file to the Serial
void printFile(const char *filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}
