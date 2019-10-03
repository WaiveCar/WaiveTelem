#ifndef Mqtt_h
#define Mqtt_h

class MqttClass {
 public:
  int begin();
  void connect();
  bool isConnected();
  void poll();
  void updateShadow(const char* message, int len);
  void logMsg(const char* message, int len);

 private:
  String updateTopic;
  String logTopic;
  int32_t lastConnectTry = -1;
  int32_t lastPollTime = -1;
};

extern MqttClass Mqtt;

#endif  // Mqtt_h