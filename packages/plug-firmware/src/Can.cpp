#include <ACAN2515.h>

#include "Can.h"
#include "Config.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#define QUARTZ_FREQUENCY 16 * 1000 * 1000

static ACAN2515 can0(CAN0_CS_PIN, SPI, CAN0_INT_PIN);

#ifdef ARDUINO_SAMD_WAIVE1000
#define NUM_CANBUS 2
static ACAN2515 can1(CAN1_CS_PIN, SPI, CAN1_INT_PIN);
static ACAN2515* canbus[NUM_CANBUS] = {&can0, &can1};
#else
#define NUM_CANBUS 1
static ACAN2515* canbus[NUM_CANBUS] = {&can0};
#endif

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
        int len = status["len"] | 1;

        // Arduino for SAMD21 is little endian, byte 0 is the lowest byte in data64
        uint64_t value = inMessage.data64;
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

// static void onCanReceive0(const CANMessage& inMessage) {
//   onCanReceive(inMessage, 0);
// }

// static void onCanReceive1(const CANMessage& inMessage) {
//   onCanReceive(inMessage, 1);
// }

int CanClass::begin() {
  JsonObject can = Config.get()["can"];
  JsonArray bus = can["bus"];
  busCount = bus.size();
  for (uint32_t i = 0; i < bus.size() && i < NUM_CANBUS; i++) {
    int baud = bus[i]["baud"];
    logInfo("i|canBusNum", i, "i|baud", baud);
    JsonArray status = bus[i]["status"];
    if (status.size() > 0) {
      const int minCanId = status[0]["id"].as<int>();
      logDebug("i|minCanId", minCanId);
      const int maxCanId = status[status.size() - 1]["id"].as<int>();
      logDebug("i|maxCanId", maxCanId);
      const ACAN2515Mask rxm0 = standard2515Mask(0x7ff & (0x7ff << (int)log2(maxCanId)), 0, 0);
      const ACAN2515AcceptanceFilter filters[] = {{standard2515Filter(minCanId, 0, 0), NULL}};
      ACAN2515Settings settings(QUARTZ_FREQUENCY, baud * 1000);
#ifdef ARDUINO_SAMD_WAIVE1000
      auto lambda = (i == 0 ? [] { can0.isr(); } : [] { can1.isr(); });
#else
      auto lambda = [] { can0.isr(); };
#endif
      int errorCode = canbus[i]->begin(settings, lambda, rxm0, filters, 1);  // does soft reset
      if (errorCode) {
        logError("i|error", errorCode, "CANBUS configuration error ");
        return -1;
      }
    }
  }
  sleep();

  return 1;
}

void CanClass::poll() {
  CANMessage message;
  for (int i = 0; i < busCount; i++) {
    while (canbus[i]->available()) {
      if (isSleeping(i)) {
        canbus[i]->changeModeOnTheFly(ACAN2515Settings::NormalMode);
      }
      canbus[i]->receive(message);
      onCanReceive(message, i);
    }
  }
}

int CanClass::send() {
  CANMessage message;
  message.data[0] = 0x12;
  message.data[1] = 0x34;
  message.data[2] = 0x56;
  message.data[3] = 0x78;
  message.data[4] = 0x9a;
  message.data[5] = 0xbc;
  message.data[6] = 0xde;
  message.data[7] = 0xf0;
  char temp[32];
  sprintf(temp, "%lx%lx", message.data32[1], message.data32[0]);
  Serial.println(temp);

  char higher32[32];
  strncpy(higher32, temp, 8);
  uint32_t data32h = strtoul(higher32, NULL, 16);
  uint32_t data32l = strtoul(&temp[8], NULL, 16);
  char temp2[32];
  sprintf(temp2, "0x%lx%lx", data32h, data32l);
  Serial.println(temp2);

  message.id = 0x542;
  message.data64 = 0x000;
  can0.tryToSend(message);
  return 0;
}

void CanClass::sleep() {
  logDebug(NULL);
  for (int i = 0; i < busCount; i++) {
    canbus[i]->changeModeOnTheFly(ACAN2515Settings::SleepMode);
    sleeping[i] = true;
  }
}

bool CanClass::isSleeping(int bus) {
  return sleeping[bus];
}

CanClass Can;
