#ifndef Config_h
#define Config_h

#include <Arduino.h>

struct CanTelemetry {
  const char* name;
  uint32_t can_id;
  uint8_t can_byte_num;
  uint8_t can_bit_num;
  uint8_t can_data_len;
  uint8_t bus_id;
};

struct CanConfig {
  char* make;
  char* model;
  uint8_t num_bus;
  uint8_t num_telemetry;
  uint16_t bus_baud[2];
  CanTelemetry telemetry[20];
};

class ConfigClass {
 public:
  void load();
  char* getId();
  char* getMqttBrokerUrl();
  char* getMqttBrokerCert();
  char* getNbSimPin();
  int getGpsInterval();
  CanConfig& getCanConfig();

 private:
  char* id;
  char* mqttBrokerUrl;
  char* mqttBrokerCert;
  char* nbSimPin;
  int gpsInRideInterval;
  int gpsNotInRideInterval;
  CanConfig canConfig;
};

extern ConfigClass Config;

#endif  // Config_h