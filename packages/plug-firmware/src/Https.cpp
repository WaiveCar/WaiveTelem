#include <Arduino.h>
#include <ArduinoBearSSL.h>
#include <JsonLogger.h>
#include <SD.h>
#include <WDTZero.h>

#include "Command.h"
#include "Https.h"
#include "Internet.h"

#ifndef ARDUINO_SAMD_MKR1000
#include <utility/NBRootCerts.h>
static InternetSslClient client(NULL, 0);
#else
static InternetSslClient client;
#endif

#define DOWNLOAD_TIMEOUT 30 * 60 * 1000

static void sendGetRequest(const String& host, const String& file) {
  client.println("GET /" + file + " HTTP/1.0");
  client.println("Host: " + host);
  client.println("User-Agent: waive/" + String(FIRMWARE_VERSION));
  client.println("Accept: application/octet-stream");
  client.println();
}

static int32_t skipHeaders() {
  char prevPrevC = '\0';
  char prevC = '\0';
  char c = '\0';
  unsigned long start = millis();
  while ((client.connected() || client.available()) && (millis() - start < DOWNLOAD_TIMEOUT)) {
    if (client.available()) {
      prevPrevC = prevC;
      prevC = c;
      c = client.read();
      // Serial.print(String(c));
      if (prevPrevC == '\n' && prevC == '\r' && c == '\n') {
        return 0;
      }
    } else {
      delay(10);
    }
    Watchdog.clear();
  }
  return -1;
}

static int32_t saveFile(const char* to) {
  int counter = 0;
  SHA256.beginHmac("https failed");
  File file = SD.open(to, FILE_WRITE);
  if (!file) {
    logError("file open failed");
    return -1;
  }
  file.seek(0);
  uint8_t buf[BUFFER_SIZE];
  unsigned long start = millis();
  while ((client.connected() || client.available()) && (millis() - start < DOWNLOAD_TIMEOUT)) {
    if (client.available()) {
      int bytesRead = client.read(buf, sizeof(buf));
      // Serial.println(String(bytesRead));
      counter += bytesRead;
      SHA256.write(buf, bytesRead);
      file.write(buf, bytesRead);
    } else {
      delay(10);
    }
    Watchdog.clear();
  }
  logDebug("i|totalBytes", counter);
  SHA256.endHmac();
  file.close();
  return 0;
}

static int32_t verifyFile(const String& file) {
  String computed = "";
  while (SHA256.available()) {
    uint8_t b = SHA256.read();
    if (b < 16) {
      computed += "0";
    }
    computed += String(b, HEX);
  }
  String sha256 = file.substring(file.lastIndexOf("_") + 1);
  logDebug("computed", computed.c_str());
  if (sha256 == computed) {
    return 0;
  } else {
    logError("checksum failed");
    return -1;
  }
}

int32_t HttpsClass::download(const char* host, const char* from, const char* to) {
  logInfo("host", host, "from", from, "to", to);
  int32_t error;
  // Watchdog.clear();
  Watchdog.setup(WDT_SOFTCYCLE32S);
  int start = millis();
  if (client.connect(host, 443)) {
    logInfo("i|initTime", millis() - start);
    Watchdog.setup(WDT_SOFTCYCLE8S);
    sendGetRequest(host, from);
    error = skipHeaders();
    if (error) return error;
    error = saveFile("TEMP");
    if (error) return error;
    error = verifyFile(from);
    if (error) return error;
    return Command.moveFile("TEMP", to);
  } else {
    logError("https failed");
    return -10;
  }
}

HttpsClass Https;
