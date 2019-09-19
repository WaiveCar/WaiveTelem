#include <Arduino.h>
#include <JsonLogger.h>

#include "Logger.h"
#include "Mqtt.h"
#include "System.h"

#define LOG_FILE "LOG.TXT"

extern "C" {
const char* getLogId() {
  return System.getId();
}

const char* getLogTime() {
  return System.getDateTime();
}
}

void toConsole(int level, const char* json, int len) {
  char mod[LOG_MAX_LEN];
  memcpy(mod, json, len + 1);
  logModifyForHuman(level, mod);

  Serial.println(mod);
}

void toFile(int level, const char* json, int len) {
  File writeFile = Logger.getWriteFile();
  if (writeFile) {
    writeFile.println(json);
    writeFile.flush();
    int error = writeFile.getWriteError();
    if (error) {
      logError("i|error", error, "cannot write to " LOG_FILE);
      writeFile.close();
    }
  }
}

void toMqtt(int level, const char* json, int len) {
  if (Mqtt.isConnected()) {
    if (level >= System.getRemoteLogLevel()) {
      Mqtt.logMsg(json, len);
    }
  }
}

int LoggerClass::begin() {
  logAddSender(toConsole);
  logAddSender(toMqtt);

  writeFile = SD.open(LOG_FILE, FILE_WRITE);
  if (!writeFile) {
    logError(LOG_FILE " open failed");
    return -1;
  }
  logAddSender(toFile);

  return 1;
}

File& LoggerClass::getWriteFile() {
  return writeFile;
}

LoggerClass Logger;
