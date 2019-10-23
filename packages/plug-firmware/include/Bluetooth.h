#ifndef Bluetooth_h
#define Bluetooth_h

#include <STBLE.h>

#define CHALLENGE_LENGTH 16
#define HMAC_LENGTH 16

#define STATUS_IGNITION 0     // 0: unknown, 1: off,    2: on
#define STATUS_LOCK_CMD 1     // 0: unknown, 1: locked, 2: unlocked
#define STATUS_IMMOBILIZER 2  // 0: unknown, 1: locked, 2: unlocked
#define STATUS_CAN_DOORS 3    // 0: closed,  1: open;     bit 0: left front, 1: right front, 2: left back, 3: right back
#define STATUS_CAN_WINDOWS 4  // 0: closed,  1: open;     bit 0: left front, 1: right front, 2: left back, 3: right back
#define STATUS_CAN_LOCKS 7    // 0: locked,  1: unlocked; bit 0: left front, 1: right front, 2: left back, 3: right back

class BluetoothClass {
 public:
  int begin();
  void poll();
  void reset();
  int getHealth();
  void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t* att_data);
  void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
  void GAP_DisconnectionComplete_CB();
  void Read_Request_CB(uint16_t handle);
  void setSystemStatus(int which, uint8_t value);

 private:
  tBleStatus addService();
  tBleStatus setChallenge();
  tBleStatus setConnectable();
  void setDefaultSystemStatus();
  void updateSystemStatus();
  boolean checkCanBus(unsigned int i, const char* name, int what, u_int8_t mask);

  tBleStatus status = BLE_STATUS_ERROR;
  char name[21];
  uint16_t connection_handle = 0;
  uint16_t ServHandle, AuthCharHandle, CmdCharHandle, ChallengeCharHandle, StatusCharHandle;
  String message = "";
  uint16_t continueLength = 0;
  uint8_t challenge[CHALLENGE_LENGTH];
  uint8_t hmac[HMAC_LENGTH];
  // similar to https://api.cloudboxx.invers.com/api/doc/Bluetooth-Smart-%28BLE%29-API.html STATUS_1
  uint8_t systemStatus[8];  //0: Ignition, 1: Central Lock, 2: Immobilzer, 3: Doors, 4: Windows, 7: Central Lock (Can)
};

extern BluetoothClass Bluetooth;

#endif  // Bluetooth_h