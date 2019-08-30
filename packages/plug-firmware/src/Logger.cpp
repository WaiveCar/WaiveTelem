#include <Arduino.h>
#include <JsonLogger.h>

#include "Config.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

extern "C" {
const char* getLogId() {
  return System.getId();
}

const char* getLogTime() {
  return System.getDateTime();
}
}

void toConsole(int level, const char* json) {
  char mod[LOG_MAX_LEN];
  strcpy(mod, json);
  logModifyForHuman(level, mod);

  Serial.println(mod);
}

void toFile(int level, const char* json) {
  File writeFile = Logger.getWriteFile();
  writeFile.println(json);
  writeFile.flush();
  int error = writeFile.getWriteError();
  if (error) {
    logError("i|error", error, "cannot write to LOG.TXT");
    writeFile.close();
  }
}

void toMqtt(int level, const char* json) {
  if (Mqtt.isConnected()) {
    if (level >= System.getRemoteLogLevel()) {
      Mqtt.logMsg(json);
    }
  }
}

int LoggerClass::begin() {
  logAddSender(toConsole);
  logAddSender(toMqtt);

  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logError("LOG.TXT open failed");
    return -1;
  }

  logAddSender(toFile);

  return 1;
}

File LoggerClass::getWriteFile() {
  return writeFile;
}

LoggerClass Logger;
