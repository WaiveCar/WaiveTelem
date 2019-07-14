#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoBearSSL.h>

#include "Config.h"
#include "Console.h"
#include "Http.h"
#include "Internet.h"
#include "System.h"

static InternetClient client;

void printResult() {
  while (SHA256.available()) {
    byte b = SHA256.read();

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
  }
  Serial.println();
}

void HttpClass::download() {
  if (client.connectSSL("waiveplug.s3.us-east-2.amazonaws.com", 443)) {
    client.println("GET /010003.bin HTTP/1.0");
    client.println("Host: waiveplug.s3.us-east-2.amazonaws.com");
    client.println("User-Agent: waiveplug/" + String(VERSION));
    client.println("Accept: application/octet-stream");
    client.println();

    // skip headers
    unsigned long timeoutStart = millis();
    char prevPrevC = '\0';
    char prevC = '\0';
    char c = '\0';
    while ((client.connected() || client.available()) && ((millis() - timeoutStart) < 30000)) {
      if (client.available()) {
        prevPrevC = prevC;
        prevC = c;
        c = client.read();
        if (prevPrevC == '\n' && prevC == '\r' && c == '\n') {
          break;
        }
      } else {
        delay(1);
        Watchdog.reset();
      }
    }

    //read body
    timeoutStart = millis();
    int counter = 0;
    SHA256.beginHmac("Mqtt");

    while ((client.connected() || client.available()) && ((millis() - timeoutStart) < 30000)) {
      if (client.available()) {
        c = client.read();
        counter++;
        // Serial.write(c);
        SHA256.print(c);

      } else {
        delay(1);
        Watchdog.reset();
      }
    }
    log("https Sucessful: " + String(counter));
  } else {
    Serial.println("https failed");
  }

  SHA256.endHmac();
  printResult();
}

HttpClass Http;
