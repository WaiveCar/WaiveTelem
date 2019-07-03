#include <Arduino.h>

#include "Console.h"
#include "Gps.h"

void GpsClass::setup() {
  Serial1.begin(9600);
  while (!Serial1) {
    log("GPS Loading...");
    delay(1000);
  }
}

GpsClass Gps;