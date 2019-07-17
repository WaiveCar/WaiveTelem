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
  int latitude = 0;
  int longitude = 0;
  char time[32] = "0000-00-00T00:00:00Z";
};

extern GpsClass Gps;

#endif  // Gps_h