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
  const int maxTry = 10;
  int i = 1;
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "hologram";
  Watchdog.disable();  // nbAccess.begin() can take hours to register SIM
  while ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    Serial.print(".");
    delay(3000);
    if (i == maxTry) {
      log("DEBUG", "Failed to connect, try later");
      return false;
    }
    i++;
  }
  Watchdog.enable(WATCHDOG_TIMEOUT);
  log("DEBUG", "ss", getSignalStrength());
  log("DEBUG", "carrier", nbScanner.getCurrentCarrier());
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
