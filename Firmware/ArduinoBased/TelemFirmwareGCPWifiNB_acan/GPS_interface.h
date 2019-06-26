
#include <TinyGPS++.h>


#define GPSSerial Serial1

TinyGPSPlus gps;



static void gpsSmartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPSSerial.available())
      gps.encode(GPSSerial.read());
  } while (millis() - start < ms);
}
