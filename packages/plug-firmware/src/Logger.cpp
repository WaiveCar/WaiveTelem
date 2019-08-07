#include <Arduino.h>

#include "Logger.h"
#include "Pins.h"
#include "System.h"

void LoggerClass::setup() {
  Serial.begin(115200);
#if DEBUG
  // the following cause cause the firmware to only run if serial-monitored
  delay(5000);
#endif
  while (!SD.begin(SD_CS_PIN)) {
    logError(F("Failed to initialize SD Library"));
    delay(5000);
  }
  writeFile = SD.open("LOG.TXT", FILE_WRITE);
  if (!writeFile) {
    logError("LOG.TXT open failed");
    return;
  }
}

void LoggerClass::logLine(const char* type, const String& s) {
  const String str = System.getDateTime() + String(" ") + s;
  if (String(type) != "Debug") {
    System.telemeter(String("{\"system\":{\"last" + String(type) + "\":\"") + s + "\"}}");
  } else {
    Serial.println(str);
  }
  if (writeFile) {
    writeFile.println(str);
    writeFile.flush();
  }
}

LoggerClass Logger;