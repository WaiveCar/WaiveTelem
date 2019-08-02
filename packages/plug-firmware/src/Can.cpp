#include <ACAN2515.h>

#include "Can.h"
#include "Config.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#define QUARTZ_FREQUENCY 16 * 1000 * 1000

static ACAN2515 can0(CAN0_CS_PIN, SPI, CAN0_INT_PIN);
static ACAN2515 can1(CAN1_CS_PIN, SPI, CAN1_INT_PIN);

static void onCanReceive(const CANMessage& inMessage, int busNum) {
  if (!inMessage.rtr) {
    JsonObject can = Config.get()["can"];
    JsonObject bus = can["bus"][busNum];
    JsonArray statusArray = bus["status"];
    //Software filter for relevant IDs
    for (uint8_t i = 0; i < statusArray.size(); i++) {
      JsonObject status = statusArray[i];
      if (inMessage.id == status["id"]) {
        int bit = status["bit"];
        int len = status["len"];
        uint64_t value =
            static_cast<uint64_t>(inMessage.data[0]) |
            static_cast<uint64_t>(inMessage.data[1]) << 8 |
            static_cast<uint64_t>(inMessage.data[2]) << 16 |
            static_cast<uint64_t>(inMessage.data[3]) << 24 |
            static_cast<uint64_t>(inMessage.data[4]) << 32 |
            static_cast<uint64_t>(inMessage.data[5]) << 40 |
            static_cast<uint64_t>(inMessage.data[6]) << 48 |
            static_cast<uint64_t>(inMessage.data[7]) << 56;
        uint64_t mask = pow(2, len) - 1;
        value = (value >> bit) & mask;
        String name = status["name"];
        uint32_t delta = status["delta"];
        System.setCanStatus(name, value, delta);
      }
    }
    System.sendCanStatus();
  }
}

static void onCanReceive0(const CANMessage& inMessage) {
  onCanReceive(inMessage, 0);
}

static void onCanReceive1(const CANMessage& inMessage) {
  onCanReceive(inMessage, 1);
}

void CanClass::setup() {
  JsonObject can = Config.get()["can"];
  JsonArray bus = can["bus"];
  logDebug(F("Configure ACAN2515"));
  for (uint32_t i = 0; i < bus.size(); i++) {
    int baud = bus[i]["baud"];
    logDebug("CAN BUS #" + String(i) + ", baud: " + baud);
    ACAN2515Settings settings(QUARTZ_FREQUENCY, baud * 1000);
    JsonArray status = bus[i]["status"];
    if (status.size() > 0) {
      const int minCanId = status[0]["id"].as<int>();
      logDebug("minCanId: " + String(minCanId));
      const int maxCanId = status[status.size() - 1]["id"].as<int>();
      logDebug("maxCanId: " + String(maxCanId));
      const ACAN2515Mask rxm0 = standard2515Mask(0x7ff & (0x7ff << (int)log2(maxCanId)), 0, 0);
      const ACAN2515AcceptanceFilter filters[] = {
          {standard2515Filter(minCanId, 0, 0), NULL}};
      auto& can = (i == 0 ? can0 : can1);
      auto lambda = (i == 0 ? [] { can0.isr(); } : [] { can1.isr(); });
      const uint32_t errorCode = can.begin(settings, lambda, rxm0, filters, 1);
      if (errorCode != 0) {
        logError("Configuration error " + String(errorCode));
      }
    }
  }
}

void CanClass::poll() {
  CANMessage frame;
  while (can0.available()) {
    can0.receive(frame);
    onCanReceive0(frame);
  }
  while (can1.available()) {
    can1.receive(frame);
    onCanReceive1(frame);
  }
}

CanClass Can;
