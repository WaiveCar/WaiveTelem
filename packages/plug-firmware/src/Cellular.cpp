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
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "hologram";
  if ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    log("WARN ", "Failed to connect, try later");
    return false;
  }
  log("DEBUG", "ss", String(getSignalStrength()).c_str());
  log("INFO ", "carrier", nbScanner.getCurrentCarrier().c_str());
  // log("DEBUG", "IP Address: " + String(gprs.getIPAddress(), 16));
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
