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

#define COPY_SERVICE_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
#define COPY_AUTH_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
#define COPY_CMD_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

uint16_t ServHandle, AuthCharHandle, CmdCharHandle;

uint8_t AddService(void) {
  tBleStatus ret;
  uint8_t uuid[16];

  COPY_SERVICE_UUID(uuid);
  ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7, &ServHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  COPY_AUTH_CHAR_UUID(uuid);
  // try ATTR_PERMISSION_AUTHEN_WRITE doesn't work
  ret = aci_gatt_add_char(ServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                          16, 1, &AuthCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  COPY_CMD_CHAR_UUID(uuid);
  ret = aci_gatt_add_char(ServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                          16, 1, &CmdCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  return BLE_STATUS_SUCCESS;

fail:
  logError(F("Error while adding  service."));
  return BLE_STATUS_ERROR;
}

void setConnectable(void) {
  hci_le_set_scan_resp_data(0, NULL);

  String localName = String((char)AD_TYPE_COMPLETE_LOCAL_NAME) + Bluetooth.getName();
  tBleStatus ret = aci_gap_set_discoverable(ADV_IND,
                                            (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                            STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                            localName.length(), localName.c_str(), 0, NULL, 0, 0);
  if (ret != BLE_STATUS_SUCCESS) {
    logDebug((String)ret);
  }
}

String message = "";
uint8_t continueLength = 0;
// BLE actually only supports sending 20 bytes of data per characteristic. We need a little "protocol" to support longer array.
// You split the array in 20 bytes arrays where the first byte contain the total length of data to come, including the data in this array.
// Example:
// You start with an array like this:
// [72, 101, 108, 108, 111, 32, 109, 121, 32, 108, 111, 118, 101, 108, 121, 32, 119, 111, 114, 108, 100, 44, 32, 105, 32, 108, 111, 118, 101, 32, 121, 111, 117, 32, 97, 110, 100, 32, 97, 108, 108, 32, 97, 114, 111, 117, 110, 100, 32, 121, 111, 117]

// Then you generate the following 3 arrays:
// [52, 72, 101, 108, 108, 111, 32, 109, 121, 32, 108, 111, 118, 101, 108, 121, 32, 119, 111, 114]
// [33, 108, 100, 44, 32, 105, 32, 108, 111, 118, 101, 32, 121, 111, 117, 32, 97, 110, 100, 32]
// [14, 97, 108, 108, 32, 97, 114, 111, 117, 110, 100, 32, 121, 111, 117]
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data) {
  if (handle == AuthCharHandle + 1 || handle == CmdCharHandle + 1) {
    uint8_t bleCmdBuffer[20];
    uint8_t messageLength = att_data[0];
    int i = 0;
    for (; i < data_length - 1; i++) {
      bleCmdBuffer[i] = att_data[i + 1];
    }
    bleCmdBuffer[i] = '\0';
    // logDebug(String(messageLength));
    if (messageLength != continueLength) {
      message = "";
    }
    continueLength = messageLength - data_length + 1;
    message += String((char *)bleCmdBuffer);
    // logDebug(String(continueLength));
    // logDebug(message);
    if (continueLength == 0) {
      if (handle == AuthCharHandle + 1) {
        System.authorizeCommand(message);
      } else if (handle == CmdCharHandle + 1) {
        System.processCommand(message, true);
      }
      message = "";
    }
  }
}

void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle) {
  char sprintbuff[64];
  snprintf(sprintbuff, 64, "BLE Connected to device: %02X-%02X-%02X-%02X-%02X-%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
  logInfo(sprintbuff);
  System.unauthorize();
}

void GAP_DisconnectionComplete_CB(void) {
  logInfo(F("BLE Disconnected"));
  setConnectable();
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

  const char *id = Config.get()["id"];
  name = String("W-") + id;
  logDebug("Bluetooth name: " + name);

  ret = aci_gatt_init();
  if (ret) {
    logError(F("BLE GATT_Init failed."));
  }

  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, name.length(), &service_handle, &dev_name_char_handle, &appearance_char_handle);
  if (ret) {
    logError(F("BLE GAP_Init failed."));
  }

  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, name.length(), name.c_str());
  if (ret) {
    logError(F("BLE aci_gatt_update_char_value failed."));
  }

  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     DONOT_USE_FIXED_PIN_FOR_PAIRING,
                                     0,
                                     BONDING);
  if (ret == BLE_STATUS_SUCCESS) {
    logDebug(F("BLE Stack Initialized."));
  }

  ret = AddService();
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

String &BluetoothClass::getName() {
  return name;
}

BluetoothClass Bluetooth;
