#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();

 private:
  u_int32_t lastSentTime;
};

extern GpsClass Gps;

#endif  // Gps_h