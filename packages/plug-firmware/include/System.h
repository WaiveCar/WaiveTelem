#ifndef System_h
#define System_h

#include <ArduinoJson.h>
#include <RTCZero.h>

#define WATCHDOG_TIMEOUT 16 * 1000
#define STATUS_DOC_SIZE 1024

class SystemClass {
 public:
  const char* getId();
  void setup();
  void poll();
  void setTimes(uint32_t in);
  uint32_t getTime();
  const char* getDateTime();
  void keepTime();
  void report(const String& reported, const String& desired = "");
  void sendInfo();
  void sendHeartbeat();
  void sendCanStatus();
  void setCanStatus(const String& name, uint64_t value, uint32_t delta);
  void sleep(uint32_t sec);
  bool stayAwake();
  void setStayAwake(bool stay);
  void setCanStatusChanged();
  void reportCommandDone(const String& json, String& cmdKey, String& cmdValue);

 private:
  char id[19];
  bool canStatusChanged = false;
  int32_t lastHeartbeat = -1;
  uint32_t bootTime = 0;
  uint32_t time = 0;
  char dateTime[32] = "";
  StaticJsonDocument<STATUS_DOC_SIZE> statusDoc;
  bool stayawake = false;
  RTCZero rtc;
  uint32_t lastMillis = 0;
};

extern SystemClass System;

#endif  // System_h