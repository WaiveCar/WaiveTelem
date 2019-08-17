#ifndef Can_h
#define Can_h

class CanClass {
 public:
  void begin();
  void poll();
  void sleep();
  void wakeup();

 private:
  uint8_t numberOfCanBuses = 0;
};

extern CanClass Can;

#endif  // Can_h