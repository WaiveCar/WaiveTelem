#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
WiFiClient client;
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include <MKRNB.h>
NBClient client;
#endif
BearSSLClient sslClient(client);
MqttClient mqttClient(sslClient);

#include "Config.h"
#include "Console.h"
#include "Mqtt.h"

void onMessageReceived(int messageSize) {
  log("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  while (mqttClient.available()) {
    log((char)mqttClient.read());
  }
}

unsigned long getTime() {
#ifdef ARDUINO_SAMD_MKR1000
  return WiFi.getTime();
#elif defined(ARDUINO_SAMD_MKRNB1500)
  return 0;
#endif
}

void MqttClass::setup() {
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1)
      ;
  }
  ArduinoBearSSL.onGetTime(getTime);
  log("id: " + Config.getId());
  log("cert: " + Config.getMqttBrokerCert());
  log("url: " + Config.getMqttBrokerUrl());
  sslClient.setEccSlot(0, Config.getMqttBrokerCert().c_str());
  mqttClient.setId(Config.getId());
  mqttClient.onMessage(onMessageReceived);
}

void MqttClass::connect() {
  log("Attempting to MQTT broker: ");
  while (!mqttClient.connect(Config.getMqttBrokerUrl().c_str(), 8883)) {
    log(".");
    delay(5000);
  }
  log("You're connected to the MQTT broker");
  mqttClient.subscribe("arduino/incoming");
}

bool MqttClass::isConnected() {
  return mqttClient.connected();
}

void MqttClass::poll() {
  mqttClient.poll();
}

void MqttClass::publish(String topic, String message) {
  log("publish " + topic + " " + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
