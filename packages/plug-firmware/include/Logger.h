#ifndef Logger_h
#define Logger_h

#include <SD.h>

#include "Mqtt.h"
#include "System.h"

#define log(...) logKv(__VA_ARGS__, NULL)
#define logKv(l, ...)                                                                     \
  Logger.logKeyValueJson("t", System.getDateTime(), "l", l,                               \
                         "s", (String(__FILE__) + ":" + __LINE__).c_str(), "f", __func__, \
                         __VA_ARGS__)

class LoggerClass {
 public:
  void begin();
  void logKeyValueJson(const char* key, ...);

 private:
  File writeFile;
};

extern LoggerClass Logger;

#endif  // Logger_h