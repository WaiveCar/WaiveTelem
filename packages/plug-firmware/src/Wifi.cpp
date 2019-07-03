#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <WiFi101.h>

#include "Console.h"
#include "Wifi.h"

void WifiClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true)
      ;
  }
  log("Attempting to connect to SSID: " + String(SSID_NAME));
  while (WiFi.begin(SSID_NAME, SSID_PASWORD) != WL_CONNECTED) {
    log(".");
    delay(5000);
  }
  log("You're connected to the network");
}

bool WifiClass::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

WifiClass Wifi;
#endif