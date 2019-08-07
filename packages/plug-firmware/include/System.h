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
  void setTime(uint32_t in);
  const char* getDateTime();
  void telemeter(const String& reported, const String& desired = "");
  void sendInfo();
  void sendHeartbeat();
  void sendCanStatus();
  void setCanStatus(const String& name, uint64_t value, uint32_t delta);
  void authorizeCommand(const String& encrypted);
  void unauthorize();
  String decryptToken(const String& encrypted);
  void reportCommandDone(const String& json, String& cmdKey, String& cmdValue);
  void processCommand(const String& json, bool isBluetooth = false);
  void reboot();
  void sleep();
  int32_t moveFile(const char* from, const char* to);
  int32_t copyFile(const char* from, const char* to);

 private:
  bool canStatusChanged = false;
  int32_t lastHeartbeat = -1;
  uint32_t bootTime = 0;
  uint32_t time = 0;
  StaticJsonDocument<STATUS_DOC_SIZE> statusDoc;
  unsigned char tokenKey[32];
  unsigned char tokenIv[16];
  String authCmds = "";
  uint32_t authStart = 0;
  uint32_t authEnd = 0;
  char dateTime[32] = "";
};

extern SystemClass System;

#endif  // Status_h