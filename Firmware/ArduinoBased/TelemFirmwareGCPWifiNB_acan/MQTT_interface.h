//todo: complete transition to MqttClient.h from ArduinoMqttClient.h
// https://github.com/monstrenyatko/ArduinoMqtt/blob/master/examples/ConnectEthernetClient/ConnectEthernetClient.ino
// https://github.com/monstrenyatko/ArduinoMqtt/blob/master/examples/ConnectEsp8266WiFiClient/ConnectEsp8266WiFiClient.ino

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#elif defined(ARDUINO_SAMD_MKRNB1500)
#include <MKRNB.h>
#endif

#include <ArduinoECCX08.h>
#include <utility/ECCX08JWS.h>
#include <ArduinoMqttClient.h>
//#include <MqttClient.h>
#include "arduino_secrets.h"

#ifdef ARDUINO_SAMD_MKR1000
//Wifi client setup
char ssid[] = SECRET_SSID;    // network SSID name
char pass[] = SECRET_PASS;    // network password
int wifi_status = WL_IDLE_STATUS;
#elif defined(ARDUINO_SAMD_MKRNB1500)
const char pinnumber[]     = SECRET_PINNUMBER;
NB nbAccess; 
GPRS gprs;
#endif

String mqtt_topic_str;
String mqtt_payload_str;
char mqtt_topic[50];
char mqtt_payload[50];

int keyIndex = 0;

// Initialize wifi/nb client library

#ifdef ARDUINO_SAMD_MKR1000
WiFiSSLClient sslClient;
#elif defined(ARDUINO_SAMD_MKRNB1500)
NBSSLClient sslClient;
#endif
MqttClient    mqttClient(sslClient);


//Google Cloud setup
const char projectId[]   = SECRET_PROJECT_ID;
const char cloudRegion[] = SECRET_CLOUD_REGION;
const char registryId[]  = SECRET_REGISTRY_ID;
const String deviceId    = SECRET_DEVICE_ID;

const char broker[]      = "mqtt.googleapis.com";

unsigned long getTime() {
#ifdef ARDUINO_SAMD_MKR1000
  // get the current time from the WiFi module
  return WiFi.getTime();
#elif defined(ARDUINO_SAMD_MKRNB1500)
  // or get the current time from the NB module
  return nbAccess.getTime();
#else
  return 0;
#endif 
}



String calculateJWT() {
  unsigned long now = getTime();
  
  // calculate the JWT, based on:
  //   https://cloud.google.com/iot/docs/how-tos/credentials/jwts
  DynamicJsonDocument jwtHeader(JSON_OBJECT_SIZE(2)), jwtClaim(JSON_OBJECT_SIZE(3));

  jwtHeader["alg"] = "ES256";
  jwtHeader["typ"] = "JWT";

  jwtClaim["aud"] = projectId;
  jwtClaim["iat"] = now;
  jwtClaim["exp"] = now + (24L * 60L * 60L); // expires in 24 hours 
  String jwtHeaderS, jwtClaimS;
  serializeJson(jwtHeader, jwtHeaderS);
  serializeJson(jwtClaim, jwtClaimS);
  
  return ECCX08JWS.sign(1, jwtHeaderS, jwtClaimS);  // pull in JWT certificate from ECCx08 slot 1
}

void publishMessage(char topic[], char *message_buf) {
  Serial.println("Publishing message");
  Serial.print(topic);
  Serial.println(message_buf);
  int result;
  // send message, the Print interface can be used to set the message contents
  result = mqttClient.beginMessage("/devices/" + deviceId + topic);//, sizeof(message_buf), true);//, 1u);
//  Serial.print("begin message result: ");
//  Serial.println(result);
  mqttClient.print((char*)message_buf);
  result = mqttClient.endMessage();
//  Serial.print("end message result: ");
//  Serial.println(result);
  
//  MqttClient::Message message;
//  message.qos = MqttClient::QOS1;
//  message.retained = true;
//  message.dup = false;
//  message.payload = (void*) message_buf;
//  message.payloadLen = strlen(message_buf);
//  mqttClient.publish("/devices/" + deviceId + topic, message);
 
}



void connectMQTT() {
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");
  int i;
  while (!mqttClient.connected()) {
    // Calculate the JWT and assign it as the password
    String jwt = calculateJWT();

    mqttClient.setUsernamePassword("", jwt);

    if (!mqttClient.connect(broker, 8883)) {
      // failed, retry
      Serial.print(".");
      i = mqttClient.connectError();
      Serial.println(i);
      delay(5000);
    }
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to topics
  mqttClient.subscribe("/devices/" + deviceId + "/config", 1);
  mqttClient.subscribe("/devices/" + deviceId + "/commands/#");
  mqttClient.subscribe("/devices/" + deviceId + "/state/#");
}

String calculateClientId() {
  String clientId;

  // Format:
  //
  //   projects/{project-id}/locations/{cloud-region}/registries/{registry-id}/devices/{device-id}
  //

  clientId += "projects/";
  clientId += projectId;
  clientId += "/locations/";
  clientId += cloudRegion;
  clientId += "/registries/";
  clientId += registryId;
  clientId += "/devices/";
  clientId += deviceId;

  return clientId;
}

#ifdef ARDUINO_SAMD_MKRNB1500
void connectNB() {
  Serial.println("Attempting to connect to the cellular network");

  while ((nbAccess.begin(pinnumber) != NB_READY) ||
         (gprs.attachGPRS("hologram") != GPRS_READY)) {
    // failed, retry
    Serial.print(".");
    delay(1000);
  }

  Serial.println("You're connected to the cellular network");
  Serial.println();
}
#endif

#ifdef ARDUINO_SAMD_MKR1000
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectWiFi(){
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while(true);
  } 
  while (wifi_status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    //Connect to WPA/WPA2 network:
    wifi_status = WiFi.begin(ssid, pass);

    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
}
#endif
