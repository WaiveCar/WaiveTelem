#include <Arduino.h>
#include <NMEAGPS.h>

#include "Gps.h"
#include "Logger.h"
#include "System.h"

#define GPSSerial Serial1

#define COMMAND_DELAY 250

#ifdef ARDUINO_SAMD_WAIVE1000
#include <ublox/ubxGPS.h>
static ubloxGPS gps(&GPSSerial);
#else
#define PMTK_SET_BAUD_57600 "$PMTK251,57600*2C"                                             ///<  57600 bps
#define PMTK_SET_NMEA_OUTPUT_RMCGGAGSV "$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29"  ///< turn on GPRMC, GPGGA and GPGSV
static NMEAGPS gps;
#endif

void GpsClass::setup() {
  GPSSerial.begin(9600);
#ifdef ARDUINO_SAMD_WAIVE1000
  const char baud115200[] PROGMEM = "PUBX,41,1,3,3,115200,0";
  // we only want RMC, GGA and GSV
  const char disableGLL[] PROGMEM = "PUBX,40,GLL,0,0,0,0,0,0";
  const char disableGSA[] PROGMEM = "PUBX,40,GSA,0,0,0,0,0,0";
  const char disableVTG[] PROGMEM = "PUBX,40,VTG,0,0,0,0,0,0";

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
#else
  GPSSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGAGSV);
  GPSSerial.println(PMTK_SET_BAUD_57600);
  delay(COMMAND_DELAY);
  GPSSerial.end();
  delay(COMMAND_DELAY);
  GPSSerial.begin(57600);
#endif

  poll();
}

void GpsClass::poll() {
  // while (GPSSerial.available()) {    // If anything comes in GPSSerial
  //   Serial.write(GPSSerial.read());  // read it and send it out Serial (USB)
  // }
  // return;
  int start = millis();
  gps_fix fix;
  connected = false;
  // try at most 1 second
  while (!connected && millis() - start < 1000) {
    while (gps.available(GPSSerial)) {
      fix = gps.read();
      if (fix.valid.location && fix.valid.date && fix.valid.time && fix.valid.speed && fix.valid.hdop) {
        latitude = fix.latitudeL();
        longitude = fix.longitudeL();
        hdop = fix.hdop;
        speed = fix.speed_mph();
        time = fix.dateTime;
        System.setTime(time);
        connected = true;
        break;
      }
    }
  }
}

bool GpsClass::isConnected() {
  return connected;
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

GpsClass Gps;