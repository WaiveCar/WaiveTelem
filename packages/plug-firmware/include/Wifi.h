#ifndef Wifi_h
#define Wifi_h

class WifiClass {
 public:
  void connect();
  bool isConnected();
};

extern WifiClass Wifi;

#endif  // Wifi_h