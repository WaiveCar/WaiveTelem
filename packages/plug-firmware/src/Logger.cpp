#include <Arduino.h>
#include <json_builder.h>

#include "Config.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#ifndef LOG_ID_VALUE
#error You must define LOG_ID_VALUE in build_flags (e.g. -D LOG_ID_VALUE=System.getId())which is used by logger.cpp to log device id
#endif

#ifndef LOG_TIME_VALUE
#error You must define LOG_ID_VALUEin build_flags (e.g. -D LOG_ID_VALUE=System.getDateTime()) which is used by logger.cpp to log time string
#endif

// #ifndef LOG_EASYREAD_SERIAL
// #pragma message "define LOG_EASYREAD_SERIAL in build_flags (e.g. -D LOG_EASYREAD_SERIAL) if you want more readable SERIAL output (but no longer JSON)"
// #endif

// can still generate json > LOG_RESERVE_SIZE, just takes slighly longer
#ifndef LOG_RESERVE_SIZE
#define LOG_RESERVE_SIZE 256
#endif

const char* LEVELS[5] = {"", "DEBUG", "INFO", "WARN", "ERROR"};

int LoggerClass::begin() {
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logError("LOG.TXT open failed");
    return -1;
  }
  mqttLevel = Config.get()["logger"]["mqttLevel"] | 1;
  return 1;
}

int LoggerClass::logKeyValueJson(int level, const char* placeholder, ...) {
  char fragment[128], jstr[512];
  json(fragment, "-{", LOG_ID_KEY, LOG_ID_VALUE, LOG_TIME_KEY, LOG_TIME_VALUE, LOG_LEVEL_KEY, LEVELS[level]);
  va_list args;
  va_start(args, placeholder);
  int ret = vbuild_json(jstr, 512, fragment, args);
  va_end(args);

  if (level >= mqttLevel) {
    if (Mqtt.isConnected()) {
      Mqtt.logMsg(jstr);
      if (level == 4) {
        String escapedJson = jstr;
        escapedJson.replace("\"", "\\\"");
        System.report("{\"system\":{\"lastError\":\"" + escapedJson + "\"}}");
      }
    }
  }
  if (writeFile) {
    writeFile.println(jstr);
    writeFile.flush();
  }

#if LOG_EASYREAD_SERIAL
  String jstring = jstr;
  jstring.replace(String("{\"" LOG_ID_KEY "\":"), "");
  jstring.replace(String("\"") + LOG_ID_VALUE + "\"", "");
  jstring.replace(String(",\"" LOG_TIME_KEY "\":\""), "");
  jstring.replace(String("\",\"" LOG_LEVEL_KEY "\":\""), strlen(LEVELS[level]) == 5 ? " " : "  ");
  jstring.replace(String("\",\"" LOG_SOURCE_KEY "\":\""), " ");
  jstring.replace(String("\",\"" LOG_FUNC_KEY "\":\""), " ");

  jstring.replace("\"", " ");
  jstring.replace("\\", "");

  Serial.println(jstring);
#else
  Serial.println(jstr);
#endif
  return ret;
}

LoggerClass Logger;
