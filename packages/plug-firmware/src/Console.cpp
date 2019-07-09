#include <Arduino.h>

#include "Console.h"

extern "C" char* sbrk(int incr);

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void ConsoleClass::setup() {
  Serial.begin(9600);
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
}

void ConsoleClass::logFreeMemory() {
  log("Free Memory: " + String(freeMemory()));
}

ConsoleClass Console;