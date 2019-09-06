#ifndef Internet_h
#define Internet_h

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#define InternetClient WiFiClient
#define InternetSslClient WiFiSSLClient
#else
#include <MKRNB.h>
#define InternetClient NBClient
#define InternetSslClient NBSSLClient
#endif

class InternetClass {
 public:
  bool connect();
  bool isConnected();
  unsigned long getTime();
  int getSignalStrength();
  const char* getModemVersion();
};

extern InternetClass Internet;

#endif  // Internet_h