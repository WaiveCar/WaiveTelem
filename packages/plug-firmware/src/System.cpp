#include <Adafruit_SleepyDog.h>
#include <Arduino.h>
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>
#include <NMEAGPS.h>
#include <bearssl/bearssl_ssl.h>
#include <rBase64.h>

#include "Config.h"
#include "Gps.h"
#include "Https.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "Pins.h"
#include "System.h"

#define AUTH_DOC_SIZE 128
#define COMMAND_DOC_SIZE 512

extern "C" char* sbrk(int incr);
extern volatile uint32_t _ulTickCount;

int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void SystemClass::setup() {
  statusDoc.createNestedObject("can");
  statusDoc.createNestedObject("heartbeat");
  statusDoc["heartbeat"].createNestedObject("gps");
  statusDoc["heartbeat"].createNestedObject("system");

  // set bluetooth token key and iv
  const char* cert = Config.get()["mqtt"]["cert"];
  size_t length = rbase64_dec_len((char*)&cert[743], 64);
  char* buf = (char*)malloc(length);
  rbase64_decode(buf, (char*)&cert[743], 64);
  memcpy(tokenIv, buf, 16);
  memcpy(tokenKey, &buf[16], 32);
  free(buf);
}

void SystemClass::poll() {
  bool inRide = (statusDoc["inRide"] == "true");
  uint32_t interval = Config.get()["heartbeat"][inRide ? "inRide" : "notInRide"] | 60;
  // logInfo("time: " + String(time));
  // logInfo("lastHeartbeat: " + String(lastHeartbeat));
  if (time - lastHeartbeat == interval * 29 / 30 - 15) {
    Gps.setup();  // to wake GPS up
  } else if (lastHeartbeat == -1 || time - lastHeartbeat >= interval) {
    if (Gps.poll()) {
      sendHeartbeat();
      lastHeartbeat = time;
      Gps.sleep();
    }
  }
}

const char* SystemClass::getDateTime() {
  NeoGPS::time_t dt = time;
  sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.full_year(dt.year), dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
  return dateTime;
}

void SystemClass::sendInfo() {
  statusDoc["firmware"] = FIRMWARE_VERSION;
#ifdef DEBUG
  statusDoc["inRide"] = "true";
#else
  statusDoc["inRide"] = "false";
#endif
  String version = "{\"inRide\":\"" + String(statusDoc["inRide"].as<char*>()) + "\", \"system\":{\"firmware\":\"" +
                   FIRMWARE_VERSION + "\",\"configFreeMem\":" + Config.getConfigFreeMem() + "}}";
  telemeter(version);
}

void SystemClass::sendHeartbeat() {
  JsonObject heartbeat = statusDoc["heartbeat"];
  JsonObject gps = heartbeat["gps"];
  JsonObject system = heartbeat["system"];
  gps["lat"] = Gps.getLatitude() / 1e7;
  gps["long"] = Gps.getLongitude() / 1e7;
  gps["hdop"] = Gps.getHdop();
  gps["speed"] = Gps.getSpeed();
  gps["heading"] = Gps.getHeading();
  system["dateTime"] = System.getDateTime();
  system["uptime"] = time - bootTime;
  system["signalStrength"] = Internet.getSignalStrength();
  system["heapFreeMem"] = freeMemory();
  system["statusFreeMem"] = STATUS_DOC_SIZE - statusDoc.memoryUsage();
  telemeter(statusDoc["heartbeat"].as<String>());
}

void SystemClass::sendCanStatus() {
  if (canStatusChanged) {
    telemeter("{\"can\":" + statusDoc["can"].as<String>() + "}");
    canStatusChanged = false;
  }
}

void SystemClass::setCanStatus(const String& name, uint64_t value, uint32_t delta) {
  JsonObject can = statusDoc["can"];
  uint64_t oldValue = can[name];
  if (oldValue != value) {
    can[name] = value;
    if (abs(oldValue - value) >= delta) {
      canStatusChanged = true;
    }
  }
}

void SystemClass::authorizeCommand(const String& encrypted) {
  String json = decryptToken(encrypted);
  StaticJsonDocument<AUTH_DOC_SIZE> authDoc;
  DeserializationError error = deserializeJson(authDoc, json);
  if (error) {
    logError("Failed to read json: " + String(error.c_str()));
    return;
  }
  authCmds = authDoc["cmds"] | "";
  authStart = authDoc["start"] | 0;
  authEnd = authDoc["end"] | 0;
  const char* secret = authDoc["secret"] | "";
  rbase64_decode((char*)authSecret, (char*)secret, 20);
}

uint8_t* SystemClass::getAuthSecret() {
  return authSecret;
}

void SystemClass::reportCommandDone(const String& json, String& cmdKey, String& cmdValue) {
  String escapedJson = json;
  escapedJson.replace("\"", "\\\"");
  String lastCmd = "\"system\":{\"lastCmd\":\"" + escapedJson + "\"}";
  telemeter("{" + lastCmd + +",\"" + cmdKey + "\":\"" + cmdValue + "\"}", "{\"" + cmdKey + "\":null}");
}

void SystemClass::processCommand(const String& json, bool isBluetooth) {
  StaticJsonDocument<COMMAND_DOC_SIZE> cmdDoc;
  DeserializationError error = deserializeJson(cmdDoc, json);
  if (error) {
    logError("Failed to read json: " + String(error.c_str()));
    return;
  }
  JsonObject desired = isBluetooth ? cmdDoc.as<JsonObject>() : cmdDoc["state"];
  String cmdKey, cmdValue;
  for (JsonPair p : desired) {
    cmdKey = p.key().c_str();
    cmdValue = p.value().as<String>();
  }
  if (isBluetooth) {
    if (authCmds.indexOf(cmdKey) == -1) {
      logError("authCmds: " + authCmds + ", cmdKey: " + cmdKey);
      return;
    }
    if (authStart > time) {
      logError("authStart: " + String(authStart) + ", time: " + time);
      return;
    }
    if (authEnd < time) {
      logError("authEnd: " + String(authEnd) + ", time: " + time);
      return;
    }
  }
  JsonObject download = desired["download"];
  JsonObject copy = desired["copy"];
  if (cmdKey == "lock" && cmdValue == "open") {
    Pins.unlockDoors();
  } else if (cmdKey == "lock" && cmdValue == "close") {
    Pins.lockDoors();
  } else if (cmdKey == "immo" && cmdValue == "lock") {
    Pins.immobilize();
  } else if (cmdKey == "immo" && cmdValue == "unlock") {
    Pins.unimmobilize();
  } else if (cmdKey == "inRide" && (cmdValue == "true" || cmdValue == "false")) {
    canStatusChanged = true;
  } else if (cmdKey == "reboot" && cmdValue == "true") {
    reportCommandDone(json, cmdKey, cmdValue);
    reboot();
    return;
  } else if (!download.isNull()) {
    const char* host = download["host"] | "";
    const char* from = download["from"] | "";
    const char* to = download["to"] | "";
    if (strlen(host) > 0 && strlen(from) > 0 && strlen(to) > 0) {
      Https.download(host, from, to);
      reportCommandDone(json, cmdKey, cmdValue);
      reboot();
    } else {
      logError("Error: " + json);
    }
    return;
  } else if (!copy.isNull()) {
    const char* from = copy["from"] | "";
    const char* to = copy["to"] | "";
    if (strlen(from) > 0 && strlen(to) > 0) {
      copyFile(from, to);
      reportCommandDone(json, cmdKey, cmdValue);
      reboot();
    } else {
      logError("Error: " + json);
    }
    return;
  } else {
    logError("Unknown command: " + json);
    return;
  }
  statusDoc[cmdKey] = cmdValue;
  reportCommandDone(json, cmdKey, cmdValue);
}

void SystemClass::sleep(uint32_t sec) {
  digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG
  // don't use Watchdog.sleep as it disconnects USB
  delay(sec * 1000);
#else
  // Watchdog.sleep(sec * 1000);  // if USB monitoring, it won't sleep
  // _ulTickCount = _ulTickCount + sec * 1000;
  delay(sec * 1000);
#endif
  if (time % 30 == 0 && Internet.isConnected()) {  // don't get time from modem too often; only every 30 secs
    setTime(Internet.getTime());
  } else {
    time += sec;  // not very accurate about 1.5% longer
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void SystemClass::reboot() {
  logInfo(F("Rebooting now"));
  delay(1000);
  Watchdog.enable(1);
  while (true)
    ;
}

int32_t SystemClass::moveFile(const char* from, const char* to) {
  int32_t error = copyFile(from, to);
  if (!error) {
    SD.remove((char*)from);
  }
  return error;
}

int32_t SystemClass::copyFile(const char* from, const char* to) {
  File readFile = SD.open(from, FILE_READ);
  if (!readFile) {
    logError("readFile open failed");
    return -1;
  }
  File writeFile = SD.open(to, FILE_WRITE);
  if (!writeFile) {
    logError("writeFile open failed");
    return -1;
  }
  writeFile.seek(0);  // workaround BUG in SD to default to append
  uint8_t buf[BUFFER_SIZE];
  while (readFile.available()) {
    int bytesRead = readFile.read(buf, sizeof(buf));
    writeFile.write(buf, bytesRead);
    // logDebug("write " + String(bytesRead));
    Watchdog.reset();
  }
  readFile.close();
  writeFile.close();
  return 0;
}

void SystemClass::setTime(uint32_t in) {
  time = in;
  if (bootTime == 0) {
    bootTime = in;
  }
}

void SystemClass::telemeter(const String& reported, const String& desired) {
  String message = "{\"state\":{" +
                   (reported != "" ? "\"reported\":" + reported : "") +
                   (reported != "" && desired != "" ? "," : "") +
                   (desired != "" ? "\"desired\":" + desired : "") + "}}";
  Logger.logLine("Debug", message);
  if (Mqtt.isConnected()) {
    Mqtt.send(message);
  }
}

String SystemClass::decryptToken(const String& encrypted) {
  // see https://github.com/nogoegst/bearssl/blob/master/test/test_crypto.c
  unsigned char iv[16];
  br_aes_gen_cbcdec_keys v_dc;
  const br_block_cbcdec_class** dc;
  const br_block_cbcdec_class* vd = &br_aes_big_cbcdec_vtable;
  dc = &v_dc.vtable;
  vd->init(dc, tokenKey, 32);
  memcpy(iv, tokenIv, 16);
  size_t length = rbase64_dec_len((char*)encrypted.c_str(), encrypted.length());
  char* buf = (char*)malloc(length);
  rbase64_decode(buf, (char*)encrypted.c_str(), encrypted.length());
  for (size_t v = 0; v < length; v += 16) {
    vd->run(dc, iv, buf + v, 16);
  }
  size_t numberOfPadding = buf[length - 1];
  buf[length - numberOfPadding] = '\0';
  logDebug(buf);
  String token(buf);
  free(buf);
  return token;
}

void SystemClass::unauthorize() {
  authCmds = "";
  authStart = 0;
  authEnd = 0;
}

bool SystemClass::getStayAwake() {
  return stayAwake;
}

void SystemClass::setStayAwake(bool stay) {
  stayAwake = stay;
}

SystemClass System;
