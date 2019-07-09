#include <ACAN2515.h>

#include "Can.h"
#include "Config.h"
#include "Console.h"
#include "Mqtt.h"
#include "Pins.h"

#define QUARTZ_FREQUENCY 16 * 1000 * 1000

static ACAN2515 can0(CAN0_CS_PIN, SPI, CAN0_INT_PIN);
static ACAN2515 can1(CAN1_CS_PIN, SPI, CAN1_INT_PIN);

int bit_filter(int data_bytes, int bit_num, int data_len) {
  data_bytes = data_bytes >> bit_num;
  int filt = pow(2, data_len) - 1;
  int data = data_bytes & filt;
  return data;
}

int min_can(int bus_num, CanConfig& config) {
  int min_can = 0x7FF;
  for (int i = 0; i < NUM_CAN_ITEMS; i++) {
    if (config.bus_id[i] == bus_num) min_can = min(min_can, config.can_id[0]);
  }
  return min_can;
}
int max_can(int bus_num, CanConfig& config) {
  int max_can = 0;
  for (int i = 0; i < NUM_CAN_ITEMS; i++) {
    if (config.bus_id[i] == bus_num) max_can = max(max_can, config.can_id[0]);
  }
  return max_can;
}

static void onCanReceive(const CANMessage& inMessage) {
  if (!inMessage.rtr) {
    CanConfig& config = Config.getCanConfig();
    //Software filter for relevant IDs
    for (int i = 0; i < NUM_CAN_ITEMS; i++) {
      if (inMessage.id == (u_int32_t)config.can_id[i]) {
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
        if (i == IGNITION) {
          // set_ignition_status(data);
        } else if (i == MILEAGE) {
          // set_mileage(data, 16.0934);
        } else if (i == FUEL_LEVEL) {
          // set_fuel_charge_level(data);
        } else if (i == CHARGING) {
          // set_charging_status(data);
        } else if (i == CAN_SPEED) {
          // set_speed(data, 100.0);
        } else if (i == CENTRAL_LOCK) {
          // set_central_lock_status(data);
        } else if (i == DOOR_FRONT_LEFT) {
          // door front left
          // set_door_status(data, 1);
        } else if (i == DOOR_FRONT_RIGHT) {
          // door front right
          // set_door_status(data, 2);
        } else if (i == DOOR_BACK_LEFT) {
          // door back left
          // set_door_status(data, 3);
        } else if (i == DOOR_BACK_RIGHT) {
          // door back right
          // set_door_status(data, 4);
        } else if (i == WINDOW_FRONT_LEFT) {
          // window driver
          // set_window_status(data, 1);
        } else if (i == WINDOW_FRONT_RIGHT) {
          // window codriver
          // set_window_status(data, 2);
        } else if (i == WINDOW_BACK_LEFT) {
          // window back left
          // set_window_status(data, 3);
        } else if (i == WINDOW_BACK_RIGHT) {
          // window back right
          // set_window_status(data, 4);
        }
      }
    }
  }
}

void CanClass::setup() {
  CanConfig& config = Config.getCanConfig();
  log(F("Configure ACAN2515"));
  for (int i = 0; i < config.num_can; i++) {
    log("CAN" + String(i) + ", baud: " + config.bus_baud[i]);
    ACAN2515Settings settings(QUARTZ_FREQUENCY, config.bus_baud[i] * 1000);
    //  settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
    const ACAN2515Mask rxm0 = standard2515Mask((0x7FF - (min_can(i, config) - max_can(i, config))), 0, 0);
    const ACAN2515AcceptanceFilter filters[] = {
        {standard2515Filter(min_can(i, config), 0, 0), onCanReceive}};
    auto& can = (i == 0 ? can0 : can1);
    auto lambda = (i == 0 ? [] { can0.isr(); } : [] { can1.isr(); });
    const uint32_t errorCode = can.begin(settings, lambda, rxm0, filters, 1);
    if (errorCode != 0) {
      Serial.print("Configuration error 0x");
      Serial.println(errorCode, HEX);
    }
  }
}

CanClass Can;
