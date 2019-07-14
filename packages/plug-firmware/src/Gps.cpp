#include <Arduino.h>
#include <NMEAGPS.h>

#include "Config.h"
#include "Console.h"
#include "Gps.h"
#include "Mqtt.h"
#include "System.h"

#define GPSSerial Serial1

static NMEAGPS gps;

void GpsClass::setup() {
  GPSSerial.begin(9600);
  while (!GPSSerial) {
    log(F("G"));
    delay(1000);
  }

  lastSentTime = 0;
}

void GpsClass::poll() {
  gps_fix fix;
  while (gps.available(GPSSerial)) {
    fix = gps.read();
    if (fix.valid.location && fix.valid.date && fix.valid.time) {
      int interval = Config.getGpsInterval();
      if (lastSentTime == 0 || (interval >= 0 && millis() - lastSentTime > (u_int32_t)interval)) {
        NeoGPS::time_t dt = fix.dateTime;
        char latitude[16], longitude[16], time[32];
        sprintf(latitude, "%0.6f", fix.latitudeL() / 1e7);
        sprintf(longitude, "%0.6f", fix.longitudeL() / 1e7);
        sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
        Mqtt.telemeter("{" + System.getStatus() + "\"gps\": {\"lat\": " + String(latitude) + ", \"long\": " + longitude + ", \"time\": \"" + time + "\"" + "}}");
        lastSentTime = millis();
      }
    }
  }
}

GpsClass Gps;