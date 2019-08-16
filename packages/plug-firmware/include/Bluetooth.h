#ifndef Bluetooth_h
#define Bluetooth_h

#define CHALLENGE_LENGTH 16
#define HMAC_LENGTH 16

class BluetoothClass {
 public:
  void setup();
  uint8_t addService();
  void poll();
  void reset();
  uint8_t setChallenge();
  void setConnectable();
  void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t* att_data);
  void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
  void GAP_DisconnectionComplete_CB();
  void Read_Request_CB(uint16_t handle);

 private:
  char name[21];
  uint16_t connection_handle = 0;
  uint16_t ServHandle, AuthCharHandle, CmdCharHandle, ChallengeCharHandle;
  String message = "";
  uint16_t continueLength = 0;
  uint8_t challenge[CHALLENGE_LENGTH];
  uint8_t hmac[HMAC_LENGTH];
};

extern BluetoothClass Bluetooth;

#endif  // Bluetooth_h