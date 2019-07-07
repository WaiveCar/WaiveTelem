#ifndef Can_h
#define Can_h

class CanClass {
 public:
  void setup();
  void poll();
};

extern CanClass Can;

#endif  // Can_h