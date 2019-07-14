#ifdef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <WiFi101.h>

#include "Console.h"
#include "Internet.h"

void InternetClass::connect() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi hardware not present"));
    while (true)
      ;
  }
  logLine("Attempting to connect to SSID: " + String(WIFI_SSID));
  while (WiFi.begin(WIFI_SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    log(F("."));
    delay(3000);
  }
  logLine(F("You're connected to the network"));
}

bool InternetClass::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

unsigned long InternetClass::getTime() {
  return WiFi.getTime();
}

String InternetClass::getSignalStrength() {
  long rssi = WiFi.RSSI();
  return String(rssi) + " dBm";
}

InternetClass Internet;
#endif