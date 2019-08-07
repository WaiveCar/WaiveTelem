#ifdef ARDUINO_SAMD_MKR1000
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <WiFi101.h>

#include "Internet.h"
#include "Logger.h"

bool InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    logError(F("WiFi hardware not present"));
    return false;
  }
  logDebug("Attempting to connect to SSID: " + String(WIFI_SSID));
  const int maxTry = 5;
  int i = 1;
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
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