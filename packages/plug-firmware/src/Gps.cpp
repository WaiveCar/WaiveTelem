#include <Arduino.h>
#include <NMEAGPS.h>
#include <ublox/ubxGPS.h>

#include "Gps.h"
#include "Logger.h"

#define GPSSerial Serial1

#define COMMAND_DELAY 250

static ubloxGPS gps(&GPSSerial);
// static NMEAGPS gps;

void GpsClass::setup() {
  const char baud115200[] PROGMEM = "PUBX,41,1,3,3,115200,0";
  // we only want RMC, GGA and GSV
  const char disableGLL[] PROGMEM = "PUBX,40,GLL,0,0,0,0,0,0";
  const char disableGSA[] PROGMEM = "PUBX,40,GSA,0,0,0,0,0,0";
  const char disableVTG[] PROGMEM = "PUBX,40,VTG,0,0,0,0,0,0";

  GPSSerial.begin(9600);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableGLL);
  delay(COMMAND_DELAY);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableGSA);
  delay(COMMAND_DELAY);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableVTG);
  delay(COMMAND_DELAY);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)baud115200);
  GPSSerial.flush();
  GPSSerial.end();
  delay(COMMAND_DELAY);
  GPSSerial.begin(115200);
}

void GpsClass::poll() {
  // if (GPSSerial.available()) {       // If anything comes in GPSSerial
  //   Serial.write(GPSSerial.read());  // read it and send it out Serial (USB)
  // }
  // return;
  int start = millis();
  gps_fix fix;
  bool noData = true;
  // try at most 1 second
  while (noData && millis() - start < 1000) {
    while (gps.available(GPSSerial)) {
      fix = gps.read();
      if (fix.valid.location && fix.valid.date && fix.valid.time && fix.valid.speed && fix.valid.hdop) {
        latitude = fix.latitudeL();
        longitude = fix.longitudeL();
        hdop = fix.hdop;
        time = fix.dateTime;
        speed = fix.speed_mph();
        NeoGPS::time_t dt = fix.dateTime;
        sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
        // logDebug(dateTime);
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

int GpsClass::getHdop() {
  return hdop;
}
float GpsClass::getSpeed() {
  return speed;
}

uint32_t GpsClass::getTime() {
  return time;
}
const char *GpsClass::getDateTime() {
  return dateTime;
}

GpsClass Gps;