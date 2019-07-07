#include <ACAN2515.h>

#include "Can.h"
#include "Config.h"
#include "Console.h"
#include "Mqtt.h"
#include "Pins.h"

#define QUARTZ_FREQUENCY 16 * 1000 * 1000

static ACAN2515 can0(CAN0_CS_PIN, SPI, CAN0_INT_PIN);
static ACAN2515 can1(CAN1_CS_PIN, SPI, CAN1_INT_PIN);

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

struct Config_t {
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

static Config_t config;

char* can_labels[] = {"ignition", "mileage", "fuel_level", "charging", "can_speed", "central_lock", "door_front_left",
                      "door_front_right", "door_back_left", "door_back_right", "window_front_left", "window_front_right",
                      "window_back_left", "window_back_right"};

int bit_filter(int data_bytes, int bit_num, int data_len) {
  data_bytes = data_bytes >> bit_num;
  int filt = pow(2, data_len) - 1;
  int data = data_bytes & filt;
  return data;
}

int min_can(int bus_num) {
  int min_can = 0x7FF;
  for (int i = 0; i < num_can_items; i++) {
    if (config.bus_id[i] == bus_num) min_can = min(min_can, config.can_id[0]);
  }
  return min_can;
}
int max_can(int bus_num) {
  int max_can = 0;
  for (int i = 0; i < num_can_items; i++) {
    if (config.bus_id[i] == bus_num) max_can = max(max_can, config.can_id[0]);
  }
  return max_can;
}

static void onReceive1(const CANMessage& inMessage) {
  if (!inMessage.rtr) {
    //Software filter for relevant IDs
    for (int i = 0; i < num_can_items; i++) {
      if (inMessage.id == config.can_id[i]) {
        int data = inMessage.data[config.can_byte_num[i]];
        int data_shifter = config.can_bit_num[i] + config.can_data_len[i];
        int j = 1;
        while (data_shifter > 8) {
          data = data | (inMessage.data[config.can_byte_num[i] + j] << (8 * j));
          data_shifter -= 8;
          j++;
        }
        data = bit_filter(data, config.can_bit_num[i], config.can_data_len[i]);
        // set status based on which data was received
      }
    }
  }
}

void CanClass::setup() {
  Serial.println("Configure ACAN2515");
  Serial.print("CAN0, baud: ");
  Serial.println(config.bus_baud[0]);
  ACAN2515Settings settings(QUARTZ_FREQUENCY, config.bus_baud[0] * 1000);  // CAN bit rate 500 kb/s
  //  settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
  const ACAN2515Mask rxm0 = standard2515Mask((0x7FF - (min_can(0) - max_can(0))), 0, 0);  // For filter #0 and #1
  const ACAN2515AcceptanceFilter filters[] = {
      {standard2515Filter(min_can(0), 0, 0), onReceive1}};
  const uint32_t errorCode = can0.begin(settings, [] { can0.isr(); }, rxm0, filters, 1);
  if (errorCode != 0) {
    Serial.print("Configuration error 0x");
    Serial.println(errorCode, HEX);
  }

  if (config.num_can > 1) {
    Serial.print("CAN1, baud: ");
    Serial.println(config.bus_baud[1]);
    ACAN2515Settings settings(QUARTZ_FREQUENCY, config.bus_baud[1] * 1000);  // CAN bit rate stored value
    //  settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
    const ACAN2515Mask rxm0 = standard2515Mask((0x7FF - (min_can(1) - max_can(1))), 0, 0);  // For filter #0 and #1
    const ACAN2515AcceptanceFilter filters[] = {
        {standard2515Filter(min_can(1), 0, 0), onReceive1}};
    const uint32_t errorCode = can1.begin(settings, [] { can1.isr(); }, rxm0, filters, 1);
    if (errorCode != 0) {
      Serial.print("Configuration error 0x");
      Serial.println(errorCode, HEX);
    }
  }
}

void CanClass::poll() {
}

CanClass Can;