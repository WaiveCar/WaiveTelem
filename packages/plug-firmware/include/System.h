#ifndef Status_h
#define Status_h

#define STATUS_DOC_SIZE 1024
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
  void kickWatchdogAndSleep(int msec);
  uint64_t getMillis();
  int32_t moveFile(const char* from, const char* to);
  int32_t copyFile(const char* from, const char* to);

 private:
  bool canStatusChanged = false;
  uint32_t lastHeartbeat = 0;
  uint64_t millis = 0;
  StaticJsonDocument<STATUS_DOC_SIZE> statusDoc;
};

extern SystemClass System;

#endif  // Status_h