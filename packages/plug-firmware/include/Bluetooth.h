#ifndef Bluetooth_h
#define Bluetooth_h

class BluetoothClass {
 public:
  void setup();
  void poll();
  void readToken();
  String& getName();

 private:
  String name;
};

extern BluetoothClass Bluetooth;

#endif  // Bluetooth_h