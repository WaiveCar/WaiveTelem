#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>

#include "Bluetooth.h"
#include "Command.h"
#include "Config.h"
#include "Logger.h"
#include "System.h"

void HCI_Event_CB(void *pckt) {
  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;

  if (hci_pckt->type != HCI_EVENT_PKT) {
    return;
  }

  logDebug("i_evt", event_pckt->evt);

  switch (event_pckt->evt) {
    case EVT_DISCONN_COMPLETE: {
      //evt_disconn_complete *evt = (void *)event_pckt->data;
      Bluetooth.GAP_DisconnectionComplete_CB();
    } break;

    case EVT_LE_META_EVENT: {
      evt_le_meta_event *evt = (evt_le_meta_event *)event_pckt->data;
      switch (evt->subevent) {
        case EVT_LE_CONN_COMPLETE: {
          evt_le_connection_complete *cc = (evt_le_connection_complete *)evt->data;
          Bluetooth.GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        } break;
      }
    } break;

    case EVT_VENDOR: {
      evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;
      switch (blue_evt->ecode) {
        case EVT_BLUE_GATT_READ_PERMIT_REQ: {
          evt_gatt_read_permit_req *pr = (evt_gatt_read_permit_req *)blue_evt->data;
          Bluetooth.Read_Request_CB(pr->attr_handle);
        } break;
        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED: {
          evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1 *)blue_evt->data;
          Bluetooth.Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
        } break;
      }
    } break;
  }
}

tBleStatus BluetoothClass::begin() {
  HCI_Init();
  BNRG_SPI_Init();
  reset();

  sprintf(name, "W-%s", System.getId());
  logInfo("name", name);

  status = aci_gatt_init();
  if (status) {
    logError("i_status", status, "BLE GATT_Init failed");
    return status;
  }

  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  status = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 20, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  if (status) {
    logError("i_status", status, "BLE GAP_Init failed");
    return status;
  }

  status = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, 20, name);
  if (status) {
    logError("i_status", status, "BLE aci_gatt_update_char_value failed");
    return status;
  }

  status = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                        OOB_AUTH_DATA_ABSENT,
                                        NULL,
                                        7,
                                        16,
                                        DONOT_USE_FIXED_PIN_FOR_PAIRING,
                                        0,
                                        BONDING);
  if (status) {
    logError("i_status", status, "BLE aci_gap_set_auth_requirement failed");
    return status;
  }

  addService();

  status = aci_hal_set_tx_power_level(1, 0);  // 0 is lowest, prevents eavesdropping
  if (status) {
    logError("i_status", status, "BLE aci_hal_set_tx_power_level failed");
    return status;
  }

  setConnectable();

  return status;
}

#define UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {                                                                                                                                                              \
    uuid_struct[0] = uuid_0;                                                                                                                                        \
    uuid_struct[1] = uuid_1;                                                                                                                                        \
    uuid_struct[2] = uuid_2;                                                                                                                                        \
    uuid_struct[3] = uuid_3;                                                                                                                                        \
    uuid_struct[4] = uuid_4;                                                                                                                                        \
    uuid_struct[5] = uuid_5;                                                                                                                                        \
    uuid_struct[6] = uuid_6;                                                                                                                                        \
    uuid_struct[7] = uuid_7;                                                                                                                                        \
    uuid_struct[8] = uuid_8;                                                                                                                                        \
    uuid_struct[9] = uuid_9;                                                                                                                                        \
    uuid_struct[10] = uuid_10;                                                                                                                                      \
    uuid_struct[11] = uuid_11;                                                                                                                                      \
    uuid_struct[12] = uuid_12;                                                                                                                                      \
    uuid_struct[13] = uuid_13;                                                                                                                                      \
    uuid_struct[14] = uuid_14;                                                                                                                                      \
    uuid_struct[15] = uuid_15;                                                                                                                                      \
  } while (0)

#define SERVICE_UUID(uuid_struct) UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
#define AUTH_CHAR_UUID(uuid_struct) UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
#define CMD_CHAR_UUID(uuid_struct) UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
#define CHALLENGE_CHAR_UUID(uuid_struct) UUID_128(uuid_struct, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

void BluetoothClass::addService() {
  uint8_t uuid[16];

  SERVICE_UUID(uuid);
  status = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 7, &ServHandle);
  if (status != BLE_STATUS_SUCCESS) goto fail;

  AUTH_CHAR_UUID(uuid);
  status = aci_gatt_add_char(ServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                             16, 1, &AuthCharHandle);
  if (status != BLE_STATUS_SUCCESS) goto fail;

  CMD_CHAR_UUID(uuid);
  status = aci_gatt_add_char(ServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                             16, 1, &CmdCharHandle);
  if (status != BLE_STATUS_SUCCESS) goto fail;

  CHALLENGE_CHAR_UUID(uuid);
  status = aci_gatt_add_char(ServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                             16, 1, &ChallengeCharHandle);
  if (status != BLE_STATUS_SUCCESS) goto fail;

fail:
  if (status) {
    logError("BLE Error while adding service");
  }
}

void BluetoothClass::poll() {
  if (status == BLE_STATUS_SUCCESS) {
    HCI_Process();
  }
}

void BluetoothClass::reset() {
  BlueNRG_RST();
}

uint8_t BluetoothClass::setChallenge() {
  ECCX08.random(challenge, sizeof(challenge));
  status = aci_gatt_update_char_value(ServHandle, ChallengeCharHandle, 0, sizeof(challenge), challenge);
  if (status != BLE_STATUS_SUCCESS) {
    logError("i_status", status, "Error while set challenge");
    return BLE_STATUS_ERROR;
  }
  return BLE_STATUS_SUCCESS;
}

#define ADV_INTERVAL_MIN_MS 50
#define ADV_INTERVAL_MAX_MS 100

void BluetoothClass::setConnectable() {
  hci_le_set_scan_resp_data(0, NULL);

  String localName = String((char)AD_TYPE_COMPLETE_LOCAL_NAME) + name;
  status = aci_gap_set_discoverable(ADV_IND,
                                    (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                    STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                    localName.length(), localName.c_str(), 0, NULL, 0, 0);
  if (status != BLE_STATUS_SUCCESS) {
    logError("i_status", status);
  }
}

// BLE actually only supports sending 20 bytes of data per characteristic. We need a little "protocol" to support longer array.
// You split the array in 20 bytes arrays where the first 2 bytes contain the total length of data to come (lower byte first), including the data in this array.
// Example:
// You start with an array like this:
// [72, 101, 108, 108, 111, 32, 109, 121, 32, 108, 111, 118, 101, 108, 121, 32, 119, 111, 114, 108, 100, 44, 32, 105, 32, 108, 111, 118, 101, 32, 121, 111, 117, 32, 97, 110, 100, 32, 97, 108, 108, 32, 97, 114, 111, 117, 110, 100, 32, 121, 111, 117]
// Then you generate the following 3 arrays:
// [52, 0, 72, 101, 108, 108, 111, 32, 109, 121, 32, 108, 111, 118, 101, 108, 121, 32, 119, 111]
// [34, 0, 114, 108, 100, 44, 32, 105, 32, 108, 111, 118, 101, 32, 121, 111, 117, 32, 97, 110]
// [16, 0, 100, 32, 97, 108, 108, 32, 97, 114, 111, 117, 110, 100, 32, 121, 111, 117]

#define IS(a) handle == a + 1

void BluetoothClass::Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data) {
  if (IS(AuthCharHandle) || IS(CmdCharHandle)) {
    char strBuffer[19];
    uint16_t messageLength = att_data[0] | (att_data[1] << 8);
    uint8_t strBegin = 2;
    if (messageLength != continueLength) {  // first message
      message = "";
      if (IS(CmdCharHandle)) {
        memcpy(hmac, &att_data[1], HMAC_LENGTH);
        strBegin += HMAC_LENGTH;
      }
    }
    uint8_t strLength = data_length - strBegin;
    strncpy(strBuffer, (const char *)&att_data[strBegin], strLength);
    strBuffer[strLength] = '\0';
    continueLength = messageLength - data_length + 2;  // + 2 because att_data[0] and att_data[1] are not data
    message += String(strBuffer);
    logDebug("length", continueLength);
    logDebug("i_msg", message);
    if (continueLength == 0) {  // last message
      if (IS(AuthCharHandle)) {
        Command.authorize(message);
      } else if (IS(CmdCharHandle)) {
        SHA256.beginHmac(Command.getAuthSecret(), AUTH_SECRET_LENGTH);
        SHA256.write((const uint8_t *)message.c_str(), message.length());
        SHA256.write(challenge, sizeof(challenge));
        SHA256.endHmac();
        uint8_t computedHmac[HMAC_LENGTH];
        SHA256.readBytes(computedHmac, HMAC_LENGTH);
        if (memcmp(computedHmac, hmac, HMAC_LENGTH)) {
          Command.processJson(message, true);
          Bluetooth.setChallenge();
        }
      }
      message = "";
    }
  }
}

void BluetoothClass::GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle) {
  connection_handle = handle;
  char sprintbuff[64];
  snprintf(sprintbuff, 64, "BLE Connected to device: %02X-%02X-%02X-%02X-%02X-%02X", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
  logInfo(sprintbuff);
  Command.unauthorize();
  Bluetooth.setChallenge();
  System.setStayAwake(true);
}

void BluetoothClass::GAP_DisconnectionComplete_CB() {
  logInfo("BLE Disconnected");
  setConnectable();
  System.setStayAwake(false);
}

void BluetoothClass::Read_Request_CB(uint16_t handle) {
  if (connection_handle != 0) {
    aci_gatt_allow_read(connection_handle);
  }
}

BluetoothClass Bluetooth;
