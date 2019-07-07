#ifndef Config_h
#define Config_h

#include <Arduino.h>

class ConfigClass {
 public:
  void load();
  String getId();
  String getMqttBrokerUrl();
  String getMqttBrokerCert();
  int getGpsInterval();

 private:
  String id;
  String mqttBrokerUrl;
  String mqttBrokerCert;
  bool gpsTelemetry;
  int gpsInRideInterval;
  int gpsNotInRideInterval;
};

extern ConfigClass Config;

#endif  // Config_h