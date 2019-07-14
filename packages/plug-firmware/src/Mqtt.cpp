#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

#include "Config.h"
#include "Console.h"
#include "Http.h"
#include "Internet.h"
#include "Mqtt.h"
#include "System.h"

static InternetClient client;
static BearSSLClient sslClient(client);
static MqttClient mqttClient(sslClient);

static void onMessageReceived(int messageSize) {
  logLine("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  logLine("payload: " + payload);
  System.processCommand(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

void MqttClass::setup() {
  if (!ECCX08.begin()) {
    Serial.println(F("No ECCX08 present"));
    while (1)
      ;
  }
  ArduinoBearSSL.onGetTime(getTime);
  logLine("id: " + String(Config.getId()));
  // logLine("cert: " + Config.getMqttBrokerCert());
  sslClient.setEccSlot(0, Config.getMqttBrokerCert());
  mqttClient.setId(Config.getId());
  mqttClient.onMessage(onMessageReceived);
  connect();
}

void MqttClass::connect() {
  if (!Internet.isConnected()) {
    Internet.connect();
  }
  logLine("Attempting to connect to MQTT broker: " + String(Config.getMqttBrokerUrl()));
  const int maxTry = 20;
  int i = 0;
  while (!mqttClient.connect(Config.getMqttBrokerUrl(), 8883)) {
    Watchdog.reset();
    if (i == maxTry) {
      logLine(F("Failed to connect, try leter"));
      return;
    }
    log(F("M"));
    delay(3000);
    i++;
  }
  logLine(F("You're connected to the MQTT broker"));
  mqttClient.subscribe("$aws/things/" + String(Config.getId()) + "/shadow/update/delta");
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

void MqttClass::telemeter(const String& reported, const String& desired) {
  String topic = "$aws/things/" + String(Config.getId()) + "/shadow/update";
  String message = "{\"state\": {" +
                   (reported != "" ? "\"reported\": " + reported : "") +
                   (desired != "" ? "\"desired\": " + desired : "") + "}}";
  logLine("publish " + topic + " " + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
