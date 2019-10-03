#ifndef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <JsonLogger.h>
#include <MKRNB.h>
#include <Modem.h>
#include <WDTZero.h>

#include "Config.h"
#include "Internet.h"
#include "System.h"

static NB nbAccess;

static GPRS gprs;
static NBScanner nbScanner;

bool InternetClass::connect() {
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "internet.swir";
  logInfo("apn", apn);
  nbAccess.setTimeout(20000);
  gprs.setTimeout(20000);
  Watchdog.setup(WDT_SOFTCYCLE32S);
  // Watchdog.setup(WDT_OFF);
  MODEM.debug(Serial);
  int start = millis();
  if ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    logWarn("Failed to connect, try later");
    Watchdog.setup(WDT_SOFTCYCLE8S);
    return false;
  }
  MODEM.noDebug();

  // to fix long mqtt cmd delay:
  // https://portal.u-blox.com/s/question/0D52p00008RlYDrCAN/long-delays-using-sarar41002b-with-att
  MODEM.send("AT+USOSO=0,6,1,1");
  MODEM.waitForResponse();
  MODEM.send("AT+USOSO=0,65535,8,1");
  MODEM.waitForResponse();
  MODEM.send("AT+CEDRXS=0");  // this is supposed to be stored in non-volatile memory, but it seems AT&T can flip it?
  MODEM.waitForResponse();

  System.setTimes(getTime());
  logInfo("carrier", nbScanner.getCurrentCarrier().c_str(), "i|initTime", millis() - start, "i|signal", getSignalStrength());
  Watchdog.setup(WDT_SOFTCYCLE8S);
  return true;
}

bool InternetClass::isConnected() {
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
  MODEM.waitForResponse(1000, &modemResponse);
  return modemResponse;
}

InternetClass Internet;

#endif
