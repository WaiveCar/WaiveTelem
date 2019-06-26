/*
   WaiveTelem arduino code revision 0.1


*/

#include <Arduino.h>
#include <wiring_private.h>
#include <SPI.h>

#include "pin_definitions.h"
#include "SD_interface.h"
#include "BLE_interface.h"
#include "GPS_interface.h"
#include "MQTT_interface.h"
#include "vehicle_control.h"
//#include "can_bus_interface.h" // not needed, included in SD_interface



//Status structure, initialized in setup, populated as status changes in loop
struct Status {
  bool ignition;
  bool central_lock_command;
  bool central_lock_status;
  bool charging;
  bool immobilized;
  float mileage;
  float fuel_charge_level;
  float speed_mph;
  bool doors;
  bool front_left_door;
  bool front_right_door;
  bool back_left_door;
  bool back_right_door;
  bool windows;
  bool front_left_window;
  bool front_right_window;
  bool back_left_window;
  bool back_right_window;
  double gps_lat;
  double gps_lon;
  char *gps_time;
  float timestamp;
  int cell_strength;

};


//declare config file name
const char *filename = "/config.txt";
Config config;

Status currentStatus;


unsigned long lastMillis = 0;

void setup() {
  // put your setup code here, to run once:
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  //Setup pin modes
  set_pin_modes();

  SPI.begin();

  //Initialize SD library
  while (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("Failed to initialize SD Library"));
    delay(1000);
  }


  // Dump config file
  Serial.println(F("Print config file..."));
  printFile(filename);


  //Load config file
  loadConfiguration(filename, config);
  Serial.print("Make: ");
  Serial.println(config.make);
  Serial.print("Model: ");
  Serial.println(config.model);
  Serial.print("ignition can id: ");
  Serial.println(config.can_id[ignition]);

  //  Initialize Flash Library
  while (!SerialFlash.begin(FL_CS_PIN)) {
    Serial.println(F("Failed to initialize Flash Library"));
    delay(1000);
  }

  Serial.println("Loading Immobilizer state...");
  //Load Immobilizer last state
  loadImmobilizer(immofile, currentStatus.immobilized);
  currentStatus.immobilized ? immobilize(currentStatus.immobilized, RELAY_2_PIN) : unimmobilize(currentStatus.immobilized, RELAY_2_PIN);
  Serial.print("Immobilized: ");
  Serial.println(currentStatus.immobilized);

  //--- Configure ACAN2515
  Serial.println ("Configure ACAN2515") ;
  Serial.print("CAN0, baud: ");
  Serial.println(config.bus_baud[0]);
  ACAN2515Settings settings (QUARTZ_FREQUENCY, config.bus_baud[0] * 1000) ; // CAN bit rate 500 kb/s
  //  settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
  const ACAN2515Mask rxm0 = standard2515Mask ((0x7FF - (min_can(0) - max_can(0))), 0, 0) ; // For filter #0 and #1
  const ACAN2515AcceptanceFilter filters[] = {
    {standard2515Filter (min_can(0), 0, 0), onReceive1}
  } ;
  const uint32_t errorCode = can0.begin (settings, [] { can0.isr () ; }, rxm0, filters, 1) ;
  if (errorCode != 0) {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

  if (config.num_can > 1) {
    Serial.print("CAN1, baud: ");
    Serial.println(config.bus_baud[1]);
    ACAN2515Settings settings (QUARTZ_FREQUENCY, config.bus_baud[1] * 1000) ; // CAN bit rate stored value
    //  settings.mRequestedMode = ACAN2515Settings::LoopBackMode ; // Select loopback mode
    const ACAN2515Mask rxm0 = standard2515Mask ((0x7FF - (min_can(1) - max_can(1))), 0, 0) ; // For filter #0 and #1
    const ACAN2515AcceptanceFilter filters[] = {
      {standard2515Filter (min_can(1), 0, 0), onReceive1}
    } ;
    const uint32_t errorCode = can1.begin (settings, [] { can1.isr () ; }, rxm0, filters, 1) ;
    if (errorCode != 0) {
      Serial.print ("Configuration error 0x") ;
      Serial.println (errorCode, HEX) ;
    }
  }

  // BLE initialization
  Serial.print("Setting up BLE...");
  BLEsetup();
  Serial.println("!");


  // GPS Initialization
  GPSSerial.begin(9600);
  while (!GPSSerial) {
    Serial.println("GPS Loading...");
    delay(1000);
  }

  // Crypto initialization
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }
  // Initialize MQTT
  String clientId = calculateClientId();
  mqttClient.setId(clientId);
  mqttClient.onMessage(onMessageReceived);


  // Network initialization
#ifdef ARDUINO_SAMD_MKR1000
  // WiFi initialization
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

#elif defined(ARDUINO_SAMD_MKRNB1500)
  if (nbAccess.status() != NB_READY || gprs.status() != GPRS_READY) {
    connectNB();
  }
#endif
  Serial.println("\nStarting MQTT Connection...");
  // if you get a connection report back via serial:
  while (!mqttClient.connected()) {
    connectMQTT();
    delay(100);
  }

  Serial.println("Survived setup");
}

void loop() {
  // looped CAN function
  can0.dispatchReceivedMessage();
  if (config.num_can > 1) can1.dispatchReceivedMessage();


  // looped MQTT functions
  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    Serial.print("Reconnecting MQTT: ");
    Serial.println(millis());
    connectMQTT();
  }
  // poll for new MQTT messages and send keep alives
  mqttClient.poll();
  // publish a message.
  if (millis() - lastMillis > 15000) {
    // looped GPS function
    if (gps.satellites.value() > 0) {
      Serial.print("sats: ");
      Serial.println(gps.satellites.value());

      set_gps_info();
    }
    lastMillis = millis();

    //    publishMessage("/state", "dummy message");
    //    Serial.println("published the message");
  }

  // looped BLE functions
  aci_loop(); //must run frequently
  if (ble_rx_buffer_len) {//Check if data is available
    Serial.print(ble_rx_buffer_len);
    Serial.print(" : ");
    Serial.println((char*)ble_rx_buffer);
    Serial.println("you just got some BLE data, pushing as a command");
    //    String buf_str = String(*ble_rx_buffer);
    command_handler((char*)ble_rx_buffer);
    ble_rx_buffer_len = 0;//clear afer reading
  }
  //forward serial port input to BLE output
  if (Serial.available()) {//Check if serial input is available to send
    delay(10);//should catch input
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
    sendBuffer[sendLength] = '\0'; //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
    {
      Serial.println(F("TX dropped!"));
    } else
    {
      Serial.println(F("Forwarded serial input to BLE"));
    }
  }
  gpsSmartDelay(1);
}

// MQTT subscription hanlder:
void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  String cmd_str = "";
  char topic[50];
  char command_topic[50];
  char command[25];
  String command_topic_str = "/devices/" + deviceId + "/commands";
  command_topic_str.toCharArray(command_topic, command_topic_str.length() + 1);
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  mqttClient.messageTopic().toCharArray(topic, mqttClient.messageTopic().length() + 1);
  Serial.print(topic);
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  int i = 0;
  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    cmd_str += (char)mqttClient.read();
  }
  Serial.println(cmd_str);
  if (!strcmp(topic, command_topic))
  {
    cmd_str.toCharArray(command, cmd_str.length() + 1);
    command_handler(command);
  }
}

//Interrupt based CAN receiver/printer
static void onReceive1(const CANMessage & inMessage) {
  if (!inMessage.rtr) {
    //Software filter for relevant IDs
    for (int i = 0; i < num_can_items; i++) {
      if (inMessage.id == config.can_id[i]) {

        int data = inMessage.data[config.can_byte_num[i]];
        int data_shifter = config.can_bit_num[i] + config.can_data_len[i];
        int j = 1;
        while (data_shifter > 8) {
          data = data | (inMessage.data[config.can_byte_num[i] + j] << (8 * j));
          data_shifter -= 8;
          j++;
        }
        data = bit_filter(data, config.can_bit_num[i], config.can_data_len[i]);
        // set status based on which data was received
        if (i == 0) {
          set_ignition_status(data);
        } else if (i == 1) {
          set_mileage(data, 16.0934);
        } else if (i == 2) {
          set_fuel_charge_level(data);
        } else if (i == 3) {
          set_charging_status(data);
        } else if (i == 4) {
          set_speed(data, 100.0);
        } else if (i == 5) {
          set_central_lock_status(data);
        } else if (i == 6) {
          // door front left
          set_door_status(data, 1);
        } else if (i == 7) {
          // door front right
          set_door_status(data, 2);
        } else if (i == 8) {
          // door back left
          set_door_status(data, 3);
        } else if (i == 9) {
          // door back right
          set_door_status(data, 4);
        } else if (i == 10) {
          // window driver
          set_window_status(data, 1);
        } else if (i == 11) {
          // window codriver
          set_window_status(data, 2);
        } else if (i == 12) {
          // window back left
          set_window_status(data, 3);
        } else if (i == 13) {
          // window back right
          set_window_status(data, 4);
        }
      }
    }
  }
}

// Set CAN filter to limit interupts to as close to relevant as possible
// Filter = min_can, mask = 0x7FF - (max_can - min_can)
int min_can(int bus_num) {
  int min_can = 0x7FF;
  for (int i = 0; i < num_can_items; i++) {
    if (config.bus_id[i] == bus_num) min_can = min(min_can, config.can_id[0]);
  }
  return min_can;
}
int max_can(int bus_num) {
  int max_can = 0;
  for (int i = 0; i < num_can_items; i++) {
    if (config.bus_id[i] == bus_num) max_can = max(max_can, config.can_id[0]);
  }
  return max_can;
}



int command_handler(char *command)
{

  if (!strcmp(command, "lock\n") or !strcmp(command, "lock")) {
    Serial.println("command received, locking doors");
    unlock_lock_doors(DOOR_LOCK_PIN);
    set_central_lock_command(command);
  } else if (!strcmp(command, "unlock\n") or !strcmp(command, "unlock")) {
    Serial.println("command received, unlocking doors");
    unlock_lock_doors(DOOR_UNLOCK_PIN);
    set_central_lock_command(command);
  } else if (!strcmp(command, "immobilize\n") or !strcmp(command, "immobilize")) {
    Serial.println("command received, locking immo");
    immobilize(currentStatus.immobilized, RELAY_2_PIN);
    send_immobilized_status();
  } else if (!strcmp(command, "unimmobilize\n") or !strcmp(command, "unimmobilize")) {
    Serial.println("command received, unlocking immo");
    unimmobilize(currentStatus.immobilized, RELAY_2_PIN);
    send_immobilized_status();
  } else {
    Serial.print("Unknown command received: ");
    Serial.println(command);
  }

  /*
     TODO: other commands
     - reset gps
     - reset bluetooth
     - reset network
     - reset board
     - update config
     - update status

  */

  return 1;
}


// function to set each status piece, also sends to mqtt
int set_ignition_status(int data) {
  bool ign_status;
  if (data == 3) {
    ign_status = true;
  } else if (data == 0) {
    ign_status = false;
  } else if (data == 2 or data == 4) {
    return 1;
  } else {
    Serial.print("weird ignition status: ");
    Serial.print(data);
    return 0;
  }


  if (currentStatus.ignition != ign_status) {
    Serial.print("Setting ignition status as: ");
    Serial.println(ign_status);
    currentStatus.ignition = ign_status;
    mqtt_payload_str = "{\"ignition\": " + String(currentStatus.ignition) + "}";
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state";// /ignition";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;

}

int set_central_lock_command(char *command) {
  bool cnt_lck_cmd;
  if (!strcmp(command, "unlock\n") or !strcmp(command, "unlock")) {
    cnt_lck_cmd = 1;
  } else if (!strcmp(command, "lock\n") or !strcmp(command, "lock")) {
    cnt_lck_cmd = 0;
  } else {
    Serial.print("weird central lock command: ");
    Serial.println(command);
    return 0;
  }
  if (currentStatus.central_lock_command != cnt_lck_cmd) {
    Serial.print("Setting central lock command as: ");
    Serial.println(cnt_lck_cmd);
    currentStatus.central_lock_command = cnt_lck_cmd;
    mqtt_payload_str = "{\"central_lock_cmd\": \"" + String(currentStatus.central_lock_command) + "\"}";
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state"; // /central_lock_command";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;
}


int set_central_lock_status(int data) {
  bool cnt_lck_stat;
  if (data == 1) {
    cnt_lck_stat = 1;
  } else if (data == 0) {
    cnt_lck_stat = 0;
  } else {
    Serial.print("weird central lock data: ");
    Serial.println(data);
    return 0;
  }
  if (currentStatus.central_lock_status != cnt_lck_stat) {
    Serial.print("Setting central lock status as: ");
    Serial.println(cnt_lck_stat);
    currentStatus.central_lock_status = cnt_lck_stat;
    mqtt_payload_str = String(currentStatus.central_lock_status);
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state/central_lock_command";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;
}

int set_charging_status(int data) {
  bool chrg_stat;
  if (data == 1) {
    chrg_stat = 1;
  } else if (data == 0) {
    chrg_stat = 0;
  } else {
    Serial.print("weird charge status data: ");
    Serial.println(data);
    return 0;
  }
  if (currentStatus.charging != chrg_stat) {
    Serial.print("Setting charging status as: ");
    Serial.println(chrg_stat);
    currentStatus.charging = chrg_stat;
    mqtt_payload_str = String(currentStatus.charging);
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state/charging";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;
}

void send_immobilized_status() {
  //todo: tie this into the vehicle control functions
  mqtt_payload_str = "{\"immobilized\": " + String(currentStatus.immobilized) + "}";
  mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
  mqtt_topic_str = "/state"; // /immobilized";
  mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
  Serial.print("Updating topic: ");
  Serial.print(mqtt_topic);
  Serial.print(" with message: ");
  Serial.println(mqtt_payload);
  publishMessage(mqtt_topic, mqtt_payload);
}

int set_mileage(int data, float constant) {
  // todo: dont send on every single bit change
  currentStatus.mileage = data / constant;
  mqtt_payload_str = "{\"mileage\": " + String(currentStatus.mileage) + "}";
  mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
  mqtt_topic_str = "/state"; // /mileage";
  mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
  Serial.print("Updating topic: ");
  Serial.print(mqtt_topic);
  Serial.print(" with message: ");
  Serial.println(mqtt_payload);
  publishMessage(mqtt_topic, mqtt_payload);
  return 1;
}

int set_fuel_charge_level(int data) {
  // todo: dont send on every single bit change
  currentStatus.fuel_charge_level = data;
  mqtt_payload_str = "{\"fuel_charge level\": " + String(currentStatus.fuel_charge_level) + "}";
  mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
  mqtt_topic_str = "/state"; // /fuel_charge_level";
  mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
  Serial.print("Updating topic: ");
  Serial.print(mqtt_topic);
  Serial.print(" with message: ");
  Serial.println(mqtt_payload);
  publishMessage(mqtt_topic, mqtt_payload);
  return 1;
}

int set_speed(int data, float constant) {
  currentStatus.speed_mph = data;
  mqtt_payload_str = "{\"speed\": " + String(currentStatus.speed_mph) + "}" + "}";
  mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
  mqtt_topic_str = "/state"; // /speed";
  mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
  Serial.print("Updating topic: ");
  Serial.print(mqtt_topic);
  Serial.print(" with message: ");
  Serial.println(mqtt_payload);
  publishMessage(mqtt_topic, mqtt_payload);
  return 1;
}

int set_door_status(int data, int door) {
  bool door_stat = false;
  if (data > 0)
    door_stat = true;

  if (door > 0) {
    if (door == 1) {
      currentStatus.front_left_door = door_stat;
    } else if (door == 2) {
      currentStatus.front_right_door = door_stat;
    } else if (door == 3) {
      currentStatus.back_left_door = door_stat;
    } else if (door == 4) {
      currentStatus.back_right_door = door_stat;
    } else {
      Serial.print("unknown door number: ");
      Serial.println(door);
      return 0;
    }
    door_stat = currentStatus.front_left_door | currentStatus.front_right_door | currentStatus.back_left_door | currentStatus.back_right_door;
  }
  if (currentStatus.doors != door_stat) {
    Serial.print("Setting door status as: ");
    Serial.println(door_stat);
    currentStatus.doors = door_stat;

    char* payload;
    door_stat ? payload = "1" : payload = "0";
    mqtt_payload_str = "{\"doors\": " + String(payload) + "}";
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state"; // /doors";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;
}

int set_window_status(int data, int window) {
  bool window_stat = false;
  if (data > 0)
    window_stat = true;

  if (window > 0) {
    if (window == 1) {
      currentStatus.front_left_window = window_stat;
    } else if (window == 2) {
      currentStatus.front_right_window = window_stat;
    } else if (window == 3) {
      currentStatus.back_left_window = window_stat;
    } else if (window == 4) {
      currentStatus.back_right_window = window_stat;
    } else {
      Serial.print("unknown window number: ");
      Serial.println(window);
      return 0;
    }
    window_stat = currentStatus.front_left_window | currentStatus.front_right_window | currentStatus.back_left_window | currentStatus.back_right_window;
  }
  if (currentStatus.windows != window_stat) {
    Serial.print("Setting window status as: ");
    Serial.println(window_stat);
    currentStatus.windows = window_stat;
    mqtt_payload_str = String(currentStatus.windows);
    mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
    mqtt_topic_str = "/state/windows";
    mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
    Serial.print("Updating topic: ");
    Serial.print(mqtt_topic);
    Serial.print(" with message: ");
    Serial.println(mqtt_payload);
    publishMessage(mqtt_topic, mqtt_payload);
  }
  return 1;
}

int set_gps_info() {
  Serial.println("/****GPS DATA****/");
  Serial.print("Date.Time: ");
  char sz[64];
  sprintf(sz, "%02d/%02d/%02d.%02d:%02d:%02d", gps.date.month(), gps.date.day(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
  currentStatus.gps_time = sz;
  Serial.println(sz);
  Serial.print("Satellites aquired: ");
  Serial.println(gps.satellites.value());
  Serial.print("HDOP: ");
  Serial.println(gps.hdop.hdop(), 4);
  Serial.print("Coordinates: ");
  Serial.print(gps.location.lat(), 8);
  currentStatus.gps_lat = gps.location.lat();
  currentStatus.gps_lon = gps.location.lng();
  Serial.print(", ");
  Serial.println(gps.location.lng(), 8);
  Serial.println("Fix age: ");
  Serial.println(gps.location.age());
  Serial.print("Altitude(ft): ");
  Serial.println(gps.altitude.feet());
  mqtt_payload_str = "{\"location\": \"" + String(currentStatus.gps_lat, 6) + "," + String(currentStatus.gps_lon, 6) + "\"}";
  mqtt_payload_str.toCharArray(mqtt_payload, mqtt_payload_str.length() + 1);
  mqtt_topic_str = "/state"; // /location";
  mqtt_topic_str.toCharArray(mqtt_topic, mqtt_topic_str.length() + 1);
  Serial.print("Updating topic: ");
  Serial.print(mqtt_topic);
  Serial.print(" with message: ");
  Serial.println(mqtt_payload);
  publishMessage(mqtt_topic, mqtt_payload);
}

/*
  char *gps_time
  float timestamp;
  int cell_strength;

*/
