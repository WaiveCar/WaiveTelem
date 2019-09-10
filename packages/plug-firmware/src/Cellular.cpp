#ifndef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <JsonLogger.h>
#include <MKRNB.h>
#include <Modem.h>
#include <WDTZero.h>

#include "Config.h"
#include "Internet.h"
#include "System.h"

// static NB nbAccess(true);  // turn on debug
static NB nbAccess;

static GPRS gprs;
static NBScanner nbScanner;

bool InternetClass::connect() {
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "internet.swir";
  logInfo("apn", apn);
  nbAccess.setTimeout(14000);
  gprs.setTimeout(14000);
  Watchdog.setup(WDT_SOFTCYCLE32S);
  // nbAccess.begin(nb["pin"].as<char*>(), apn, true, false);
  // gprs.attachGPRS(false);
  // Watchdog.setup(WDT_OFF);
  int start = millis();
  if ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    logWarn("Failed to connect, try later");
    Watchdog.setup(WDT_SOFTCYCLE16S);
    return false;
  }
  logInfo("carrier", nbScanner.getCurrentCarrier().c_str(), "i|initTime", millis() - start);
  System.setTimes(getTime());
  Watchdog.setup(WDT_SOFTCYCLE16S);
  return true;
}

bool InternetClass::isConnected() {
  // nbAccess.ready();
  // Serial.println(String(nbReady));
  // gprs.ready();
  // Serial.println(String(gReady));
  // Serial.println(String(nbAccess.status()));
  // Serial.println(String(gprs.status()));
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

unsigned long InternetClass::getTime() {
  return nbAccess.getTime();
}

int InternetClass::getSignalStrength() {
  return nbScanner.getSignalStrength().toInt();
}

String InternetClass::getCarrier() {
  return nbScanner.getCurrentCarrier();
}

String InternetClass::getModemVersion() {
  String modemResponse = "";
  MODEM.send("ATI9");
  MODEM.waitForResponse(100, &modemResponse);
  return modemResponse;
}

InternetClass Internet;

#endif
