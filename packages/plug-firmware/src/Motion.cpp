#include <JsonLogger.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

#include "Motion.h"

MPU6050 mpu6050(Wire);

int MotionClass::begin() {
  health = 1;
  mpu6050.begin();
  if (mpu6050.readMPU6050(MPU6050_WHO_AM_I) != 0x68) {
    health = -1;
  } else {
    mpu6050.calcGyroOffsets();
  }
  return health;
}

void MotionClass::poll() {
  if (health == 1) {
    mpu6050.update();

    // Serial.println("=======================================================");
    // Serial.print("temp : ");
    // Serial.println(mpu6050.getTemp());
    // Serial.print("accX : ");
    // Serial.print(mpu6050.getAccX());
    // Serial.print("\taccY : ");
    // Serial.print(mpu6050.getAccY());
    // Serial.print("\taccZ : ");
    // Serial.println(mpu6050.getAccZ());

    // Serial.print("gyroX : ");
    // Serial.print(mpu6050.getGyroX());
    // Serial.print("\tgyroY : ");
    // Serial.print(mpu6050.getGyroY());
    // Serial.print("\tgyroZ : ");
    // Serial.println(mpu6050.getGyroZ());

    // Serial.print("accAngleX : ");
    // Serial.print(mpu6050.getAccAngleX());
    // Serial.print("\taccAngleY : ");
    // Serial.println(mpu6050.getAccAngleY());

    // Serial.print("gyroAngleX : ");
    // Serial.print(mpu6050.getGyroAngleX());
    // Serial.print("\tgyroAngleY : ");
    // Serial.print(mpu6050.getGyroAngleY());
    // Serial.print("\tgyroAngleZ : ");
    // Serial.println(mpu6050.getGyroAngleZ());

    // Serial.print("angleX : ");
    // Serial.print(mpu6050.getAngleX());
    // Serial.print("\tangleY : ");
    // Serial.print(mpu6050.getAngleY());
    // Serial.print("\tangleZ : ");
    // Serial.println(mpu6050.getAngleZ());
    // Serial.println("=======================================================\n");
  }
}

MotionClass Motion;
