#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <ArduinoJson.h>

class ConfigClass {
 public:
  void load();
  JsonDocument& get();

 private:
  StaticJsonDocument<4096> configDoc;
};

extern ConfigClass Config;

#endif  // Config_h