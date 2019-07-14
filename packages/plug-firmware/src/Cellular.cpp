#ifdef ARDUINO_SAMD_MKRNB1500
#include <Arduino.h>
#include <MKRNB.h>

#include "Config.h"
#include "Console.h"
#include "Internet.h"

static NB nbAccess;
static GPRS gprs;
static NBScanner nbScanner;

void InternetClass::connect() {
  log(F("Attempting to connect to the Internet"));
  while ((nbAccess.begin(Config.getNbSimPin()) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    Serial.print(F("."));
    delay(1000);
  }
  log(F("You're connected to the Internet"));
}

bool InternetClass::isConnected() {
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

unsigned long InternetClass::getTime() {
  return nbAccess.getTime();
}

String InternetClass::getSignalStrength() {
  return nbScanner.getSignalStrength();
}

InternetClass Internet;

#endif
