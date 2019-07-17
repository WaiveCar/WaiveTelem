#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <WiFi101.h>

#include "Internet.h"
#include "Logger.h"

void InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    logError(F("WiFi hardware not present"));
    while (true)
      ;
  }
  logDebug("Attempting to connect to SSID: " + String(WIFI_SSID));
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    log(F("."));
    delay(3000);
  }
  logDebug(F("You're connected to the network"));
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