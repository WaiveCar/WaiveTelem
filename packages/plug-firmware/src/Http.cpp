#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#include <ArduinoBearSSL.h>
#include <SD.h>

#include "Config.h"
#include "Console.h"
#include "Http.h"
#include "Internet.h"
#include "Mqtt.h"
#include "System.h"

#define DOWNLOAD_TIMEOUT 60 * 1000
#define BUFFER_SIZE 512

static InternetClient client;

static void sendGetRequest(const String& host, const String& file) {
  client.println("GET /" + file + " HTTP/1.0");
  client.println("Host: " + host);
  client.println("User-Agent: waiveplug/" + String(FIRMWARE_VERSION));
  client.println("Accept: application/octet-stream");
  client.println();
}

static int32_t skipHeaders() {
  unsigned long timeoutStart = millis();
  char prevPrevC = '\0';
  char prevC = '\0';
  char c = '\0';
  while ((client.connected() || client.available()) && ((millis() - timeoutStart) < DOWNLOAD_TIMEOUT)) {
    if (client.available()) {
      prevPrevC = prevC;
      prevC = c;
      c = client.read();
      log(String(c));
      if (prevPrevC == '\n' && prevC == '\r' && c == '\n') {
        return 0;
      }
    } else {
      delay(1);
    }
    Watchdog.reset();
  }
  return -1;
}

static int32_t saveFile() {
  unsigned long timeoutStart = millis();
  int counter = 0;
  SHA256.beginHmac("https failed");
  File file = SD.open("TEMP", FILE_WRITE);
  if (!file) {
    Serial.println("file open failed");
    return -1;
  }
  uint8_t buf[BUFFER_SIZE];
  while ((client.connected() || client.available()) && ((millis() - timeoutStart) < DOWNLOAD_TIMEOUT)) {
    if (client.available()) {
      int bytesRead = client.read(buf, sizeof(buf));
      counter += bytesRead;
      SHA256.write(buf, bytesRead);
      file.write(buf, bytesRead);
    } else {
      delay(1);
    }
    Watchdog.reset();
  }
  logLine("total bytes downloaded: " + String(counter));
  SHA256.endHmac();
  file.close();
  return 0;
}

static int32_t verifyFile(const String& file) {
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
    return 0;
  } else {
    Serial.println("checksum failed");
    return -1;
  }
}

static int32_t moveFile() {
  File readFile = SD.open("TEMP", FILE_READ);
  if (!readFile) {
    Serial.println("readFile open failed");
    return -1;
  }
  File writeFile = SD.open("UPDATE.BIN", FILE_WRITE);
  if (!writeFile) {
    Serial.println("writeFile open failed");
    return -1;
  }
  uint8_t buf[BUFFER_SIZE];
  while (readFile.available()) {
    int bytesRead = readFile.read(buf, sizeof(buf));
    writeFile.write(buf, bytesRead);
    // logLine("write " + String(bytesRead));
    Watchdog.reset();
  }
  readFile.close();
  writeFile.close();
  SD.remove((char*)"TEMP");
  return 0;
}

void HttpClass::download(const String& host, const String& file) {
  logLine("host: " + host + ", file: " + file);
  int32_t error;
  if (client.connectSSL(host.c_str(), 443)) {
    sendGetRequest(host, file);
    error = skipHeaders();
    if (error) return;
    error = saveFile();
    if (error) return;
    error = verifyFile(file);
    if (error) return;
    error = moveFile();
    if (error) return;
    Mqtt.telemeter("", "{\"download\": null}");
    // reset
    Watchdog.enable(1000);
    delay(10 * 10000);
  } else {
    Serial.println(F("https failed"));
  }
}

HttpClass Http;
