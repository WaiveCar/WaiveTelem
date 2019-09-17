#ifndef Motion_h
#define Motion_h

class MotionClass {
 public:
  int begin();
  void poll();
  float getTemp();
  void setSleepEnabled(bool state);

 private:
  int health = 0;
};

extern MotionClass Motion;

#endif  // Motion_h
