#ifdef ARDUINO_SAMD_MKR1000
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <WiFi101.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"

bool InternetClass::connect() {
  logFunc();
  if (WiFi.status() == WL_NO_SHIELD) {
    logError(F("WiFi hardware not present"));
    return false;
  }
  const int maxTry = 5;
  int i = 1;
  JsonObject wifi = Config.get()["wifi"];
  const char* ssid = wifi["ssid"];
  const char* password = wifi["password"];
  logDebug("Attempting to connect to SSID: " + String(ssid));
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Watchdog.reset();
    log(F("."));
    if (i == maxTry) {
      logDebug(F("Failed to connect, try later"));
      return false;
    }
    i++;
  }
  logDebug(F("You're connected to the network"));
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