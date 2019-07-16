#include <Arduino.h>
#include <NMEAGPS.h>

#include "Console.h"
#include "Gps.h"

#define GPSSerial Serial1

static NMEAGPS gps;

void GpsClass::setup() {
  GPSSerial.begin(9600);
  while (!GPSSerial) {
    log(F("G"));
    delay(1000);
  }
  data = "";
}

void GpsClass::poll() {
  gps_fix fix;
  while (gps.available(GPSSerial)) {
    fix = gps.read();
    if (fix.valid.location && fix.valid.date && fix.valid.time) {
      NeoGPS::time_t dt = fix.dateTime;
      char latitude[16], longitude[16], time[32];
      sprintf(latitude, "%0.6f", fix.latitudeL() / 1e7);
      sprintf(longitude, "%0.6f", fix.longitudeL() / 1e7);
      sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
      data = "\"gps\": {\"lat\": " + String(latitude) + ", \"long\": " + longitude + ", \"time\": \"" + time + "\"}";
    }
  }
}

const String& GpsClass::getData() {
  return data;
}

GpsClass Gps;