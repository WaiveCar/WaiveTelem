#ifndef Eeprom_h
#define Eeprom_h

#include <Arduino.h>
#include <BearSSLClient.h>

class EepromClass {
 public:
  int begin();
  char* getMqttUrl();
  uint8_t* getMqttCert();
  uint16_t getMqttCertLen();
  uint8_t* getTokenKey();
  uint8_t* getTokenIv();

 private:
  int decodeMqttConfig(uint8_t type, const char* url, const char* cert);
  int saveMqttConfig(uint8_t type, const char* url);
  int loadMqttConfig();
  static void clientAppendCert(void* ctx, const void* data, size_t len);

  char* mqttUrl;
  uint8_t* mqttCert;
  uint16_t mqttCertLen;
  br_x509_certificate _ecCert;
};

extern EepromClass Eeprom;

#endif  // Eeprom_h