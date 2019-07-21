#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();
  int getLatitude();
  int getLongitude();
  float getSpeed();
  float getHeading();
  int getTime();
  const char* getDateTime();

 private:
  int latitude = 0;
  int longitude = 0;
  float speed = 0;
  float heading = 0;
  int time = 0;
  char dateTime[32] = "0000-00-00T00:00:00Z";
};

extern GpsClass Gps;

#endif  // Gps_h