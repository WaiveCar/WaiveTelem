#ifndef Mqtt_h
#define Mqtt_h

class MqttClass {
 public:
  int begin();
  void connect();
  bool isConnected();
  void poll();
  void updateShadow(const String& message);
  void logMsg(const String& message);
  void send(const String& message);

 private:
  String updateTopic;
  String logTopic;
};

extern MqttClass Mqtt;

#endif  // Mqtt_h