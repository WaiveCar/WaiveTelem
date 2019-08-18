#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <WiFi101.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"
#include "System.h"

bool InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    logError("WiFi hardware not present");
    return false;
  }
  JsonObject wifi = Config.get()["wifi"];
  const char* ssid = wifi["ssid"];
  const char* password = wifi["password"];
  logDebug("ssid", ssid);
  if (WiFi.begin(ssid, password) != WL_CONNECTED) {
    logDebug("Failed to connect, try later");
    return false;
  }
  uint32_t time = 0;
  uint32_t start = millis();
  while (!time && millis() - start < 20000) {
    time = getTime();
  }
  if (time) {
    logDebug("start", String(start).c_str(), "millis()", String(millis()).c_str());
    System.setTimes(time);
  }
  logDebug("You're connected to the network");
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

InternetClass Internet;
#endif