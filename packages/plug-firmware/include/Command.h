#ifndef Command_h
#define Command_h

#include <ArduinoJson.h>

#define AUTH_SECRET_LENGTH 12
#define BUFFER_SIZE 512

class CommandClass {
 public:
  int begin();
  void authorize(const String& encrypted);
  void unauthorize();
  uint8_t* getAuthSecret();
  void processJson(const String& json, bool isBluetooth = false);
  int32_t moveFile(const char* from, const char* to);
  int32_t copyFile(const char* from, const char* to);
  void reboot();

 private:
  String decryptToken(const String& encrypted);

  String authCmds = "";
  uint32_t authStart = 0;
  uint32_t authEnd = 0;
  uint8_t authSecret[AUTH_SECRET_LENGTH];
};

extern CommandClass Command;

#endif  // Command_h