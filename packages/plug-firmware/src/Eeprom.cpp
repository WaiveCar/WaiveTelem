#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <ArduinoJson.h>
#include <JsonLogger.h>
#include <SD.h>

#include "Eeprom.h"

#define MQTT_FILE "MQTT.TXT"

struct slot {
  uint8_t num;
  uint16_t maxLen;
};

const struct slot slots[] = {
    {.num = 8, .maxLen = 416},
    {.num = 9, .maxLen = 72},
    {.num = 10, .maxLen = 72},
    {.num = 11, .maxLen = 72},
    {.num = 12, .maxLen = 72},
    {.num = 13, .maxLen = 72},
};

int EepromClass::begin() {
  File file = SD.open(MQTT_FILE);
  if (file) {
    const size_t capacity = JSON_OBJECT_SIZE(3) + 1260;
    DynamicJsonDocument doc(capacity);
    // go to https://arduinojson.org/v6/assistant/ to find the size
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      logError("error", error.c_str());
    }

    const char* type = doc["type"];
    if (strcmp(type, "x509") == 0) {
      const char* url = doc["url"];
      const char* cert = doc["cert"];

      decodeMqttConfig(1, url, cert);
    }

    file.close();
    SD.remove((char*)MQTT_FILE);
  }

  loadMqttConfig();

  return 1;
}

char* EepromClass::getMqttUrl() {
  return mqttUrl;
}

uint8_t* EepromClass::getMqttCert() {
  return mqttCert;
}

uint16_t EepromClass::getMqttCertLen() {
  logTrace("i|mqttCertLen", mqttCertLen);
  return mqttCertLen;
}

uint8_t* EepromClass::getTokenKey() {
  return &mqttCert[0x220];
}

uint8_t* EepromClass::getTokenIv() {
  return &mqttCert[0x210];
}

int EepromClass::decodeMqttConfig(uint8_t type, const char* url, const char* cert) {
  // try to decode the cert
  br_pem_decoder_context pemDecoder;

  size_t certLen = strlen(cert);

  // assume the decoded cert is 3/4 the length of the input
  _ecCert.data = (unsigned char*)malloc(((certLen * 3) + 3) / 4);
  _ecCert.data_len = 0;

  br_pem_decoder_init(&pemDecoder);

  while (certLen) {
    size_t len = br_pem_decoder_push(&pemDecoder, cert, certLen);

    cert += len;
    certLen -= len;

    switch (br_pem_decoder_event(&pemDecoder)) {
      case BR_PEM_BEGIN_OBJ:
        br_pem_decoder_setdest(&pemDecoder, EepromClass::clientAppendCert, this);
        break;

      case BR_PEM_END_OBJ:
        if (_ecCert.data_len) {
          // done
          int ret = saveMqttConfig(type, url);
          free(_ecCert.data);
          return ret;
        }
        break;

      case BR_PEM_ERROR:
        // failure
        free(_ecCert.data);
        return -1;
    }
  }
  return -2;
}

int EepromClass::saveMqttConfig(uint8_t type, const char* url) {
  // write config type, size info and url
  uint8_t info[72];
  info[0] = type;

  if (type == 1) {
    uint16_t certLen = _ecCert.data_len;
    if (certLen > 416 + 72 * 5) {
      return -2;  // too long
    }
    // save mqttUrl
    uint8_t urlLen = strlen(url);
    info[1] = urlLen;
    info[2] = certLen;
    info[3] = certLen >> 8;
    memcpy(&info[4], url, urlLen + 1);
    int ret = ECCX08.writeSlot(14, info, ((urlLen + 1 + 2 + 3) / 4) * 4);
    if (ret != 1) {
      logError("i|ret", ret);
      return -3;
    }
    // save mqttCert
    uint16_t bytesToWrite = ((int)((certLen + 3) / 4)) * 4;
    uint8_t i = 0;
    uint16_t offset = 0;
    while (bytesToWrite > 0) {
      uint16_t maxLen = slots[i].maxLen;
      uint8_t slotNum = slots[i].num;
      uint16_t slotLenToWrite = bytesToWrite > maxLen ? maxLen : bytesToWrite;
      int ret = ECCX08.writeSlot(slotNum, &_ecCert.data[offset], slotLenToWrite);
      if (ret != 1) {
        logError("i|ret", ret, "i|slotNum", slotNum, "i|slotLenToWrite", slotLenToWrite);
        return -3;
      } else {
        logTrace("i|data", *((uint32_t*)&_ecCert.data[offset]));
      }
      bytesToWrite -= slotLenToWrite;
      offset += slotLenToWrite;
      i++;
    }
    return 1;
  } else {
    return -1;
  }
}

int EepromClass::loadMqttConfig() {
  // read config type and size info
  uint8_t info[72];
  int ret = ECCX08.readSlot(14, info, 72);
  if (ret != 1) {
    logError("i|ret", ret);
    return -3;
  }
  uint8_t type = info[0];
  logTrace("i|type", type);
  if (type == 1) {
    // load mqttUrl
    uint8_t urlLen = info[1];
    logTrace("i|urlLen", urlLen);
    mqttUrl = (char*)malloc(urlLen + 1);
    memcpy(mqttUrl, &info[4], urlLen + 1);

    // load mqttCertLen and mqttCert
    mqttCertLen = info[2] | info[3] << 8;
    logTrace("i|mqttCertLen", mqttCertLen);
    if (mqttCertLen > 416 + 72 * 5) {
      return -2;  // too long
    }
    uint16_t bytesToRead = ((int)((mqttCertLen + 3) / 4)) * 4;
    mqttCert = (uint8_t*)malloc(bytesToRead);  // round up to multiple of 4

    uint8_t i = 0;
    uint16_t offset = 0;
    while (bytesToRead > 0) {
      uint16_t maxLen = slots[i].maxLen;
      uint8_t slotNum = slots[i].num;
      uint16_t slotLenToRead = bytesToRead > maxLen ? maxLen : bytesToRead;
      int ret = ECCX08.readSlot(slotNum, &mqttCert[offset], slotLenToRead);
      if (ret != 1) {
        logError("i|ret", ret, "i|slotNum", slotNum, "i|slotLenToRead", slotLenToRead);
        return -3;
      } else {
        logTrace("i|data", *((uint32_t*)&mqttCert[offset]));
      }
      bytesToRead -= slotLenToRead;
      offset += slotLenToRead;
      i++;
    }
    return 1;

  } else {
    return -1;
  }
}

void EepromClass::clientAppendCert(void* ctx, const void* data, size_t len) {
  EepromClass* c = (EepromClass*)ctx;

  memcpy(&c->_ecCert.data[c->_ecCert.data_len], data, len);
  c->_ecCert.data_len += len;
}

EepromClass Eeprom;
