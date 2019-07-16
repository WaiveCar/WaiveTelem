#ifndef Gps_h
#define Gps_h

class GpsClass {
 public:
  void setup();
  void poll();
  const String& getData();

 private:
  String data;
};

extern GpsClass Gps;

#endif  // Gps_h