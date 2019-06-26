
// SPI reserved: 8 MOSI, 9 SCK, 10 MISO
#define RELAY_2_PIN 2
#define CAN0_CS_PIN  3
#define CAN0_INT_PIN 7
#define SD_CS_PIN   4
#define FL_CS_PIN   5
#define DOOR_LOCK_PIN A4
#define DOOR_UNLOCK_PIN A5
#define BLE_CS_PIN A1
#define BLE_INT_PIN A2
#define BLE_RST_PIN A3
//Open Digitals  11, 12 - i2c reserved for crypto
#define BLE_MOSI_PIN  0
#define BLE_SCK_PIN   1
#define BLE_MISO_PIN  6
#define CAN1_CS_PIN   A0
#define CAN1_INT_PIN  A6

void set_pin_modes(){
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(CAN0_CS_PIN, OUTPUT);
  pinMode(CAN0_INT_PIN, INPUT);
  pinMode(CAN1_CS_PIN, OUTPUT);
  pinMode(CAN1_INT_PIN, INPUT);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(FL_CS_PIN, OUTPUT);
  pinMode(DOOR_UNLOCK_PIN, OUTPUT);
  pinMode(DOOR_LOCK_PIN, OUTPUT);
  pinMode(BLE_CS_PIN, OUTPUT);
  pinMode(BLE_INT_PIN, INPUT);
  pinMode(BLE_RST_PIN, OUTPUT);
  pinPeripheral(BLE_MOSI_PIN, PIO_SERCOM); //mosi
  pinPeripheral(BLE_SCK_PIN, PIO_SERCOM); //sck
  pinPeripheral(BLE_MISO_PIN, PIO_SERCOM_ALT); //miso

  //Ensure SPI CS Pins are all high to avoid a noisy bus at inits
  digitalWrite(CAN0_CS_PIN, HIGH);
//  digitalWrite(CAN1_CS_PIN, HIGH);
  digitalWrite(SD_CS_PIN, HIGH);
  digitalWrite(FL_CS_PIN, HIGH);
  digitalWrite(BLE_CS_PIN, HIGH);
  digitalWrite(BLE_RST_PIN, HIGH);
}
