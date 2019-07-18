#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

#include "Config.h"
#include "Gps.h"
#include "Http.h"
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
  System.processCommand(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

void MqttClass::setup() {
  if (!ECCX08.begin()) {
    logError(F("No ECCX08 present"));
    while (1)
      ;
  }
  ArduinoBearSSL.onGetTime(getTime);
  JsonObject mqtt = Config.get()["mqtt"];
  const char* id = mqtt["id"];
  logDebug("id: " + String(id));
  const char* cert = mqtt["cert"];
  logDebug("cert: " + String(cert));
  sslClient.setEccSlot(0, cert);
  mqttClient.setId(id);
  mqttClient.onMessage(onMessageReceived);
  connect();
}

void MqttClass::connect() {
  if (!Internet.isConnected()) {
    Internet.connect();
  }
  const JsonObject mqtt = Config.get()["mqtt"];
  const char* url = mqtt["url"];
  String id = mqtt["id"];
  logDebug("Attempting to connect to MQTT broker: " + String(url));
  const int maxTry = 20;
  int i = 0;
  while (!mqttClient.connect(url, 8883)) {
    Watchdog.reset();
    if (i == maxTry) {
      logDebug(F("Failed to connect, try leter"));
      return;
    }
    log(F("M"));
    delay(3000);
    i++;
  }
  logDebug(F("You're connected to the MQTT broker"));
  mqttClient.subscribe("$aws/things/" + id + "/shadow/update/delta");
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
  String id = Config.get()["mqtt"]["id"];
  String topic = "$aws/things/" + id + "/shadow/update";
  String message = "{\"state\": {" +
                   (reported != "" ? "\"reported\": " + reported : "") +
                   (reported != "" && desired != "" ? ", " : "") +
                   (desired != "" ? "\"desired\": " + desired : "") + "}}";
  Serial.println(Gps.getTime() + String(" ") + message);
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
