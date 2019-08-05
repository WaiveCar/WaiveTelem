#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();
  int getLatitude();
  int getLongitude();
  int getHdop();
  float getSpeed();
  uint32_t getTime();
  const char* getDateTime();

 private:
  int latitude = 0;
  int longitude = 0;
  int hdop = 0;
  float speed = 0;
  uint32_t time = 0;
  char dateTime[32] = "0000-00-00T00:00:00Z";
};

extern GpsClass Gps;

#endif  // Gps_h