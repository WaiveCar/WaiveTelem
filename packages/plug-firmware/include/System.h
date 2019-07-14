#ifndef Status_h
#define Status_h

class SystemClass {
 public:
  void setup();
  void sendVersion();
  void setInRide(bool in);
  bool getInRide();
  String getStatus();
  void processCommand(String& json);

 private:
  bool inRide;
};

extern SystemClass System;

#endif  // Status_h