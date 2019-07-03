#include <Arduino.h>
#include <TinyGPS++.h>

#include "Console.h"
#include "Gps.h"

TinyGPSPlus gps;

void GpsClass::setup() {
  Serial1.begin(9600);
  while (!Serial1) {
    log(F("G"));
    delay(1000);
  }
}

GpsClass Gps;