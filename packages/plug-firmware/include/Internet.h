#ifndef Internet_h
#define Internet_h

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#define InternetClient WiFiClient
#else
#include <MKRNB.h>
#define InternetClient NBClient
#endif

class InternetClass {
 public:
  void connect();
  bool isConnected();
  unsigned long getTime();
  int getSignalStrength();
};

extern InternetClass Internet;

#endif  // Internet_h