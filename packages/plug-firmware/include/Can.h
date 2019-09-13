#ifndef Can_h
#define Can_h

#include <ArduinoJson.h>

class CanClass {
 public:
  int begin();
  void poll();
  int getHealth();
  void sendCommand(const char* cmd);
  void sleep();
  bool isSleeping(int bus);

 private:
  int health = 0;
#ifdef ARDUINO_SAMD_WAIVE1000
  bool sleeping[2] = {false, false};
#else
  bool sleeping[1] = {false};
#endif
  uint8_t busCount = 0;
};

extern CanClass Can;

#endif  // Can_h