#ifndef Cellular_h
#define Cellular_h

#include <MKRNB.h>

class CellularClass {
 public:
  void connect();
  bool isConnected();
  unsigned long getTime();

 protected:
  NB nbAccess;
  GPRS gprs;
};

extern CellularClass Cellular;

#endif  // Cellular_h