#ifndef Config_h
#define Config_h

#include <Arduino.h>

enum can_info_type {
  IGNITION,
  MILEAGE,
  FUEL_LEVEL,
  CHARGING,
  CAN_SPEED,
  CENTRAL_LOCK,
  DOOR_FRONT_LEFT,
  DOOR_FRONT_RIGHT,
  DOOR_BACK_LEFT,
  DOOR_BACK_RIGHT,
  WINDOW_FRONT_LEFT,
  WINDOW_FRONT_RIGHT,
  WINDOW_BACK_LEFT,
  WINDOW_BACK_RIGHT,
  NUM_CAN_ITEMS
};

struct CanConfig {
  char make[32];
  char model[32];
  int8_t num_can = 0;
  int16_t bus_baud[2];
  int8_t bus_id[NUM_CAN_ITEMS];
  int16_t can_id[NUM_CAN_ITEMS];
  int8_t can_byte_num[NUM_CAN_ITEMS];
  int8_t can_bit_num[NUM_CAN_ITEMS];
  int8_t can_data_len[NUM_CAN_ITEMS];
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