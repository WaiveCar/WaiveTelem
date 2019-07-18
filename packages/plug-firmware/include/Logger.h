#ifndef Logger_h
#define Logger_h

#include <SD.h>
#include <stdio.h>

#include "Gps.h"
#include "Mqtt.h"

#define logError(s) Logger.logLine(String("ERROR ") + s)
#define logInfo(s) Logger.logLine(String("INFO ") + s)

#ifdef DEBUG
#define log(s) Serial.print(s)
#define logDebug(s) Logger.logLine(String("DEBUG ") + s)
#else
#define log(s)
#define logDebug(s)
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