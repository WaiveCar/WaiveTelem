#ifndef ARDUINO_SAMD_MKR1000
#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <MKRNB.h>

#include "Config.h"
#include "Internet.h"
#include "Logger.h"
#include "System.h"

// #ifdef DEBUG
// static NB nbAccess(true);  // turn on debug
// #else
static NB nbAccess;
// #endif

static GPRS gprs;
static NBScanner nbScanner;

bool InternetClass::connect() {
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "hologram";
  logInfo("apn", apn);
  nbAccess.setTimeout(3000);
  gprs.setTimeout(10000);
  // Watchdog.disable();
  if ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    // Watchdog.enable(WATCHDOG_TIMEOUT);
    logWarn("Failed to connect, try later");
    return false;
  }
  // Watchdog.enable(WATCHDOG_TIMEOUT);
  logInfo("carrier", nbScanner.getCurrentCarrier().c_str());
  System.setTimes(getTime());
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
