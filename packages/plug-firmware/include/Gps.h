#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();
  float getLatitude();
  float getLongitude();
  const char* getTime();

 private:
  int latitude;
  int longitude;
  char time[32];
};

extern GpsClass Gps;

#endif  // Gps_h