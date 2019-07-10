#include <Arduino.h>

#include "Console.h"

void ConsoleClass::setup() {
  Serial.begin(9600);
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
}

ConsoleClass Console;