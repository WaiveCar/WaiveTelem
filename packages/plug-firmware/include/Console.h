#ifndef Console_h
#define Console_h

#ifdef DEBUG
#define log(s) Serial.println(s)
#else
#define log(s)
#endif

class ConsoleClass {
 public:
  void setup();
};

extern ConsoleClass Console;

#endif  // Console_h