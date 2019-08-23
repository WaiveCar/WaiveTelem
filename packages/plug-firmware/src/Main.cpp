#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoECCX08.h>
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

void setup() {
  Serial.begin(115200);
#ifdef DEBUG
  delay(5000);  // to see beginning of the login
#endif
  Watchdog.enable(WATCHDOG_TIMEOUT);

  // int pinsInit = Pins.begin();
  Pins.begin();
  int eccInit = ECCX08.begin();
  int sdInit = SD.begin(SD_CS_PIN);
  int cfgInit = Config.begin();
  int loggerInit = Logger.begin();
  int cmdInit = Command.begin();
  // int sysInit = System.begin();
  System.begin();
  int mqttInit = Mqtt.begin();
  Mqtt.poll();
  int bleInit = Bluetooth.begin();
  int gpsInit = Gps.begin();
  int canInit = Can.begin();

  char initJson[128], sysJson[256];
  json(initJson, "i|ecc", eccInit,
       "i|sd", sdInit,
       "i|cfg", cfgInit,
       "i|logger", loggerInit,
       "i|cmd", cmdInit,
       //  "i|sys", sysInit,
       "i|mqtt", mqttInit,
       "i|ble", bleInit,
       "i|gps", gpsInit,
       "i|can", canInit);
  json(sysJson, "firmware", FIRMWARE_VERSION, "i|configFreeMem", Config.getConfigFreeMem(), "o|init", initJson);
  System.sendInfo(sysJson);
}

float readings[5];
int readingIndex = 0;
bool averageValid = false;

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

#ifdef ARDUINO_SAMD_WAIVE1000
  {
    float vin = (float)analogRead(VIN_SENSE) / 4096 * 3.3 * 50.4 / 10.2;
    char reading[32];
    sprintf(reading, "%.3f", vin);
    Serial.println(String("VIN: ") + reading);

    readings[readingIndex] = vin;
    readingIndex++;
    if (readingIndex == 5) {
      averageValid = true;
      readingIndex = 0;
    }
    if (averageValid) {
      float total = 0;
      for (int i = 0; i < 5; i++) {
        total += readings[i];
      }
      char reading[32];
      sprintf(reading, "%.3f", total / 5);
      Serial.println(String("Average of last 5 VINs: ") + reading);
    }
  }
#endif
}
