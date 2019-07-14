#ifndef Console_h
#define Console_h

#ifdef DEBUG
#define log(s) Serial.print(s)
#define logLine(s)        \
  Serial.print(F("# ")); \
  Serial.println(s)
#else
#define log(s)
#define logLine(s)
#endif

class ConsoleClass {
 public:
  void setup();
  void logFreeMemory();
};

extern ConsoleClass Console;

#endif  // Console_h