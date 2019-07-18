#include <STBLE.h>

#include "Bluetooth.h"
#include "Config.h"
#include "Logger.h"

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0

void setConnectable(void);
uint8_t Write_UART_TX(char *TXdata, uint8_t datasize);

volatile uint8_t set_connectable = 1;
uint16_t connection_handle = 0;

#define ADV_INTERVAL_MIN_MS 50
#define ADV_INTERVAL_MAX_MS 100

int connected = FALSE;

void aci_loop() {
  HCI_Process();
  ble_connection_state = connected;
  if (set_connectable) {
    setConnectable();
    set_connectable = 0;
  }
  if (HCI_Queue_Empty()) {
    //Enter_LP_Sleep_Mode();
  }
}

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {                                                                                                                                                                   \
    uuid_struct[0] = uuid_0;                                                                                                                                             \
    uuid_struct[1] = uuid_1;                                                                                                                                             \
    uuid_struct[2] = uuid_2;                                                                                                                                             \
    uuid_struct[3] = uuid_3;                                                                                                                                             \
    uuid_struct[4] = uuid_4;                                                                                                                                             \
    uuid_struct[5] = uuid_5;                                                                                                                                             \
    uuid_struct[6] = uuid_6;                                                                                                                                             \
    uuid_struct[7] = uuid_7;                                                                                                                                             \
    uuid_struct[8] = uuid_8;                                                                                                                                             \
    uuid_struct[9] = uuid_9;                                                                                                                                             \
    uuid_struct[10] = uuid_10;                                                                                                                                           \
    uuid_struct[11] = uuid_11;                                                                                                                                           \
    uuid_struct[12] = uuid_12;                                                                                                                                           \
    uuid_struct[13] = uuid_13;                                                                                                                                           \
    uuid_struct[14] = uuid_14;                                                                                                                                           \
    uuid_struct[15] = uuid_15;                                                                                                                                           \
  } while (0)

#define COPY_UART_SERVICE_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_TX_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_RX_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)

uint16_t UARTServHandle, UARTTXCharHandle, UARTRXCharHandle;

uint8_t Add_UART_Service(void) {
  tBleStatus ret;
  uint8_t uuid[16];

  COPY_UART_SERVICE_UUID(uuid);
  ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7, &UARTServHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  COPY_UART_TX_CHAR_UUID(uuid);
  ret = aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                          16, 1, &UARTTXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  COPY_UART_RX_CHAR_UUID(uuid);
  ret = aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                          16, 1, &UARTRXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  return BLE_STATUS_SUCCESS;

fail:
  logError("Error while adding UART service.");
  return BLE_STATUS_ERROR;
}

uint8_t lib_aci_send_data(uint8_t ignore, uint8_t *sendBuffer, uint8_t sendLength) {
  return !Write_UART_TX((char *)sendBuffer, sendLength);
}

uint8_t Write_UART_TX(char *TXdata, uint8_t datasize) {
  tBleStatus ret;

  ret = aci_gatt_update_char_value(UARTServHandle, UARTRXCharHandle, 0, datasize, (uint8_t *)TXdata);

  if (ret != BLE_STATUS_SUCCESS) {
    logError("Error while updating UART characteristic.");
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;
}

void Read_Request_CB(uint16_t handle) {
  /*if(handle == UARTTXCharHandle + 1)
    {

    }
    else if(handle == UARTRXCharHandle + 1)
    {


    }*/

  if (connection_handle != 0)
    aci_gatt_allow_read(connection_handle);
}

void setConnectable(void) {
  tBleStatus ret;

  hci_le_set_scan_resp_data(0, NULL);
  logDebug("BLE General Discoverable Mode.");

  ret = aci_gap_set_discoverable(ADV_IND,
                                 (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                 STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                 0, NULL, 0, NULL, 0, 0);

  if (ret != BLE_STATUS_SUCCESS)
    logDebug(ret);
}

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data) {
  if (handle == UARTTXCharHandle + 1) {
    int i;
    for (i = 0; i < data_length; i++) {
      ble_rx_buffer[i] = att_data[i];
    }
    ble_rx_buffer[i] = '\0';
    ble_rx_buffer_len = data_length;
  }
}

void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle) {
  connected = TRUE;
  connection_handle = handle;

  char sprintbuff[64];
  snprintf(sprintbuff, 64, "BLE Connected to device: %02X-%02X-%02X-%02X-%02X-%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
  logInfo(sprintbuff);
}

void GAP_DisconnectionComplete_CB(void) {
  connected = FALSE;
  logInfo("BLE Disconnected");
  /* Make the device connectable again. */
  set_connectable = TRUE;
}

void HCI_Event_CB(void *pckt) {
  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;

  if (hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch (event_pckt->evt) {
    case EVT_DISCONN_COMPLETE: {
      //evt_disconn_complete *evt = (void *)event_pckt->data;
      GAP_DisconnectionComplete_CB();
    } break;

    case EVT_LE_META_EVENT: {
      evt_le_meta_event *evt = (evt_le_meta_event *)event_pckt->data;

      switch (evt->subevent) {
        case EVT_LE_CONN_COMPLETE: {
          evt_le_connection_complete *cc = (evt_le_connection_complete *)evt->data;
          GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        } break;
      }
    } break;

    case EVT_VENDOR: {
      evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;
      switch (blue_evt->ecode) {
        case EVT_BLUE_GATT_READ_PERMIT_REQ: {
          evt_gatt_read_permit_req *pr = (evt_gatt_read_permit_req *)blue_evt->data;
          Read_Request_CB(pr->attr_handle);
        } break;

        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED: {
          evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1 *)blue_evt->data;
          Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
        } break;
      }
    } break;
  }
}

void BluetoothClass::setup() {
  int ret;
  //  Serial.println("HCI_init");
  HCI_Init();
  /* Init SPI interface */
  //  Serial.println("SPI_init");
  BNRG_SPI_Init();
  /* Reset BlueNRG/BlueNRG-MS SPI interface */
  //  Serial.println("BNRG RST");
  BlueNRG_RST();

  String uid = Config.get()["id"];
  uint8_t *bdaddr = (uint8_t *)uid.substring(uid.length() - CONFIG_DATA_PUBADDR_LEN).c_str();
  //  Serial.println("Set BR_ADDR");
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);

  if (ret) {
    logError("Setting BD_ADDR failed.");
  }
  //  Serial.println("ACI GATT INIT");
  ret = aci_gatt_init();

  if (ret) {
    logError("GATT_Init failed.");
  }

  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  //  Serial.println("GAP Init");
  ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  if (ret) {
    logError("GAP_Init failed.");
  }

  const char *name = Config.get()["id"];
  //  Serial.println("update char value");
  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *)name);

  if (ret) {
    logError("aci_gatt_update_char_value failed.");
  } else {
    logInfo("BLE Stack Initialized.");
  }

  //  Serial.println("add uart");
  ret = Add_UART_Service();

  if (ret == BLE_STATUS_SUCCESS) {
    logInfo("UART service added successfully.");
  } else {
    logError("Error while adding UART service.");
  }

  /* +8 dBm output power */
  ret = aci_hal_set_tx_power_level(1, 7);
}

void BluetoothClass::poll() {
  aci_loop();               //must run frequently
  if (ble_rx_buffer_len) {  //Check if data is available
    Serial.print(ble_rx_buffer_len);
    Serial.print(" : ");
    Serial.println((char *)ble_rx_buffer);
    Serial.println("you just got some BLE data, pushing as a command");
    //    String buf_str = String(*ble_rx_buffer);
    // command_handler((char *)ble_rx_buffer);
    ble_rx_buffer_len = 0;  //clear afer reading
  }
  //forward serial port input to BLE output
  if (Serial.available()) {  //Check if serial input is available to send
    delay(10);               //should catch input
    uint8_t sendBuffer[21];
    uint8_t sendLength = 0;
    while (Serial.available() && sendLength < 19) {
      sendBuffer[sendLength] = Serial.read();
      sendLength++;
    }
    if (Serial.available()) {
      Serial.print(F("Input truncated, dropped: "));
      if (Serial.available()) {
        Serial.write(Serial.read());
      }
    }
    sendBuffer[sendLength] = '\0';  //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t *)sendBuffer, sendLength)) {
      Serial.println(F("TX dropped!"));
    } else {
      Serial.println(F("Forwarded serial input to BLE"));
    }
  }
}

BluetoothClass Bluetooth;