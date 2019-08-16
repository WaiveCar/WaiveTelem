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
  logDebug("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  logDebug("payload: " + payload);
  Command.processJson(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

void MqttClass::setup() {
  logFunc();
  ArduinoBearSSL.onGetTime(getTime);
  JsonObject mqtt = Config.get()["mqtt"];
  const char* cert = mqtt["cert"];
  logDebug("cert: " + String(cert));
  sslClient.setEccSlot(0, cert);
  mqttClient.setId(System.getId());
  mqttClient.onMessage(onMessageReceived);
}

void MqttClass::connect() {
  logFunc();
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
  const JsonObject mqtt = Config.get()["mqtt"];
  const char* url = mqtt["url"] | "a2ink9r2yi1ntl-ats.iot.us-east-2.amazonaws.com";  // "waive.azure-devices.net";

  logDebug("Attempting to connect to MQTT broker: " + String(url));
  const int maxTry = 10;
  int i = 1;
  while (!mqttClient.connect(url, 8883)) {
    Watchdog.reset();
    if (i == maxTry) {
      logDebug(F("Failed to connect, try later"));
      return;
    }
    logDebug("MQTT connect error: " + String(mqttClient.connectError()));
    delay(5000);
    i++;
  }
  logDebug(F("You're connected to the MQTT broker"));
  String id = System.getId();
  mqttClient.subscribe("$aws/things/" + id + "/shadow/update/delta");
  topic = "$aws/things/" + id + "/shadow/update";
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

void MqttClass::send(const String& message) {
  logFunc();
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
