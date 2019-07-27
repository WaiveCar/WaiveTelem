#include <STBLE.h>

#include "Bluetooth.h"
#include "Config.h"
#include "Logger.h"
#include "System.h"

#define ADV_INTERVAL_MIN_MS 50
#define ADV_INTERVAL_MAX_MS 100

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

#define COPY_WAIVE_SERVICE_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_WAIVE_CMD_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)

uint16_t WaiveServHandle, WaiveCmdCharHandle;

uint8_t AddWaiveService(void) {
  tBleStatus ret;
  uint8_t uuid[16];

  COPY_WAIVE_SERVICE_UUID(uuid);
  ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7, &WaiveServHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  COPY_WAIVE_CMD_CHAR_UUID(uuid);
  ret = aci_gatt_add_char(WaiveServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                          16, 1, &WaiveCmdCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  return BLE_STATUS_SUCCESS;

fail:
  logError(F("Error while adding Waive service."));
  return BLE_STATUS_ERROR;
}

void setConnectable(void) {
  tBleStatus ret;

  hci_le_set_scan_resp_data(0, NULL);

  ret = aci_gap_set_discoverable(ADV_IND,
                                 (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                 STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                 0, NULL, 0, NULL, 0, 0);

  if (ret != BLE_STATUS_SUCCESS) {
    logDebug((String)ret);
  }
}

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data) {
  if (handle == WaiveCmdCharHandle + 1) {
    uint8_t bleCmdBuffer[21];
    int i;
    for (i = 0; i < data_length; i++) {
      bleCmdBuffer[i] = att_data[i];
    }
    bleCmdBuffer[i] = '\0';
    String command = String((char *)bleCmdBuffer);
    logDebug(command);
    System.processCommand(command);
  }
}

void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle) {
  char sprintbuff[64];
  snprintf(sprintbuff, 64, "BLE Connected to device: %02X-%02X-%02X-%02X-%02X-%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
  logInfo(sprintbuff);
}

void GAP_DisconnectionComplete_CB(void) {
  logInfo(F("BLE Disconnected"));
  setConnectable();
}

// called by HCI_Process() in BluetoothClass::poll()
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
  HCI_Init();
  BNRG_SPI_Init();
  BlueNRG_RST();

  String uid = Config.get()["id"];
  uint8_t *bdaddr = (uint8_t *)uid.substring(uid.length() - CONFIG_DATA_PUBADDR_LEN).c_str();
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);

  if (ret) {
    logError(F("BLE Setting BD_ADDR failed."));
  }
  ret = aci_gatt_init();

  if (ret) {
    logError(F("BLE GATT_Init failed."));
  }

  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  //  Serial.println("GAP Init");
  ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  if (ret) {
    logError(F("BLE GAP_Init failed."));
  }

  const char *name = Config.get()["id"];
  //  Serial.println("update char value");
  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *)name);

  if (ret) {
    logError(F("BLE aci_gatt_update_char_value failed."));
  }

  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     71764,
                                     BONDING);
  if (ret == BLE_STATUS_SUCCESS) {
    logDebug(F("BLE Stack Initialized."));
  }

  ret = AddWaiveService();

  if (ret == BLE_STATUS_SUCCESS) {
    logDebug(F("BLE Service added successfully."));
  } else {
    logError(F("BLE Error while adding service."));
  }

  /* +8 dBm output power */
  ret = aci_hal_set_tx_power_level(1, 7);

  setConnectable();
}

void BluetoothClass::poll() {
  HCI_Process();
}

BluetoothClass Bluetooth;