#ifndef Config_h
#define Config_h

#include <Arduino.h>

enum can_info_type {
  ignition,
  mileage,
  fuel_level,
  charging,
  can_speed,
  central_lock,
  door_front_left,
  door_front_right,
  door_back_left,
  door_back_right,
  window_front_left,
  window_front_right,
  window_back_left,
  window_back_right,
  num_can_items
};

struct CanConfig {
  char make[32];
  char model[32];
  int num_can = 0;
  int bus_baud[3];
  int can_id[num_can_items];
  int can_byte_num[num_can_items];
  int can_bit_num[num_can_items];
  int can_data_len[num_can_items];
  int bus_id[num_can_items];
};

class ConfigClass {
 public:
  void load();
  String& getId();
  String& getMqttBrokerUrl();
  String& getMqttBrokerCert();
  int getGpsInterval();
  CanConfig& getCanConfig();

 private:
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