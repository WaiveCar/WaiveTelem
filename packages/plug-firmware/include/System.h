#ifndef Status_h
#define Status_h

class SystemClass {
 public:
  void setup();
  void poll();
  void sendVersion();
  void sendHeartbeat();
  void sendCanStatus();
  void setCanStatus(const String& name, const uint64_t value);
  void setInRide(bool in);
  bool getInRide();
  void processCommand(const String& json);

 private:
  String getStatus();
  bool inRide;
  uint32_t lastHeartbeat;
  StaticJsonDocument<2048> jsonDoc;
  bool canStatusChanged;
};

extern SystemClass System;

#endif  // Status_h