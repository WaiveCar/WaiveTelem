#ifndef Bluetooth_h
#define Bluetooth_h

class BluetoothClass {
 public:
  void setup();
  void poll();
};

extern BluetoothClass Bluetooth;

#endif  // Bluetooth_h