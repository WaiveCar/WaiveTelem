#include <Arduino.h>
#include <JsonLogger.h>
#include <NMEAGPS.h>

#include "Gps.h"
#include "Internet.h"
#include "System.h"

#define GPSSerial Serial1

#define COMMAND_DELAY 200

#ifdef ARDUINO_SAMD_WAIVE1000

#include <ublox/ubxGPS.h>
#define UBX_MSG_LEN(msg) (sizeof(msg) - sizeof(ublox::msg_t))
static ubloxGPS gps(&GPSSerial);
const char baud115200[] PROGMEM = "PUBX,41,1,3,3,115200,0";
// we only want RMC, GGA and GSV
const char disableGLL[] PROGMEM = "PUBX,40,GLL,0,0,0,0,0,0";
const char disableGSA[] PROGMEM = "PUBX,40,GSA,0,0,0,0,0,0";
const char disableVTG[] PROGMEM = "PUBX,40,VTG,0,0,0,0,0,0";

#else

#define PMTK_SET_BAUD_57600 "$PMTK251,57600*2C"                                             ///<  57600 bps
#define PMTK_SET_NMEA_OUTPUT_RMCGGAGSV "$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29"  ///< turn on GPRMC, GPGGA and GPGSV
#define PMTK_STANDBY "$PMTK161,0*28"
static NMEAGPS gps;

#endif

int GpsClass::begin() {
  GPSSerial.begin(9600);
#ifdef ARDUINO_SAMD_WAIVE1000
  // reset();
  // delay(1000);
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
  return 1;
}

bool GpsClass::poll() {
  // while (GPSSerial.available()) {    // If anything comes in GPSSerial
  //   Serial.write(GPSSerial.read());  // read it and send it out Serial (USB)
  // }
  // return false;
  uint32_t start = millis();
  gps_fix fix;
  bool hasData = false;
  while (!hasData && millis() - start < 2000) {
    while (gps.available(GPSSerial)) {
      fix = gps.read();
      if (fix.valid.location && fix.valid.date && fix.valid.time && fix.valid.speed && fix.valid.hdop) {
        latitude = fix.latitudeL();
        longitude = fix.longitudeL();
        hdop = fix.hdop;
        speed = fix.speed_mkn();
        if (fix.valid.heading) {
          heading = fix.heading_cd();
        }
        if (!Internet.isConnected()) {
          System.setTimes(fix.dateTime);
        }
        hasData = true;
        break;
      }
    }
  }
  if (!hasData) {
    wakeup();
  }
  return hasData;
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

int GpsClass::getSpeed() {
  return speed;
}

int GpsClass::getHeading() {
  return heading;
}

void GpsClass::sleep() {
  logTrace(NULL);
#ifdef ARDUINO_SAMD_WAIVE1000
  const unsigned char ubxPMREQ[] PROGMEM = {0x02, 0x41, 0x08, 0x00, 0, 0, 0, 0, 0x02};
  const ublox::msg_t *cfg_ptr = (const ublox::msg_t *)ubxPMREQ;
  gps.send_request_P(*cfg_ptr);
#else
  GPSSerial.println(PMTK_STANDBY);
#endif
}

void GpsClass::wakeup() {
  logTrace(NULL);
#ifdef ARDUINO_SAMD_WAIVE1000
  GPSSerial.begin(9600);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableGLL);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableGSA);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)disableVTG);
  gps.send_P(&GPSSerial, (const __FlashStringHelper *)baud115200);
  GPSSerial.flush();
  GPSSerial.end();
  GPSSerial.begin(115200);
#else
  GPSSerial.println("");
#endif
}

// crashes the system for some reason
void GpsClass::reset() {
#ifdef ARDUINO_SAMD_WAIVE1000
  static const uint8_t ubxReset[] __PROGMEM =
      {
          ublox::UBX_CFG, ublox::UBX_CFG_RST,
          UBX_MSG_LEN(ublox::cfg_reset_t), 0,  // word length MSB is 0
          0, 0,                                // clear bbr section
          ublox::cfg_reset_t::HW_RESET,        // reset mode
          0x00                                 // reserved
      };
  const ublox::cfg_reset_t *cfg_cold_ptr = (const ublox::cfg_reset_t *)ubxReset;
  gps.send_request_P(*cfg_cold_ptr);
#endif
}

GpsClass Gps;