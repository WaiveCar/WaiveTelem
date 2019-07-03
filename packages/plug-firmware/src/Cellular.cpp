#ifdef ARDUINO_SAMD_MKRNB1500
#include <Arduino.h>
#include <MKRNB.h>

#include "Cellular.h"
#include "Console.h"

NB nbAccess;
GPRS gprs;

void CellularClass::connect() {
  log(F("Attempting to connect to the cellular network"));
  while ((nbAccess.begin("") != NB_READY) || (gprs.attachGPRS("hologram") != GPRS_READY)) {
    Serial.print(F("."));
    delay(1000);
  }
  log(F("You're connected to the cellular network"));
}

bool CellularClass::isConnected() {
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

CellularClass Cellular;

#endif
