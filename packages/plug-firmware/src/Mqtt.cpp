#include <Adafruit_SleepyDog.h>
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>

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
  System.processCommand(payload);
}

static unsigned long getTime() {
  return Internet.getTime();
}

void MqttClass::setup() {
  while (!ECCX08.begin()) {
    logError(F("No ECCX08 present"));
    delay(5000);
  }
  ArduinoBearSSL.onGetTime(getTime);
  JsonObject mqtt = Config.get()["mqtt"];
  const char* id = Config.get()["id"];
  const char* cert = mqtt["cert"];
  logDebug("cert: " + String(cert));
  sslClient.setEccSlot(0, cert);
  mqttClient.setId(id);
  // set a long keep-alive interval as mqttClient won't keep alive for us
  // since millis() is not accurate as the device is deep sleeping most of the time
  mqttClient.setKeepAliveInterval((Config.get()["heartbeat"]["notInRide"].as<int>() + 30) * 1000);
  mqttClient.onMessage(onMessageReceived);
  connect();
}

void MqttClass::connect() {
  if (!Internet.isConnected()) {
    Internet.connect();
    // logDebug(String(Internet.getTime()));
    // test internect connection
    // Https.download("www.pivotaltracker.com", "/", "TEMP");
    // Https.download("community.libra.org", "/", "TEMP");
    // Https.download("news.ycombinator.com", "/", "TEMP");
    // Https.download("www.wikipedia.org", "/", "TEMP");
    // Https.download("waiveplug.s3.us-east-2.amazonaws.com", "config_waive-1_dd22d948fbd671c5751640a11dec139da46c5997bb3f20d0b6ad5bd61ac7e0cc", "TEMP");  // connect sometimes works with DigiCertBaltimoreCA_2G2, but WR failed
    // Https.download("storage.googleapis.com", "www.swiperweb.com/privacy.html", "TEMP");  //works
    // Https.download("workflowy.com", "/", "TEMP");   // very long timeout
    // Https.download("trello.com", "/", "TEMP");      // very long timeout
    // Https.download("www.apple.com", "/", "TEMP");   // very long timeout
    // Https.download("www.amazon.com", "/", "TEMP");  // very long timeout
    // Https.download("discordapp.com", "/", "TEMP");  //works, cloudflare, doesn't need any RootCerts
    // Https.download("www.producthunt.com", "/", "TEMP");  //works
    // Https.download("gmail.com", "/", "TEMP");            // works
    // Https.download("www.google.com", "/", "TEMP");       //works
    // Https.download("www.bing.com", "/", "TEMP");         //works
    // Https.download("reelgood.com", "/", "TEMP");         // weird binary stuff, but seems to work
  }
  const JsonObject mqtt = Config.get()["mqtt"];
  const char* url = mqtt["url"];
  String id = Config.get()["id"];
  logDebug("id: " + id);
  logDebug("Attempting to connect to MQTT broker: " + String(url));
  const int maxTry = 20;
  int i = 0;
  while (!mqttClient.connect(url, 8883)) {
    Watchdog.reset();
    if (i == maxTry) {
      logDebug(F("Failed to connect, try leter"));
      return;
    }
    logDebug("MQTT connect error: " + String(mqttClient.connectError()));
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

void MqttClass::send(const String& message) {
  String id = Config.get()["id"];
  String topic = "$aws/things/" + id + "/shadow/update";
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
}

MqttClass Mqtt;
