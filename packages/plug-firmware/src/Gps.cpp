#include <Arduino.h>
#include <NMEAGPS.h>
#include <ublox/ubxGPS.h>

#include "Gps.h"
#include "Logger.h"
#include "System.h"

#define GPSSerial Serial1

#define COMMAND_DELAY 250

static ubloxGPS gps(&GPSSerial);

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

  poll();
}

void GpsClass::poll() {
  // if (GPSSerial.available()) {       // If anything comes in GPSSerial
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