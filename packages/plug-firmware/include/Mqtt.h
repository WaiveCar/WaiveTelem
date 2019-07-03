#ifndef Mqtt_h
#define Mqtt_h

class MqttClass {
 public:
  void setup();
  void connect();
  bool isConnected();
  void poll();
  void publish(String topic, String message);
};

extern MqttClass Mqtt;

#endif  // Mqtt_h