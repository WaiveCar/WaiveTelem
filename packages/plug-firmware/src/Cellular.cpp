#ifndef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <MKRNB.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"

static NB nbAccess(true);  // turn on debug
// static NB nbAccess;
static GPRS gprs;
static NBScanner nbScanner;

void InternetClass::connect() {
  logDebug(F("Attempting to connect to the Internet"));
  JsonObject nb = Config.get()["nb"];
  while ((nbAccess.begin(nb["pin"].as<char*>(), nb["apn"].as<char*>()) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    log(F("."));
    delay(1000);
  }
  logDebug(F("You're connected to the Internet"));
  logDebug("Signal Strength: " + String(getSignalStrength()));
  logDebug("Current Carrier: " + nbScanner.getCurrentCarrier());
  // logDebug("IP Address: " + String(gprs.getIPAddress(), 16));
}

bool InternetClass::isConnected() {
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

unsigned long InternetClass::getTime() {
  return nbAccess.getTime();
}

int InternetClass::getSignalStrength() {
  if (isConnected()) {
    return nbScanner.getSignalStrength().toInt();
  } else {
    return 0;
  }
}

InternetClass Internet;

#endif
