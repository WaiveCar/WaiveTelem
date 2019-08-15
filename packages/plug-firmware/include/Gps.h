#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  bool poll();
  int getLatitude();
  int getLongitude();
  int getHdop();
  float getSpeed();
  float getHeading();
  void sleep();
  void wakeup();
  void reset();

 private:
  int latitude = 0;
  int longitude = 0;
  int hdop = 0;
  float speed = 0.0;
  float heading = 0.0;
};

extern GpsClass Gps;

#endif  // Gps_h