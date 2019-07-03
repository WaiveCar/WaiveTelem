#ifndef Config_h
#define Config_h

#include <String.h>

class ConfigClass {
 public:
  void load();
  String getId();
  String getMqttBrokerUrl();
  String getMqttBrokerCert();

 private:
  String id;
  String mqttBrokerUrl;
  String mqttBrokerCert;
};

extern ConfigClass Config;

#endif  // Config_h