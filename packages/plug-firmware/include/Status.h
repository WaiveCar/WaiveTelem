#ifndef Status_h
#define Status_h

class StatusClass {
 public:
  void setup();
  void sendVersion();
  void setInRide(bool in);
  bool getInRide();
  String& getStatus();

 protected:
  String status;
  bool inRide;
};

extern StatusClass Status;

#endif  // Status_h