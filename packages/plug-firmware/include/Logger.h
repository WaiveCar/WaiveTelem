#ifndef Logger_h
#define Logger_h

#include <SD.h>

#include "Gps.h"
#include "Mqtt.h"

#define logError(s) Logger.logLine("Error", s)
#define logInfo(s) Logger.logLine("Info", s)

#ifdef DEBUG
#define log(s) Serial.print(s)
#define logDebug(s) Logger.logLine("Debug", s)
#define logFunc() Logger.logLine("Debug", String(__FILE__) + ":" + __LINE__ + " " + __func__ + "()")
#else
#define log(s)
#define logDebug(s)
#define logFunc(s)
#endif

class LoggerClass {
 public:
  void setup();
  void logFreeMemory();
  void logLine(const char* type, const String& s);

 private:
  File writeFile;
};

extern LoggerClass Logger;

#endif  // Logger_h