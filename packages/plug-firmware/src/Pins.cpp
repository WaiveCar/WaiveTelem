#include <Arduino.h>
#include <SPI.h>
#include <wiring_private.h>

#include "Pins.h"

void PinsClass::setup() {
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(CAN0_CS_PIN, OUTPUT);
  pinMode(CAN0_INT_PIN, INPUT);
  pinMode(CAN1_CS_PIN, OUTPUT);
  pinMode(CAN1_INT_PIN, INPUT);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(FL_CS_PIN, OUTPUT);
  pinMode(DOOR_UNLOCK_PIN, OUTPUT);
  pinMode(DOOR_LOCK_PIN, OUTPUT);
  pinMode(BLE_CS_PIN, OUTPUT);
  pinMode(BLE_INT_PIN, INPUT);
  pinMode(BLE_RST_PIN, OUTPUT);
  pinPeripheral(BLE_MOSI_PIN, PIO_SERCOM);      //mosi
  pinPeripheral(BLE_SCK_PIN, PIO_SERCOM);       //sck
  pinPeripheral(BLE_MISO_PIN, PIO_SERCOM_ALT);  //miso

  //Ensure SPI CS Pins are all high to avoid a noisy bus at inits
  digitalWrite(CAN0_CS_PIN, HIGH);
  //  digitalWrite(CAN1_CS_PIN, HIGH);
  digitalWrite(SD_CS_PIN, HIGH);
  digitalWrite(FL_CS_PIN, HIGH);
  digitalWrite(BLE_CS_PIN, HIGH);
  digitalWrite(BLE_RST_PIN, HIGH);

  SPI.begin();
}

PinsClass Pins;
