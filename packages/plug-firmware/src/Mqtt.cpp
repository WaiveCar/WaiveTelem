#include <ArduinoBearSSL.h>
#include <ArduinoMqttClient.h>
#include <JsonLogger.h>
#include <Modem.h>
#include <WDTZero.h>

#include "Command.h"
#include "Eeprom.h"
#include "Internet.h"
#include "Mqtt.h"
#include "System.h"

// this is Amazon certificate, taken from ArduinoBearSSL BearSSLTrustAnchors.h
static const unsigned char TA1_DN[] = {
    0x30, 0x39, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
    0x02, 0x55, 0x53, 0x31, 0x0F, 0x30, 0x0D, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x13, 0x06, 0x41, 0x6D, 0x61, 0x7A, 0x6F, 0x6E, 0x31, 0x19, 0x30, 0x17,
    0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x10, 0x41, 0x6D, 0x61, 0x7A, 0x6F,
    0x6E, 0x20, 0x52, 0x6F, 0x6F, 0x74, 0x20, 0x43, 0x41, 0x20, 0x31};

static const unsigned char TA1_RSA_N[] = {
    0xB2, 0x78, 0x80, 0x71, 0xCA, 0x78, 0xD5, 0xE3, 0x71, 0xAF, 0x47, 0x80,
    0x50, 0x74, 0x7D, 0x6E, 0xD8, 0xD7, 0x88, 0x76, 0xF4, 0x99, 0x68, 0xF7,
    0x58, 0x21, 0x60, 0xF9, 0x74, 0x84, 0x01, 0x2F, 0xAC, 0x02, 0x2D, 0x86,
    0xD3, 0xA0, 0x43, 0x7A, 0x4E, 0xB2, 0xA4, 0xD0, 0x36, 0xBA, 0x01, 0xBE,
    0x8D, 0xDB, 0x48, 0xC8, 0x07, 0x17, 0x36, 0x4C, 0xF4, 0xEE, 0x88, 0x23,
    0xC7, 0x3E, 0xEB, 0x37, 0xF5, 0xB5, 0x19, 0xF8, 0x49, 0x68, 0xB0, 0xDE,
    0xD7, 0xB9, 0x76, 0x38, 0x1D, 0x61, 0x9E, 0xA4, 0xFE, 0x82, 0x36, 0xA5,
    0xE5, 0x4A, 0x56, 0xE4, 0x45, 0xE1, 0xF9, 0xFD, 0xB4, 0x16, 0xFA, 0x74,
    0xDA, 0x9C, 0x9B, 0x35, 0x39, 0x2F, 0xFA, 0xB0, 0x20, 0x50, 0x06, 0x6C,
    0x7A, 0xD0, 0x80, 0xB2, 0xA6, 0xF9, 0xAF, 0xEC, 0x47, 0x19, 0x8F, 0x50,
    0x38, 0x07, 0xDC, 0xA2, 0x87, 0x39, 0x58, 0xF8, 0xBA, 0xD5, 0xA9, 0xF9,
    0x48, 0x67, 0x30, 0x96, 0xEE, 0x94, 0x78, 0x5E, 0x6F, 0x89, 0xA3, 0x51,
    0xC0, 0x30, 0x86, 0x66, 0xA1, 0x45, 0x66, 0xBA, 0x54, 0xEB, 0xA3, 0xC3,
    0x91, 0xF9, 0x48, 0xDC, 0xFF, 0xD1, 0xE8, 0x30, 0x2D, 0x7D, 0x2D, 0x74,
    0x70, 0x35, 0xD7, 0x88, 0x24, 0xF7, 0x9E, 0xC4, 0x59, 0x6E, 0xBB, 0x73,
    0x87, 0x17, 0xF2, 0x32, 0x46, 0x28, 0xB8, 0x43, 0xFA, 0xB7, 0x1D, 0xAA,
    0xCA, 0xB4, 0xF2, 0x9F, 0x24, 0x0E, 0x2D, 0x4B, 0xF7, 0x71, 0x5C, 0x5E,
    0x69, 0xFF, 0xEA, 0x95, 0x02, 0xCB, 0x38, 0x8A, 0xAE, 0x50, 0x38, 0x6F,
    0xDB, 0xFB, 0x2D, 0x62, 0x1B, 0xC5, 0xC7, 0x1E, 0x54, 0xE1, 0x77, 0xE0,
    0x67, 0xC8, 0x0F, 0x9C, 0x87, 0x23, 0xD6, 0x3F, 0x40, 0x20, 0x7F, 0x20,
    0x80, 0xC4, 0x80, 0x4C, 0x3E, 0x3B, 0x24, 0x26, 0x8E, 0x04, 0xAE, 0x6C,
    0x9A, 0xC8, 0xAA, 0x0D};

static const unsigned char TA1_RSA_E[] = {
    0x01, 0x00, 0x01};

static const br_x509_trust_anchor TAs[] = {
    {{(unsigned char*)TA1_DN, sizeof TA1_DN},
     BR_X509_TA_CA,
     {BR_KEYTYPE_RSA,
      {.rsa = {
           (unsigned char*)TA1_RSA_N,
           sizeof TA1_RSA_N,
           (unsigned char*)TA1_RSA_E,
           sizeof TA1_RSA_E,
       }}}},
};

static InternetClient client;
static BearSSLClient sslClient(client, TAs, 1);
static MqttClient mqttClient(sslClient);

static void onMessageReceived(int messageSize) {
  String payload = mqttClient.readString();
  Command.processJson(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

int MqttClass::begin() {
  ArduinoBearSSL.onGetTime(getTime);

  sslClient.setEccSlot(0, Eeprom.getMqttCert(), Eeprom.getMqttCertLen());

  String id = System.getId();
  mqttClient.setId(id);
  mqttClient.setKeepAliveInterval(20 * 60 * 1000);
  mqttClient.onMessage(onMessageReceived);
  updateTopic = "$aws/things/" + id + "/shadow/update";
  logTopic = "things/" + id + "/log";
  return 1;
}

void MqttClass::connect() {
  if (!Internet.isConnected()) {
    if (!Internet.connect()) {
      return;
    }
  }
  int signalStrength = Internet.getSignalStrength();
  // logDebug("i|signal", signalStrength);
  if (signalStrength < 10) {
    return;
  }
  logInfo("broker", Eeprom.getMqttUrl());
  int start = millis();
  Watchdog.setup(WDT_SOFTCYCLE1M);
  // MODEM.debug(Serial);
  if (!mqttClient.connect(Eeprom.getMqttUrl(), 8883)) {
    System.setTimes(Internet.getTime());
    logWarn("i|error", mqttClient.connectError());
    Watchdog.setup(WDT_SOFTCYCLE8S);
    return;
  }
  // MODEM.noDebug();
  System.setTimes(Internet.getTime());
  logDebug("i|initTime", millis() - start, "You're connected to the MQTT broker");
  Watchdog.setup(WDT_SOFTCYCLE8S);
  mqttClient.subscribe("$aws/things/" + String(System.getId()) + "/shadow/update/delta");
  // to fix long mqtt cmd delay:
  // https://portal.u-blox.com/s/question/0D52p00008RlYDrCAN/long-delays-using-sarar41002b-with-att
  MODEM.send("AT+USOSO=0,6,1,1");
  MODEM.waitForResponse();
  MODEM.send("AT+USOSO=0,65535,8,1");
  MODEM.waitForResponse();
  MODEM.send("AT+CEDRXS=0");  // this is supposed to be stored in non-volatile memory, but it seems AT&T can flip it?
  MODEM.waitForResponse();
}

bool MqttClass::isConnected() {
  return Internet.isConnected() && mqttClient.connected();
}

void MqttClass::poll() {
  if (!Mqtt.isConnected()) {
    // try in 60 secs if not connected and not required to be responsive because cell and mqtt connect can take a long time (40 seconds not unusual)
    uint32_t elapsedTime = System.getTime() - lastConnectTry;
    if (!System.stayResponsive() && (lastConnectTry == -1 || elapsedTime >= 60)) {
      Mqtt.connect();
      lastConnectTry = System.getTime();
    }
  }
  // uint32_t start = millis();
  mqttClient.poll();
  // uint32_t total = millis() - start;
  // logDebug("i|pollTime", total); // poll takes 150ms over cellular
}

void MqttClass::updateShadow(const char* message, int len) {
  // logDebug("i|msgLen", strlen(message));
  mqttClient.beginMessage(updateTopic, len, false, 0, false);
  mqttClient.print(message);
  mqttClient.endMessage();
}

void MqttClass::logMsg(const char* message, int len) {
  mqttClient.beginMessage(logTopic, len, false, 0, false);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
