#ifndef Logger_h
#define Logger_h

#include <SD.h>

#include "Gps.h"
#include "Mqtt.h"

#ifdef DEBUG
#define log(...) Serial.print(__VA_ARGS__)
#define logDebug(...) Logger.logLine(__VA_ARGS__)
#else
#define log(...)
#define logDebug(...)
#endif

class LoggerClass {
 public:
  void setup();
  void logFreeMemory();
  void logLine(const String& s);

 private:
  File writeFile;
};

extern LoggerClass Logger;

#endif  // Logger_h