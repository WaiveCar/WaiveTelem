#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <JsonLogger.h>
#include <WiFi101.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"
#include "System.h"

int InternetClass::begin() {
}

bool InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    logError("WiFi hardware not present");
    return false;
  }
  JsonObject wifi = Config.get()["wifi"];
  const char* ssid = wifi["ssid"];
  const char* password = wifi["password"];
  logInfo("ssid", ssid);
  if (WiFi.begin(ssid, password) != WL_CONNECTED) {
    logWarn("Failed to connect, try later");
    return false;
  }
  uint32_t time = 0;
  uint32_t start = millis();
  while (!time /* && millis() - start < 14000 */) {  //if it doesn't connect, might need battery power
    delay(10);
    time = getTime();
  }
  logDebug("i|start", start, "i|millis()", millis(), "i|time", time);
  if (time) {
    System.setTimes(time);
  }
  logInfo("You're connected to the network");
  return true;
}

bool InternetClass::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

unsigned long InternetClass::getTime() {
  return WiFi.getTime();
}

int InternetClass::getSignalStrength() {
  return WiFi.RSSI();
}

String InternetClass::getCarrier() {
  return String(WiFi.SSID());
}

InternetClass Internet;
#endif