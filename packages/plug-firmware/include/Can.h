#ifndef Can_h
#define Can_h

class CanClass {
 public:
  int begin();
  void poll();
  int send();
  void sleep();
  bool isSleeping(int bus);

 private:
  bool sleeping[2];
  uint8_t busCount = 0;
};

extern CanClass Can;

#endif  // Can_h