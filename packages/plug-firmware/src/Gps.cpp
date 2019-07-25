#include <Arduino.h>
#include <NMEAGPS.h>

#include "Gps.h"
#include "Logger.h"

#define GPSSerial Serial1

static NMEAGPS gps;

#define PMTK_SET_BAUD_115200 "$PMTK251,115200*1F"                                        ///< 115200 bps
#define PMTK_SET_BAUD_57600 "$PMTK251,57600*2C"                                          ///<  57600 bps
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"  ///< turn on GPRMC and GPGGA

/*
#define PMTK_STANDBY "$PMTK161,0*28"  ///< standby command & boot successful message
#define PMTK_AWAKE "$PMTK010,002*2D"  ///< Wake up

boolean Adafruit_GPS::standby(void) {
  if (inStandbyMode) {
    return false;  // Returns false if already in standby mode, so that you do not wake it up by sending commands to GPS
  } else {
    inStandbyMode = true;
    sendCommand(PMTK_STANDBY);
    //return waitForSentence(PMTK_STANDBY_SUCCESS);  // don't seem to be fast enough to catch the message, or something else just is not working
    return true;
  }
}

boolean Adafruit_GPS::wakeup(void) {
  if (inStandbyMode) {
    inStandbyMode = false;
    sendCommand("");  // send byte to wake it up
    return waitForSentence(PMTK_AWAKE);
  } else {
    return false;  // Returns false if not in standby mode, nothing to wakeup
  }
}
*/

void GpsClass::setup() {
  digitalWrite(GPS_RESET, LOW);
  delay(1000);
  GPSSerial.begin(9600);
  // GPSSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // GPSSerial.println(PMTK_SET_BAUD_57600);
  // delay(200);
  // GPSSerial.end();
  // delay(200);
  // GPSSerial.begin(57600);
}

void GpsClass::poll() {
  while (GPSSerial.available()) {
    logDebug((char*)GPSSerial.read());
  }
  return;
  int start = millis();
  gps_fix fix;
  bool noData = true;
  // try at most 1 second
  while (noData && millis() - start < 1000) {
    while (gps.available(GPSSerial)) {
      fix = gps.read();
      if (fix.valid.location && fix.valid.date && fix.valid.time && fix.valid.speed && fix.valid.heading) {
        latitude = fix.latitudeL();
        longitude = fix.longitudeL();
        time = fix.dateTime;
        speed = fix.speed_mph();
        heading = fix.heading();
        NeoGPS::time_t dt = fix.dateTime;
        sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
        // logDebug(time);
        noData = false;
        break;
      }
    }
  }
}

int GpsClass::getLatitude() {
  return latitude;
}

int GpsClass::getLongitude() {
  return longitude;
}

float GpsClass::getSpeed() {
  return speed;
}

float GpsClass::getHeading() {
  return heading;
}

int GpsClass::getTime() {
  return time;
}
const char* GpsClass::getDateTime() {
  return dateTime;
}

GpsClass Gps;