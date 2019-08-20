#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoMqttClient.h>

#include "Command.h"
#include "Config.h"
#include "Https.h"
#include "Internet.h"
#include "Logger.h"
#include "Mqtt.h"
#include "System.h"

static InternetClient client;
static BearSSLClient sslClient(client);
static MqttClient mqttClient(sslClient);

static void onMessageReceived(int messageSize) {
  String payload = mqttClient.readString();
  logDebug("topic", mqttClient.messageTopic().c_str(), "i_length", messageSize, "payload", payload.c_str());
  Command.processJson(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

int MqttClass::begin() {
  ArduinoBearSSL.onGetTime(getTime);
  const char* cert = Config.get()["mqtt"]["cert"];
  if (!strlen(cert)) {
    return 1;
  }
  sslClient.setEccSlot(0, cert);
  String id = System.getId();
  mqttClient.setId(id);
  mqttClient.onMessage(onMessageReceived);
  updateTopic = "$aws/things/" + id + "/shadow/update";
  logTopic = "things/" + id + "/log";
  return 0;
}

void MqttClass::connect() {
  if (!Internet.isConnected()) {
    if (!Internet.connect()) {
      return;
    }
    // test internect connection
    // Watchdog.disable();
    // Https.download("waiveplug.s3.us-east-2.amazonaws.com", "config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "TEMP");  // connect sometimes works with DigiCertBaltimoreCA_2G2, but WR failed
    // Https.download("storage.googleapis.com", "www.swiperweb.com/privacy.html", "TEMP");  //works
    // Https.download("waive.blob.core.windows.net", "plug/config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "TEMP");
  }
#ifndef ARDUINO_SAMD_MKR1000
  return;  // mqtt doesn't work over cellular yet
#endif
  const JsonObject mqtt = Config.get()["mqtt"];
  const char* url = mqtt["url"] | "a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com";  // "waive.azure-devices.net";

  logInfo("broker", url);
  Watchdog.disable();  // Internet.connect() and mqttClient.connect() can take a long time
  if (!mqttClient.connect(url, 8883)) {
    Watchdog.enable(WATCHDOG_TIMEOUT);
    logWarn("i_error", mqttClient.connectError());
    return;
  }
  Watchdog.enable(WATCHDOG_TIMEOUT);
  logDebug("You're connected to the MQTT broker");
  mqttClient.subscribe("$aws/things/" + String(System.getId()) + "/shadow/update/delta");
}

bool MqttClass::isConnected() {
  return mqttClient.connected();
}

void MqttClass::poll() {
  if (!Mqtt.isConnected()) {
    Mqtt.connect();
  }
  mqttClient.poll();
}

void MqttClass::updateShadow(const String& message) {
  mqttClient.beginMessage(updateTopic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

// TODO: it seems first message will cause it to disconnect
void MqttClass::logMsg(const String& message) {
  mqttClient.beginMessage(logTopic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
