#ifndef Logger_h
#define Logger_h

#include <SD.h>

#include "Mqtt.h"
#include "System.h"

#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL 1
#endif

// #define LOG_EASYREAD_SERIAL // in your build option if you want more readable SERIAL output (but no longer JSON)

#ifndef ID_KEY
#define ID_KEY "i"
#endif

#ifndef ID_VALUE
#define ID_VALUE System.getId()
#endif

#ifndef TIME_KEY
#define TIME_KEY "t"
#endif

#ifndef TIME_VALUE
#define TIME_VALUE System.getDateTime()
#endif

// can still generate json > LOG_RESERVE_SIZE, just takes slighly longer
#ifndef LOG_RESERVE_SIZE
#define LOG_RESERVE_SIZE 256
#endif

#define logError(...) logKv(4, __VA_ARGS__)
#define logWarn(...) logKv(3, __VA_ARGS__)
#define logInfo(...) logKv(2, __VA_ARGS__)
#define logDebug(...) logKv(1, __VA_ARGS__)

#define logKv(level, ...)                                                                                     \
  if (level >= LOG_MIN_LEVEL) Logger.logKeyValueJson(level, "s", (String(__FILE__) + ":" + __LINE__).c_str(), \
                                                     "f", (String(__func__) + "()").c_str(), __VA_ARGS__, NULL)

class LoggerClass {
 public:
  void begin();
  void logKeyValueJson(int level, const char* key, ...);

 private:
  File writeFile;
  uint8_t mqttLevel;
};

extern LoggerClass Logger;

#endif  // Logger_h