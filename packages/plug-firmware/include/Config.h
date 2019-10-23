#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <ArduinoJson.h>

#define CONFIG_DOC_SIZE 2700

class ConfigClass {
 public:
  int begin();
  JsonDocument& get();
  int32_t getConfigFreeMem();
  char loadImmoState();  // '1': immobilized (locked), '2': unimmobilized (unlocked)
  void saveImmoState(char value);

 private:
  StaticJsonDocument<CONFIG_DOC_SIZE> configDoc;
  int32_t configFreeMem;
};

extern ConfigClass Config;

#endif  // Config_h