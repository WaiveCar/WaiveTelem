#include <Arduino.h>

#include "Logger.h"

void LoggerClass::setup() {
  Serial.begin(9600);
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logLine("LOG.TXT open failed");
    return;
  }
}

void LoggerClass::logLine(const String& s) {
  const String str = Gps.getTime() + String(" ") + s;
  if (Mqtt.isConnected()) {
    Mqtt.telemeter(String("{\"lastLog\": \"") + str + "\"}");
  } else {
    Serial.println(str);
  }
  if (writeFile) {
    writeFile.println(str);
    writeFile.flush();
  }
}

LoggerClass Logger;