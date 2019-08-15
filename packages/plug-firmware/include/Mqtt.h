#ifndef Mqtt_h
#define Mqtt_h

class MqttClass {
 public:
  void setup();
  void connect();
  bool isConnected();
  void poll();
  void send(const String& message);

 private:
  String topic;
};

extern MqttClass Mqtt;

#endif  // Mqtt_h