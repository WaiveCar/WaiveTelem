#include <Arduino.h>
#include <JsonLogger.h>
#include <SD.h>
#include <WDTZero.h>
#include <bearssl/bearssl_ssl.h>
#include <rBase64.h>

#include "Can.h"
#include "Command.h"
#include "Eeprom.h"
#include "Gps.h"
#include "Https.h"
#include "Pins.h"
#include "System.h"

#define AUTH_DOC_SIZE 256
#define COMMAND_DOC_SIZE 512

void CommandClass::authorize(const String& encrypted) {
  String json = decryptToken(encrypted);
  StaticJsonDocument<AUTH_DOC_SIZE> authDoc;
  DeserializationError error = deserializeJson(authDoc, json);
  if (error) {
    logError("error", error.c_str(), "json", json.c_str(), "Failed to read json");
    return;
  }
  authCmds = authDoc["cmds"] | "";
  authStart = authDoc["start"] | 0;
  authEnd = authDoc["end"] | 0;
  logInfo("authCmds", authCmds.c_str(), "i|authStart", authStart, "i|authEnd", authEnd);
  const char* secret = authDoc["secret"] | "";
  rbase64_decode((char*)authSecret, (char*)secret, 16);
}

uint8_t* CommandClass::getAuthSecret() {
  return authSecret;
}

void CommandClass::processJson(const String& json, bool isBluetooth) {
  logInfo("json", json.c_str());
  StaticJsonDocument<COMMAND_DOC_SIZE> cmdDoc;
  DeserializationError error = deserializeJson(cmdDoc, json);
  if (error) {
    logError("error", error.c_str(), "Failed to read json");
    return;
  }
  JsonObject desired = isBluetooth ? cmdDoc.as<JsonObject>() : cmdDoc["state"];
  String lastCmd;
  serializeJson(desired, lastCmd);
  String cmdKey, cmdValue;
  for (JsonPair p : desired) {
    cmdKey = p.key().c_str();
    cmdValue = p.value().as<String>();
  }
  if (isBluetooth) {
    if (authCmds.indexOf(cmdKey) == -1) {
      logError("authCmds", authCmds.c_str(), "cmdKey", cmdKey.c_str(), "No Auth");
      return;
    }
    uint32_t time = System.getTime();
    if (authStart > time) {
      logError("i|authStart", authStart, "i|time", time, "No Auth");
      return;
    }
    if (authEnd < time) {
      logError("i|authEnd", authStart, "i|time", time, "No Auth");
      return;
    }
  }
  JsonVariant download = desired["download"];
  JsonVariant copy = desired["copy"];
  if (cmdKey == "lock" && cmdValue == "open") {
    Pins.unlockDoors();
  } else if (cmdKey == "lock" && cmdValue == "close") {
    Pins.lockDoors();
  } else if (cmdKey == "immo" && cmdValue == "lock") {
    Pins.immobilize();
  } else if (cmdKey == "immo" && cmdValue == "unlock") {
    Pins.unimmobilize();
  } else if (cmdKey == "can") {
    Can.sendCommand(cmdValue.c_str());
    System.reportCommandDone(lastCmd.c_str(), cmdKey.c_str(), NULL);
    return;
  } else if (cmdKey == "inRide" && cmdValue == "true") {
    System.setInRide(true);
    System.sendNotYetCanStatus();
    Gps.wakeup();
  } else if (cmdKey == "inRide" && cmdValue == "false") {
    System.setInRide(false);
    System.sendNotYetCanStatus();
    // Can.sleep();
  } else if (cmdKey == "reboot" && cmdValue == "true") {
    System.reportCommandDone(lastCmd.c_str(), cmdKey.c_str(), NULL);
    reboot();
    return;
  } else if (cmdKey == "remoteLog") {
    System.setRemoteLogLevel(cmdValue.toInt());
  } else if (download) {
    System.reportCommandDone(lastCmd.c_str(), cmdKey.c_str(), NULL);
    const char* host = download["host"] | "";
    const char* from = download["from"] | "";
    const char* to = download["to"] | "";
    if (strlen(host) > 0 && strlen(from) > 0 && strlen(to) > 0) {
      if (!Https.download(host, from, to)) {
        reboot();
      }
    } else {
      logError(json.c_str(), "Invalid download object");
    }
    return;
  } else if (copy) {
    System.reportCommandDone(lastCmd.c_str(), cmdKey.c_str(), NULL);
    const char* from = copy["from"] | "";
    const char* to = copy["to"] | "";
    if (strlen(from) > 0 && strlen(to) > 0) {
      if (!copyFile(from, to)) {
        reboot();
      }
    } else {
      logError(json.c_str(), "Invalid copy object");
    }
    return;
  } else {
    logError(json.c_str());
    return;
  }
  System.reportCommandDone(lastCmd.c_str(), cmdKey.c_str(), cmdValue.c_str());
}

void CommandClass::reboot() {
  Watchdog.detachShutdown();
  Watchdog.setup(WDT_HARDCYCLE4S);
  while (true)
    ;
}

int32_t CommandClass::moveFile(const char* from, const char* to) {
  int32_t error = copyFile(from, to);
  if (!error) {
    SD.remove((char*)from);
  }
  return error;
}

int32_t CommandClass::copyFile(const char* from, const char* to) {
  logDebug("from", from, "to", to);
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
    // logDebug( "write " + String(bytesRead));
    Watchdog.clear();
  }
  readFile.close();
  writeFile.close();
  return 0;
}

String CommandClass::decryptToken(const String& encrypted) {
  // see https://github.com/nogoegst/bearssl/blob/master/test/test_crypto.c
  unsigned char iv[16];
  br_aes_gen_cbcdec_keys v_dc;
  const br_block_cbcdec_class** dc;
  const br_block_cbcdec_class* vd = &br_aes_big_cbcdec_vtable;
  dc = &v_dc.vtable;
  vd->init(dc, Eeprom.getTokenKey(), 32);
  memcpy(iv, Eeprom.getTokenIv(), 16);
  size_t length = rbase64_dec_len((char*)encrypted.c_str(), encrypted.length());
  char* buf = (char*)malloc(length);
  rbase64_decode(buf, (char*)encrypted.c_str(), encrypted.length());
  for (size_t v = 0; v < length; v += 16) {
    vd->run(dc, iv, buf + v, 16);
  }
  size_t numberOfPadding = buf[length - 1];
  buf[length - numberOfPadding] = '\0';
  logTrace(buf);
  String token(buf);
  free(buf);
  return token;
}

void CommandClass::unauthorize() {
  authCmds = "";
  authStart = 0;
  authEnd = 0;
}

CommandClass Command;