#ifndef Logger_h
#define Logger_h

#include <SD.h>

#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL 1
#endif

#ifndef LOG_ID_KEY
#define LOG_ID_KEY "i"
#endif

#ifndef LOG_TIME_KEY
#define LOG_TIME_KEY "t"
#endif

#ifndef LOG_LEVEL_KEY
#define LOG_LEVEL_KEY "l"
#endif

#ifndef LOG_SOURCE_KEY
#define LOG_SOURCE_KEY "s"
#endif

#ifndef LOG_FUNC_KEY
#define LOG_FUNC_KEY "f"
#endif

#ifndef LOG_MESSAGE_KEY
#define LOG_MESSAGE_KEY "m"
#endif

#define logError(...) logKv(4, __VA_ARGS__)
#define logWarn(...) logKv(3, __VA_ARGS__)
#define logInfo(...) logKv(2, __VA_ARGS__)
#define logDebug(...) logKv(1, __VA_ARGS__)

#define logKv(level, ...)                                                                                         \
  if (level >= LOG_MIN_LEVEL) Logger.logKeyValueJson(level, "", "s", (String(__FILE__) + ":" + __LINE__).c_str(), \
                                                     "f", (String(__func__) + "()").c_str(), __VA_ARGS__, NULL)

class LoggerClass {
 public:
  int begin();
  int logKeyValueJson(int level, const char* placeholder, ...);

 private:
  File writeFile;
  uint8_t mqttLevel;
};

extern LoggerClass Logger;

#endif  // Logger_h