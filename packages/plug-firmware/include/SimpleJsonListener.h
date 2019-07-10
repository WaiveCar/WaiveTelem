#ifndef SimpleJsonListener_h
#define SimpleJsonListener_h

#include <Arduino.h>
#include <JsonListener.h>

#include "Console.h"

class SimpleJsonListener : public JsonListener {
 public:
  virtual void key(String k) {
    currentKey = k;
  }

  virtual void startDocument() {
    currentObject = "";
    currentKey = "";
    arrayIndex = -1;
  }

  virtual void startObject() {
    currentObject = currentObject + currentKey + "/";
    // log(currentObject);
  }

  virtual void endObject() {
    int secondLastIndex = -1;
    int lastIndex = 0;
    while (lastIndex != (int)currentObject.length() - 1) {
      secondLastIndex = lastIndex;
      lastIndex = currentObject.indexOf('/', lastIndex + 1);
    }
    currentObject = currentObject.substring(0, secondLastIndex + 1);
    // log(currentObject);
    if (secondLastIndex == lastIndex - 1) {  // in array
      arrayIndex++;
    }
    currentKey = "";
  }

  virtual void startArray() {
    startObject();
    currentKey = "";
    arrayIndex = 0;
  }

  virtual void endArray() {
    endObject();
    arrayIndex = -1;
  }

  virtual void endDocument() {
  }

  virtual void whitespace(char c) {
  }

 protected:
  String currentObject;
  String currentKey;
  int arrayIndex;
};

#endif  // SimpleJsonListener_h
