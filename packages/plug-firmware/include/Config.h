#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <ArduinoJson.h>

#define CONFIG_DOC_SIZE 4096

class ConfigClass {
 public:
  void load();
  JsonDocument& get();

 private:
  StaticJsonDocument<CONFIG_DOC_SIZE> configDoc;
};

extern ConfigClass Config;

#endif  // Config_h