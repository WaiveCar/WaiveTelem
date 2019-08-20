#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>

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

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  Pins.begin();
  int eccStatus = ECCX08.begin();
  int sdStatus = SD.begin(SD_CS_PIN);
  int configStatus = Config.begin();
  int loggerStatus = Logger.begin();
  int cmdStatus = Command.begin();
  System.begin();
  int mqttStatus = Mqtt.begin();
  Mqtt.poll();
  int bleStatus = Bluetooth.begin();
  Gps.begin();
  int canStatus = Can.begin();
  System.sendInfo();
}

void loop() {
  Watchdog.reset();
  if (!System.stayAwake()) {
    System.sleep(1);
  }
  System.keepTime();
  Mqtt.poll();
  System.poll();
  Bluetooth.poll();
  Can.poll();
}
