/*
   This file houses all can related functions.




*/

#include <ACAN2515.h>

ACAN2515 can0 (CAN0_CS_PIN, SPI, CAN0_INT_PIN) ;
ACAN2515 can1 (CAN1_CS_PIN, SPI, CAN1_INT_PIN);
static const uint32_t QUARTZ_FREQUENCY = 16 * 1000 * 1000 ; // 16 MHz


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

char *can_labels[] = {"ignition", "mileage", "fuel_level", "charging", "can_speed", "central_lock", "door_front_left",
                      "door_front_right", "door_back_left", "door_back_right", "window_front_left", "window_front_right",
                      "window_back_left", "window_back_right"
                     };

int bit_filter(int data_bytes, int bit_num, int data_len) {
  data_bytes = data_bytes >> bit_num;
  int filt = pow(2, data_len)-1;
  int data = data_bytes & filt;
  return data;
}
