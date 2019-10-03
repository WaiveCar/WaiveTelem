#ifndef System_h
#define System_h

#ifndef DEBUG
#include <RTCZero.h>
#endif

#include "HashMap/HashMap.h"

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
  void setCanStatus(const char* name, int64_t value, uint32_t delta);
  void sleep();
  bool stayResponsive();
  void setStayResponsive(bool responsive);
  void reportCommandDone(const String& lastCmd, const String& cmdKey, const String& cmdValue = "");
  void setRemoteLogLevel(int8_t in);
  int8_t getRemoteLogLevel();
  void simulateIgnition(const String& cmdValue);

 private:
  void checkVin();
  void checkHeartbeat();
  void handleIgnitionOn();
  void handleIgnitionOff();

  char id[19];
  int8_t remoteLogLevel = 4;
  int32_t lastHeartbeat = -1;
  uint32_t bootTime = 0;
  uint32_t time = 0;
  char dateTime[32] = "";
  bool stayresponsive = false;
#ifndef DEBUG
  RTCZero rtc;
#endif
  volatile uint32_t lastMillis = 0;
  uint32_t vinReads[5] = {0};
  int vinIndex = 0;
  bool vinAvgValid = false;
  int32_t lastVinRead = -1;
  HashMap<const char*, int64_t, 40> canBus;
  HashMap<const char*, int64_t, 20> canBusLessThanDelta;
  HashMap<const char*, int64_t, 20> canBusBatch;
};

extern SystemClass System;

#endif  // System_h