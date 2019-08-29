#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
#ifndef ARDUINO_SAMD_MKR1000
#include <Modem.h>
#endif
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <json_builder.h>

#include "Bluetooth.h"
#include "Can.h"
#include "Command.h"
#include "Config.h"
#include "Gps.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(4000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  int eccInit = ECCX08.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  int sdInit = SD.begin(SD_CS_PIN);
  int cfgInit = Config.begin();
  int loggerInit = Logger.begin();
  int cmdInit = Command.begin();
  System.begin();
  int mqttInit = Mqtt.begin();
  Mqtt.poll();
  Gps.begin();

#ifndef ARDUINO_SAMD_MKR1000
  String modemResponse = "";
  MODEM.send("ATI9");
  MODEM.waitForResponse(100, &modemResponse);
#endif

  char initStatus[128], sysJson[256];
  json(initStatus, "-{",
#ifndef ARDUINO_SAMD_MKR1000
       "modem", modemResponse.c_str(),
#endif
       "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit,
       "i|cmd", cmdInit,
       "i|mqtt", mqttInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), initStatus);
  System.sendInfo(sysJson);
}

long timer = 0;

void loop() {
  Watchdog.reset();
  if (!System.stayResponsive()) {
    System.sleep(1);
  }
  System.keepTime();

  Mqtt.poll();
  Bluetooth.poll();
  Can.poll();
  System.poll();

  mpu6050.update();

  // if (millis() - timer > 1000) {
  //   Serial.println("=======================================================");
  //   Serial.print("temp : ");
  //   Serial.println(mpu6050.getTemp());
  //   Serial.print("accX : ");
  //   Serial.print(mpu6050.getAccX());
  //   Serial.print("\taccY : ");
  //   Serial.print(mpu6050.getAccY());
  //   Serial.print("\taccZ : ");
  //   Serial.println(mpu6050.getAccZ());

  //   Serial.print("gyroX : ");
  //   Serial.print(mpu6050.getGyroX());
  //   Serial.print("\tgyroY : ");
  //   Serial.print(mpu6050.getGyroY());
  //   Serial.print("\tgyroZ : ");
  //   Serial.println(mpu6050.getGyroZ());

  //   Serial.print("accAngleX : ");
  //   Serial.print(mpu6050.getAccAngleX());
  //   Serial.print("\taccAngleY : ");
  //   Serial.println(mpu6050.getAccAngleY());

  //   Serial.print("gyroAngleX : ");
  //   Serial.print(mpu6050.getGyroAngleX());
  //   Serial.print("\tgyroAngleY : ");
  //   Serial.print(mpu6050.getGyroAngleY());
  //   Serial.print("\tgyroAngleZ : ");
  //   Serial.println(mpu6050.getGyroAngleZ());

  //   Serial.print("angleX : ");
  //   Serial.print(mpu6050.getAngleX());
  //   Serial.print("\tangleY : ");
  //   Serial.print(mpu6050.getAngleY());
  //   Serial.print("\tangleZ : ");
  //   Serial.println(mpu6050.getAngleZ());
  //   Serial.println("=======================================================\n");
  //   timer = millis();
  // }
}
