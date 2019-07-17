#include <Arduino.h>
#include <NMEAGPS.h>

#include "Gps.h"
#include "Logger.h"

#define GPSSerial Serial1

static NMEAGPS gps;

void GpsClass::setup() {
  GPSSerial.begin(9600);
  while (!GPSSerial) {
    log(F("G"));
    delay(1000);
  }
}

void GpsClass::poll() {
  gps_fix fix;
  while (gps.available(GPSSerial)) {
    fix = gps.read();
    if (fix.valid.location && fix.valid.date && fix.valid.time) {
      latitude = fix.latitudeL();
      longitude = fix.longitudeL();
      NeoGPS::time_t dt = fix.dateTime;
      sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
      // logDebug(time);
    }
  }
}

float GpsClass::getLatitude() {
  return latitude;
}

float GpsClass::getLongitude() {
  return longitude;
}

const char* GpsClass::getTime() {
  return time;
}

GpsClass Gps;