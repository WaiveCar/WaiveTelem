#include <JsonLogger.h>
#include <SD.h>

#include "Config.h"

#define CONFIG_FILE "CONFIG.TXT"
#define IMMO_FILE "IMMO.TXT"

int ConfigClass::begin() {
  File file = SD.open(CONFIG_FILE);
  if (!file) {
    logError(CONFIG_FILE " open failed");
    return -1;
  }
  // go to https://arduinojson.org/v6/assistant/ to find the size
  DeserializationError error = deserializeJson(configDoc, file);
  if (error) {
    logError("error", error.c_str());
  }
  configFreeMem = CONFIG_DOC_SIZE - configDoc.memoryUsage();
  file.close();
  int err = -error.code();
  return err ? err : 1;
}

JsonDocument& ConfigClass::get() {
  return configDoc;
}

int32_t ConfigClass::getConfigFreeMem() {
  return configFreeMem;
}

char ConfigClass::loadImmoState() {
  File file = SD.open(IMMO_FILE);
  if (!file) {
    logWarn(IMMO_FILE " open failed");
    return -1;
  }

  char value;
  char len = file.read(&value, 1);
  if (len < 1) {
    logError("i|len", len);
    value = -1;
  }
  file.close();
  return value;
}

void ConfigClass::saveImmoState(char value) {
  File file = SD.open(IMMO_FILE, FILE_WRITE);

  if (!file) {
    logError(IMMO_FILE " open failed");
  }
  file.seek(0);
  char len = file.write(&value, 1);
  file.flush();
  if (len < 1) {
    logError("i|len", len);
  }
  file.close();
}

ConfigClass Config;
