#include <Arduino.h>

#include "Console.h"
#include "Mqtt.h"
#include "Status.h"

extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void StatusClass::setup() {
  inRide = false;
}

void StatusClass::sendVersion() {
  String version = "{\"inRide\": \"false\", \"firmware\": \"" + String(VERSION) + "\"}";
  Mqtt.telemeter(version, false);
}

void StatusClass::setInRide(bool in) {
  inRide = in;
}

bool StatusClass::getInRide() {
  return inRide;
}

String& StatusClass::getStatus() {
  status = "\"freeMemory\": " + String(freeMemory());
  return status;
}

StatusClass Status;