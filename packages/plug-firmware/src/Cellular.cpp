#ifdef ARDUINO_SAMD_MKRNB1500
#include <Arduino.h>
#include <MKRNB.h>

#include "Config.h"
#include "Console.h"
#include "Internet.h"

static NB nbAccess;
static GPRS gprs;

void InternetClass::connect() {
  log(F("Attempting to connect to the Internet network"));
  while ((nbAccess.begin(Config.getNbSimPin()) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    Serial.print(F("."));
    delay(1000);
  }
  log(F("You're connected to the Internet network"));
}

bool InternetClass::isConnected() {
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

unsigned long InternetClass::getTime() {
  return nbAccess.getTime();
}

InternetClass Internet;

#endif
