#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <WiFi101.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"

bool InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    log("ERROR", "WiFi hardware not present");
    return false;
  }
  JsonObject wifi = Config.get()["wifi"];
  const char* ssid = wifi["ssid"];
  const char* password = wifi["password"];
  log("DEBUG", "ssid", ssid);
  if (WiFi.begin(ssid, password) != WL_CONNECTED) {
    log("DEBUG", "Failed to connect, try later");
    return false;
  }
  log("DEBUG", "You're connected to the network");
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