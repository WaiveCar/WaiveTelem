#ifndef Mqtt_h
#define Mqtt_h

class MqttClass {
 public:
  void setup();
  void connect();
  bool isConnected();
  void poll();
  void telemeter(const String& json, bool resetDesired = false);
};

extern MqttClass Mqtt;

#endif  // Mqtt_h