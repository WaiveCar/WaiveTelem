#ifndef Status_h
#define Status_h

#define BUFFER_SIZE 512

class SystemClass {
 public:
  void setup();
  void poll();
  void sendVersion();
  void sendHeartbeat();
  void sendCanStatus();
  void setCanStatus(const String& name, const uint64_t value);
  void processCommand(const String& json);
  void reboot();
  int32_t moveFile(const char* from, const char* to);
  int32_t copyFile(const char* from, const char* to);

 private:
  bool canStatusChanged = false;
  uint32_t lastHeartbeat = 0;
  StaticJsonDocument<1024> statusDoc;
};

extern SystemClass System;

#endif  // Status_h