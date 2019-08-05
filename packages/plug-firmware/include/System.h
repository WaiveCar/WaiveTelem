#ifndef Status_h
#define Status_h

#include <ArduinoJson.h>

#define WATCHDOG_TIMEOUT 16 * 1000

#define STATUS_DOC_SIZE 1024
#define BUFFER_SIZE 512

class SystemClass {
 public:
  void setup();
  void poll();
  void telemeter(const String& reported, const String& desired = "");
  void sendInfo();
  void sendHeartbeat();
  void sendCanStatus();
  void setCanStatus(const String& name, uint64_t value, uint32_t delta);
  void authorizeCommand(const String& encrypted);
  String decryptToken(const String& encrypted);
  void processCommand(const String& json, bool isBluetooth = false);
  void reboot();
  void kickWatchdogAndSleep();
  uint32_t getUptime();
  int32_t moveFile(const char* from, const char* to);
  int32_t copyFile(const char* from, const char* to);

 private:
  bool canStatusChanged = false;
  int32_t lastHeartbeat = -1;
  uint32_t uptime = 0;
  StaticJsonDocument<STATUS_DOC_SIZE> statusDoc;
  String authCmds;
  uint32_t authStart;
  uint32_t authEnd;
};

extern SystemClass System;

#endif  // Status_h