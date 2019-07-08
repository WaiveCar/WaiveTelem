#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
static WiFiClient client;
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include <MKRNB.h>
extern NB nbAccess;
static NBClient client;
#endif
static BearSSLClient sslClient(client);
static MqttClient mqttClient(sslClient);

#include "Config.h"
#include "Console.h"
#include "Mqtt.h"
#include "Pins.h"

static void onMessageReceived(int messageSize) {
  log("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  log("payload: " + payload);
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Failed to read file: " + String(error.c_str()));
    return;
  }
  String doors = doc["state"]["doors"].as<String>();
  String vehicle = doc["state"]["vehicle"].as<String>();
  if (doors == "unlocked") {
    Pins.unlockDoors();
    Mqtt.telemeter("{\"doors\": \"unlocked\"}", true);
  } else if (doors == "locked") {
    Pins.lockDoors();
    Mqtt.telemeter("{\"doors\": \"locked\"}", true);
  } else if (vehicle == "immobilized") {
    Pins.immobilize();
    Mqtt.telemeter("{\"vehicle\": \"immobilize\"}", true);
  } else if (vehicle == "unimmobilized") {
    Pins.unimmobilize();
    Mqtt.telemeter("{\"vehicle\": \"unimmobilize\"}", true);
  } else {
    Serial.println("Unknown command: " + payload);
  }
}

static unsigned long getTime() {
#ifdef ARDUINO_SAMD_MKR1000
  return WiFi.getTime();
#elif defined(ARDUINO_SAMD_MKRNB1500)
  return nbAccess.getTime();
#endif
}

void MqttClass::setup() {
  if (!ECCX08.begin()) {
    Serial.println(F("No ECCX08 present!"));
    while (1)
      ;
  }
  ArduinoBearSSL.onGetTime(getTime);
  log("id: " + Config.getId());
  // log("cert: " + Config.getMqttBrokerCert());
  sslClient.setEccSlot(0, Config.getMqttBrokerCert().c_str());
  mqttClient.setId(Config.getId());
  mqttClient.onMessage(onMessageReceived);
}

void MqttClass::connect() {
  log("Attempting to connect to MQTT broker: " + Config.getMqttBrokerUrl());
  const int maxTry = 10;
  int i = 0;
  while (!mqttClient.connect(Config.getMqttBrokerUrl().c_str(), 8883)) {
    i++;
    if (i == maxTry) {
      log(F("Failed to connect"));
      return;
    }
    Serial.print(F("M"));
    delay(3000);
  }
  log(F("You're connected to the MQTT broker"));
  mqttClient.subscribe("$aws/things/" + Config.getId() + "/shadow/update/delta");
}

bool MqttClass::isConnected() {
  return mqttClient.connected();
}

void MqttClass::poll() {
  mqttClient.poll();
}

void MqttClass::telemeter(String json, bool resetDesired) {
  String topic = "$aws/things/" + Config.getId() + "/shadow/update";
  String message = "{\"state\": {\"reported\": " + json + (resetDesired ? ", \"desired\": null" : "") + "}}";
  log("publish " + topic + " " + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
