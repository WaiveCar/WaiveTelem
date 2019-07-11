#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoJson.h>
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
#include "Status.h"

#include "Pins.h"

static void onMessageReceived(int messageSize) {
  log("Received a message with topic '" + mqttClient.messageTopic() + "', length " + messageSize + " bytes:");
  String payload = mqttClient.readString();
  log("payload: " + payload);
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Failed to read file: " + String(error.c_str()));
    return;
  }

  JsonObject stateDoc = doc["state"];
  String doors = stateDoc["doors"] | "";
  String vehicle = stateDoc["vehicle"] | "";
  String inRide = stateDoc["inRide"] | "";
  if (doors == "unlocked") {
    Pins.unlockDoors();
    // Mqtt.telemeter("{\"doors\": \"unlocked\"}");
  } else if (doors == "locked") {
    Pins.lockDoors();
    // Mqtt.telemeter("{\"doors\": \"locked\"}");
  } else if (vehicle == "immobilized") {
    Pins.immobilize();
    Mqtt.telemeter("{\"vehicle\": \"immobilized\"}");
  } else if (vehicle == "unimmobilized") {
    Pins.unimmobilize();
    Mqtt.telemeter("{\"vehicle\": \"unimmobilized\"}");
  } else if (inRide == "true") {
    Status.setInRide(true);
    Mqtt.telemeter("{\"inRide\": \"true\"}");
  } else if (inRide == "false") {
    Status.setInRide(false);
    Mqtt.telemeter("{\"inRide\": \"false\"}");
  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(payload);
  }
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
    i++;
    if (i == maxTry) {
      log(F("Failed to connect"));
      return;
    }
    Serial.print(F("M"));
    delay(3000);
    Watchdog.reset();
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

void MqttClass::telemeter(const String& json, bool resetDesired) {
  String topic = "$aws/things/" + String(Config.getId()) + "/shadow/update";
  String message = "{\"state\": {\"reported\": " + json + (resetDesired ? ", \"desired\": null" : "") + "}}";
  log("publish " + topic + " " + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
