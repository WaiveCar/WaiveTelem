#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();
  bool isConnected();
  int getLatitude();
  int getLongitude();
  int getHdop();
  float getSpeed();
  uint32_t getTime();

 private:
  bool connected = false;
  int latitude = 0;
  int longitude = 0;
  int hdop = 0;
  float speed = 0;
  uint32_t time = 0;
};

extern GpsClass Gps;

#endif  // Gps_h