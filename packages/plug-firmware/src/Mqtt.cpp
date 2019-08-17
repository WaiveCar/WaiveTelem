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
  log("DEBUG", "Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  log("DEBUG", "payload: " + payload);
  Command.processJson(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

void MqttClass::begin() {
  log("DEBUG");
  ArduinoBearSSL.onGetTime(getTime);
  JsonObject mqtt = Config.get()["mqtt"];
  const char* cert = mqtt["cert"];
  // log("DEBUG", "cert: " + cert);
  sslClient.setEccSlot(0, cert);
  mqttClient.setId(System.getId());
  mqttClient.onMessage(onMessageReceived);
}

void MqttClass::connect() {
  Watchdog.disable();  // Internet.connect() and mqttClient.connect() can take a long time
  if (!Internet.isConnected()) {
    if (!Internet.connect()) {
      Watchdog.enable();
      return;
    }
    System.keepTime();
    // test internect connection
    // Watchdog.disable();
    // Https.download("waiveplug.s3.us-east-2.amazonaws.com", "config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "TEMP");  // connect sometimes works with DigiCertBaltimoreCA_2G2, but WR failed
    // Https.download("storage.googleapis.com", "www.swiperweb.com/privacy.html", "TEMP");  //works
    // Https.download("waive.blob.core.windows.net", "plug/config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "TEMP");
  }
  const JsonObject mqtt = Config.get()["mqtt"];
  const char* url = mqtt["url"] | "a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com";  // "waive.azure-devices.net";

  log("INFO ", "broker", url);
  if (!mqttClient.connect(url, 8883)) {
    Watchdog.enable();
    log("WARN ", "error", String(mqttClient.connectError()).c_str());
    return;
  }
  Watchdog.enable();
  log("DEBUG", "You're connected to the MQTT broker");
  String id = String(System.getId());
  mqttClient.subscribe("$aws/things/" + id + "/shadow/update/delta");
  updateTopic = "$aws/things/" + id + "/shadow/update";
  // things/+/log
  logTopic = "things/" + id + "/log";
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

void MqttClass::logMsg(const String& message) {
  mqttClient.beginMessage(logTopic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
