#ifndef Motion_h
#define Motion_h

class MotionClass {
 public:
  int begin();
  void poll();
  // to be implemented, awake only when inRide?
  // void sleep();
  // void wakeup();

 private:
  int health = 0;
};

extern MotionClass Motion;

#endif  // Motion_h
