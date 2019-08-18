#ifndef Logger_h
#define Logger_h

#include <SD.h>

#include "Mqtt.h"
#include "System.h"

#define logError(...) log(4, __VA_ARGS__)
#define logWarn(...) log(3, __VA_ARGS__)
#define logInfo(...) log(2, __VA_ARGS__)
#define logDebug(...) log(1, __VA_ARGS__)
#define log(...) logKv(__VA_ARGS__, NULL)
#define logKv(l, ...)                                                                     \
  if (l >= Logger.getMinLevel())                                                          \
  Logger.logKeyValueJson("t", System.getDateTime(), "l", String(l).c_str(),               \
                         "s", (String(__FILE__) + ":" + __LINE__).c_str(), "f", __func__, \
                         __VA_ARGS__)

class LoggerClass {
 public:
  void begin();
  void logKeyValueJson(const char* key, ...);
  uint8_t getMinLevel();

 private:
  File writeFile;
  uint8_t minLevel;
};

extern LoggerClass Logger;

#endif  // Logger_h