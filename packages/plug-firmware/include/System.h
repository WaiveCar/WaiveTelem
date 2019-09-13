#ifndef System_h
#define System_h

#include <ArduinoJson.h>
#include <RTCZero.h>

#define STATUS_DOC_SIZE 1024

class SystemClass {
 public:
  int begin();
  const char* getId();
  void poll();
  void setTimes(uint32_t in);
  uint32_t getTime();
  const char* getDateTime();
  void keepTime();
  void report(const char* reported, const char* desired = NULL);
  void sendInfo(const char* sysJson);
  void resetLastHeartbeat();
  void sendHeartbeat();
  void sendCanStatus(const char* type);
  bool setCanStatus(const char* name, uint64_t value, uint32_t delta);
  void sleep(uint32_t sec);
  bool stayResponsive();
  void setStayResponsive(bool responsive);
  void reportCommandDone(const String& lastCmd, const String& cmdKey, const String& cmdValue = "");
  void setRemoteLogLevel(int8_t in);
  int8_t getRemoteLogLevel();

 private:
  void checkVin();
  void checkHeartbeat();

  char id[19];
  int8_t remoteLogLevel = 4;
  int32_t lastHeartbeat = -1;
  uint32_t bootTime = 0;
  uint32_t time = 0;
  char dateTime[32] = "";
  StaticJsonDocument<STATUS_DOC_SIZE> statusDoc;
  bool stayresponsive = false;
  RTCZero rtc;
  uint32_t lastMillis = 0;
  uint32_t vinReads[5] = {0};
  int vinIndex = 0;
  bool vinAvgValid = false;
  int32_t lastVinRead = -1;
};

extern SystemClass System;

#endif  // System_h