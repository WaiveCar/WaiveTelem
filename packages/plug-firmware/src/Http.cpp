#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoBearSSL.h>

#include "Config.h"
#include "Console.h"
#include "Http.h"
#include "Internet.h"
#include "System.h"

static InternetClient client;

static void sendGetRequest(String& host, String& file) {
  client.println("GET /" + file + " HTTP/1.0");
  client.println("Host: " + host);
  client.println("User-Agent: waiveplug/" + String(FIRMWARE_VERSION));
  client.println("Accept: application/octet-stream");
  client.println();
}

static void skipHeaders() {
  unsigned long timeoutStart = millis();
  char prevPrevC = '\0';
  char prevC = '\0';
  char c = '\0';
  while ((client.connected() || client.available()) && ((millis() - timeoutStart) < 30000)) {
    if (client.available()) {
      prevPrevC = prevC;
      prevC = c;
      c = client.read();
      log(String(c));
      if (prevPrevC == '\n' && prevC == '\r' && c == '\n') {
        break;
      }
    } else {
      delay(1);
      Watchdog.reset();
    }
  }
}

static void saveFile() {
  unsigned long timeoutStart = millis();
  int counter = 0;
  SHA256.beginHmac("https failed");

  while ((client.connected() || client.available()) && ((millis() - timeoutStart) < 30000)) {
    if (client.available()) {
      char c = client.read();
      counter++;
      SHA256.print(c);
    } else {
      delay(1);
      Watchdog.reset();
    }
  }
  logLine("total bytes downloaded: " + String(counter));
  SHA256.endHmac();
}

static void moveFile(String& file) {
  String computed = "";
  while (SHA256.available()) {
    byte b = SHA256.read();
    if (b < 16) {
      computed += "0";
    }
    computed += String(b, HEX);
  }
  String sha256 = file.substring(file.lastIndexOf("_") + 1);
  logLine(sha256);
  logLine(computed);
  if (sha256 == computed) {
    logLine("checksum passed");
  }
}

void HttpClass::download(String& host, String& file) {
  if (client.connectSSL(host.c_str(), 443)) {
    sendGetRequest(host, file);
    skipHeaders();
    saveFile();
    moveFile(file);
  } else {
    Serial.println(F("https failed"));
  }
}

HttpClass Http;
