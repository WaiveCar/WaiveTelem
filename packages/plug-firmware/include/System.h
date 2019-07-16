#ifndef Status_h
#define Status_h

class SystemClass {
 public:
  void setup();
  void poll();
  void sendVersion();
  void sendHeartbeat();
  void setInRide(bool in);
  bool getInRide();
  void processCommand(String& json);

 private:
  String getStatus();
  bool inRide;
  u_int32_t lastSentTime;
};

extern SystemClass System;

#endif  // Status_h