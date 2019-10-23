#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  int begin();
  bool poll();
  int getLatitude();
  int getLongitude();
  int getHdop();
  int getSpeed();
  int getHeading();
  void sleep();
  void wakeup();
  void reset();

 private:
  int latitude = 0;
  int longitude = 0;
  int hdop = 0;
  uint32_t speed = 0;
  uint16_t heading = 0;
  uint32_t lastDataTime = 0;
};

extern GpsClass Gps;

#endif  // Gps_h