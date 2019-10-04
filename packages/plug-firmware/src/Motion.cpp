#include <JsonLogger.h>
#include <MPU6050.h>

#include "Motion.h"
#include "System.h"

MPU6050 mpu;

int MotionClass::begin() {
  health = 1;
  if (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G)) {
    health = -1;
    return health;
  }

  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);
  // Motion detection acceleration threshold. 1LSB = 1mg?
  mpu.setMotionDetectionThreshold(120);  //TBD
  // Motion detection event duration in ms
  mpu.setMotionDetectionDuration(50);  //TBD, higher value, less detection

  mpu.setSleepEnabled(true);

  return health;
}

void MotionClass::poll() {
  if (health == 1) {
    mpu.readScaledAccel();
    Activites act = mpu.readActivites();

    // Serial.print(accel.XAxis);
    // Serial.print(" ");
    // Serial.print(accel.YAxis);
    // Serial.print(" ");
    // Serial.print(accel.ZAxis);
    // Serial.print(" ");

    // Serial.print(act.isActivity);
    // Serial.print(act.isInactivity);

    // Serial.print("\n");

    if (act.isActivity) {
      char info[64];
      json(info, "i|collisionDetected", act.isActivity);
      System.report(info);
    }
  }
}

float MotionClass::getTemp() {
  return health == 1 ? mpu.readTemperature() * 9 / 5 + 32 : -1;
}

void MotionClass::setSleepEnabled(bool state) {
  if (health == 1) {
    mpu.setSleepEnabled(state);
  }
}

MotionClass Motion;
