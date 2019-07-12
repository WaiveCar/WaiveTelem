#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
static WiFiClient client;
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include <MKRNB.h>
static NBClient client;
#include "Cellular.h"
#endif
static BearSSLClient sslClient(client);
static MqttClient mqttClient(sslClient);

#include "Config.h"
#include "Console.h"
#include "Mqtt.h"
#include "System.h"

static void onMessageReceived(int messageSize) {
  log("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  log("payload: " + payload);
  System.processCommand(payload);
}

static unsigned long getTime() {
#ifdef ARDUINO_SAMD_MKR1000
  return WiFi.getTime();
#elif defined(ARDUINO_SAMD_MKRNB1500)
  return Cellular.getTime();
#endif
}

void MqttClass::setup() {
  if (!ECCX08.begin()) {
    Serial.println(F("No ECCX08 present!"));
    while (1)
      ;
  }
  ArduinoBearSSL.onGetTime(getTime);
  log("id: " + String(Config.getId()));
  // log("cert: " + Config.getMqttBrokerCert());
  sslClient.setEccSlot(0, Config.getMqttBrokerCert());
  mqttClient.setId(Config.getId());
  mqttClient.onMessage(onMessageReceived);
}

void MqttClass::connect() {
  log("Attempting to connect to MQTT broker: " + String(Config.getMqttBrokerUrl()));
  const int maxTry = 20;
  int i = 0;
  while (!mqttClient.connect(Config.getMqttBrokerUrl(), 8883)) {
    Watchdog.reset();
    if (i == maxTry) {
      log(F("Failed to connect"));
      return;
    }
    Serial.print(F("M"));
    delay(3000);
    i++;
  }
  log(F("You're connected to the MQTT broker"));
  mqttClient.subscribe("$aws/things/" + String(Config.getId()) + "/shadow/update/delta");
}

bool MqttClass::isConnected() {
  return mqttClient.connected();
}

void MqttClass::poll() {
  mqttClient.poll();
}

void MqttClass::telemeter(const String& json) {
  String topic = "$aws/things/" + String(Config.getId()) + "/shadow/update";
  String message = "{\"state\": {\"reported\": " + json + "}}";
  log("publish " + topic + " " + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
