#ifndef Pins_h
#define Pins_h

#ifdef ARDUINO_SAMD_MKR1000

// SPI reserved: 8 MOSI, 9 SCK, 10 MISO
#define RELAY_2_PIN 2
#define CAN0_CS_PIN 3
#define CAN0_INT_PIN 7
#define SD_CS_PIN 4
#define FL_CS_PIN 5
#define DOOR_LOCK_PIN A4
#define DOOR_UNLOCK_PIN A5
#define BLE_CS_PIN A1
#define BLE_INT_PIN A2
#define BLE_RST_PIN A3
//Open Digitals  11, 12 - i2c reserved for crypto
#define BLE_MOSI_PIN 0
#define BLE_SCK_PIN 1
#define BLE_MISO_PIN 6
#define CAN1_CS_PIN A0
#define CAN1_INT_PIN A6

#else

// SPI reserved: 8 MOSI, 9 SCK, 10 MISO
#define RELAY_2_PIN 2
#define CAN0_CS_PIN 3
#define CAN0_INT_PIN 7
#define SD_CS_PIN 4
#define FL_CS_PIN 5
#define DOOR_LOCK_PIN A4
#define DOOR_UNLOCK_PIN A5
#define BLE_CS_PIN A1
#define BLE_INT_PIN A2
#define BLE_RST_PIN A3
//Open Digitals  11, 12 - i2c reserved for crypto
#define BLE_MOSI_PIN 0
#define BLE_SCK_PIN 1
#define BLE_MISO_PIN 6
#define CAN1_CS_PIN A0
#define CAN1_INT_PIN A6

#endif

#include <Arduino.h>

class PinsClass {
 public:
  void setup();
  void unlockDoors();
  void lockDoors();
  void immobilize();
  void unimmobilize();
};

extern PinsClass Pins;

#endif  // Pins_h