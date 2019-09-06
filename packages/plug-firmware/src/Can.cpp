#include <ACAN2515.h>
#include <JsonLogger.h>

#include "Can.h"
#include "Config.h"
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
    logTrace("i|busNum", busNum, "i|inMessage.id", inMessage.id, "i|valueHi", inMessage.data32[1], "i|valueLow", inMessage.data32[0]);
    // Software filter for relevant IDs
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

int CanClass::begin() {
  JsonObject can = Config.get()["can"];
  JsonArray bus = can["bus"];
  busCount = bus.size();
  health = 1;
  for (uint32_t i = 0; i < busCount && i < NUM_CANBUS; i++) {
    int baud = bus[i]["baud"];
    logInfo("i|canBusNum", i, "i|baud", baud);
    JsonArray status = bus[i]["status"];
    ACAN2515Settings settings(QUARTZ_FREQUENCY, baud * 1000);
    settings.mReceiveBufferSize = 16;
    settings.mTransmitBuffer0Size = 2;

#ifdef ARDUINO_SAMD_WAIVE1000
    auto lambda = (i == 0 ? [] { can0.isr(); } : [] { can1.isr(); });
#else
    auto lambda = [] { can0.isr(); };
#endif

    int errorCode = 0;
    if (status.size() > 0) {
      const int minCanId = status[0]["id"].as<int>();
      const int maxCanId = status[status.size() - 1]["id"].as<int>();
      uint16_t mask = 0x7ff & ~(minCanId ^ maxCanId);
      bool clearRestBits = false;
      for (uint16_t bit = 0x400; bit != 0; bit >>= 1) {
        if (clearRestBits) {
          mask = mask & ~(bit);
        } else {
          if (!(mask & bit)) {
            clearRestBits = true;
          }
        }
      }
      logDebug("i|minCanId", minCanId, "i|maxCanId", maxCanId, "i|mask", mask);
      const ACAN2515Mask rxm0 = standard2515Mask(mask, 0, 0);
      const ACAN2515AcceptanceFilter filters[] = {{standard2515Filter(minCanId, 0, 0), NULL}};
      errorCode = canbus[i]->begin(settings, lambda, rxm0, filters, 1);
    } else {
      errorCode = canbus[i]->begin(settings, lambda);
    }
    if (errorCode) {
      logError("i|error", errorCode, "can configuration error ");
      health = -1;
      return health;
    }
  }
  // sleep();

  return health;
}

void CanClass::poll() {
  if (health <= 0) {
    begin();
  } else {
    CANMessage message;
    for (int i = 0; i < busCount; i++) {
      int totalMsg = 0;
      while (canbus[i]->available()) {
        if (isSleeping(i)) {
          canbus[i]->changeModeOnTheFly(ACAN2515Settings::NormalMode);
        }
        canbus[i]->receive(message);
        onCanReceive(message, i);
        totalMsg++;
      }
      logTrace("i|totalMsg", totalMsg);
    }
  }
}

void CanClass::send(JsonObject& cmdJson) {
  CANMessage message;
  int bus = cmdJson["bus"] || 0;

  if (bus >= busCount) {
    logError("cmdJson[bus] >= busCount");
    return;
  }
  const char* msg = cmdJson["msg"];

  message.id = cmdJson["id"];
  char higher32[9];
  strncpy(higher32, msg, 8);
  message.data32[1] = strtoul(higher32, NULL, 16);
  message.data32[0] = strtoul(&msg[8], NULL, 16);
  message.len = 8;
  logDebug("i|bus", bus, "i|id", message.id, "msg", msg);
  // char str[32];
  // sprintf(str, "%08lx%08lx", message.data32[1], message.data32[0]);
  // logDebug("str", str);

  bool ok = canbus[bus]->tryToSend(message);
  if (!ok) {
    logError("can tryToSend() failed");
    health = -2;
  }
}

void CanClass::sendCommand(const char* cmd) {
  if (health == 1) {
    logDebug("cmd", cmd);
    JsonObject cmdJson = Config.get()["can"]["cmd"][cmd];
    // char output[128];
    // serializeJson(cmdJson, output);
    // logDebug("cmdJson", output);
    send(cmdJson);
  } else {
    logWarn("can not initialized yet");
  }
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

int CanClass::getHealth() {
  return health;
}

CanClass Can;
