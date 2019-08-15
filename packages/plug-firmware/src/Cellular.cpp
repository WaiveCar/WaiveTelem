#ifndef ARDUINO_SAMD_MKR1000
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <MKRNB.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"
#include "System.h"

// static NB nbAccess(true);  // turn on debug
static NB nbAccess;

static GPRS gprs;
static NBScanner nbScanner;

bool InternetClass::connect() {
  logDebug(F("Attempting to connect to the Internet"));
  const int maxTry = 10;
  int i = 1;
  JsonObject nb = Config.get()["nb"];
  Watchdog.disable();  // nbAccess.begin() can take hours to register SIM
  while ((nbAccess.begin(nb["pin"].as<char*>(), nb["apn"].as<char*>()) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    log(F("."));
    delay(3000);
    if (i == maxTry) {
      logDebug(F("Failed to connect, try later"));
      return false;
    }
    i++;
  }
  Watchdog.enable(WATCHDOG_TIMEOUT);
  logDebug(F("You're connected to the Internet"));
  logDebug("Signal Strength: " + String(getSignalStrength()));
  logDebug("Current Carrier: " + nbScanner.getCurrentCarrier());
  // logDebug("IP Address: " + String(gprs.getIPAddress(), 16));
  return true;
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
