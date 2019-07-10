#ifndef MqttJsonListener_h
#define MqttJsonListener_h

#include "Console.h"
#include "SimpleJsonListener.h"

class MqttJsonListener : public SimpleJsonListener {
 public:
  virtual void value(String v) {
    // log("v: " + v);
    if (currentObject == "/state/") {
      command = currentKey + "_" + v;
    }
  }

  String& getCommand() {
    return command;
  }

 protected:
  String command;
};

#endif  // MqttJsonListener_h