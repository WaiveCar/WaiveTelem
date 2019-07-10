#ifndef Config_h
#define Config_h

#include <Arduino.h>

#ifndef USE_ARDUINO_JSON
#include "SimpleJsonListener.h"
#endif

struct CanTelemetry {
  String name;
  uint32_t can_id;
  uint8_t can_byte_num;
  uint8_t can_bit_num;
  uint8_t can_data_len;
  uint8_t bus_id;
};

struct CanConfig {
  String make;
  String model;
  uint8_t num_bus;
  uint8_t num_telemetry;
  uint16_t bus_baud[2];
  CanTelemetry telemetry[20];
};

#ifdef USE_ARDUINO_JSON
class ConfigClass {
#else
class ConfigClass : public SimpleJsonListener {
#endif
 public:
  void load();
  String& getId();
  String& getMqttBrokerUrl();
  String& getMqttBrokerCert();
  int getGpsInterval();
  CanConfig& getCanConfig();
  void value(String v);

 protected:
  String id;
  String mqttBrokerUrl;
  String mqttBrokerCert;
  bool gpsTelemetry;
  int gpsInRideInterval;
  int gpsNotInRideInterval;
  CanConfig canConfig;
};

extern ConfigClass Config;

#endif  // Config_h