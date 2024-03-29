/*
  ArduinoECCX08 - CSR (Certificate Signing Request)

  This sketch can be used to generate a CSR for a private key
  generated in an ECC508/ECC608 crypto chip slot.

  If the ECC508/ECC608 is not configured and locked it prompts
  the user to configure and lock the chip with a default TLS
  configuration.

  The user is prompted for the following information that is contained
  in the generated CSR:
  - country
  - state or province
  - locality
  - organization
  - organizational unit
  - common name

  The user can also select a slot number to use for the private key
  A new private key can also be generated in this slot.

  The circuit:
  - Arduino MKR board equipped with ECC508 or ECC608 chip

  This example code is in the public domain.
*/

#include <ArduinoECCX08.h>
#include <utility/ECCX08CSR.h>
#include <utility/ECCX08DefaultTLSConfig.h>
#ifndef ARDUINO_SAMD_MKR1000
#include <MKRNB.h>
#include <Modem.h>
#endif

static void setEdrx() {
  // to fix long mqtt cmd delay:
  // https://portal.u-blox.com/s/question/0D52p00008RlYDrCAN/long-delays-using-sarar41002b-with-att
  // https://github.com/botletics/SIM7000-LTE-Shield/wiki/Current-Consumption
  MODEM.send("AT+CEDRXS=1,4,\"0000\"");  // up to 5.12 sec delay
  // MODEM.send("AT+CEDRXS=0");  // no delay, but more power consumption
  MODEM.waitForResponse(2000);
  String modemResponse;
  // MODEM.send("AT+CEDRXS=?");
  // MODEM.waitForResponse(2000, &modemResponse);
  MODEM.send("AT+CEDRXS?");
  MODEM.waitForResponse(2000, &modemResponse);
}

static void chooseNetwork() {
  MODEM.send("AT+COPS=2");
  MODEM.waitForResponse(2000);
  // https://www.u-blox.com/sites/default/files/SARA-R4-N4-Application-Development_AppNote_%28UBX-18019856%29.pdf
  // 0: SW default profile, 1: ICCID Select, 2: AT&T, 3: Verizon
  MODEM.send("AT+UMNOPROF=0");
  MODEM.waitForResponse(2000);
  String modemResponse;
  MODEM.send("AT+UMNOPROF?");
  MODEM.waitForResponse(2000, &modemResponse);
  setEdrx();
  // reset sim card
  MODEM.send("AT+CFUN=15");
}

void setup() {
  Serial.begin(115200);
  // while (!Serial)
  //   ;

#ifndef ARDUINO_SAMD_MKR1000
  int ret = MODEM.begin();
  if (ret != 1) {
    Serial.println("MODEM.begin() failed: " + String(ret));
    return;
  }

  int start = millis();
  while (!MODEM.noop())
    ;

  // import root certificates to modem
  NBSSLClient client(false);
  client.connect("127.0.0.1", 443);
  while (millis() - start < 12000) {
    client.ready();
  }

  // set LED as network status indicator
  MODEM.send("AT+UGPIOC=16,2");
  ret = MODEM.waitForResponse(2000);
  if (ret != 1) {
    Serial.println("Error 'AT+UGPIOC=16,2' ret: " + String(ret));
    return;
  }

  // get firmware version
  MODEM.send("ATI9");
  String modemResponse = "";
  MODEM.waitForResponse(2000, &modemResponse);
  if (modemResponse != "L0.0.00.00.05.08,A.02.04") {
    Serial.println("Error 'ATI9' response: " + modemResponse);
  }

  // disable OTA firmware update
  ret = 0;
  while (ret != 1) {
    MODEM.send("AT+UFOTACONF=2,-1");
    ret = MODEM.waitForResponse(2000);
  }

  // confirm OTA firmware update is disabled
  MODEM.send("AT+UFOTACONF=2");
  modemResponse = "";
  MODEM.waitForResponse(2000, &modemResponse);
  if (modemResponse != "+UFOTACONF: 2, -1") {
    Serial.println("Error 'AT+UFOTACONF=2' response: " + modemResponse);
  }

  chooseNetwork();

#endif

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1)
      ;
  }

  String serialNumber = ECCX08.serialNumber();

  // Serial.print("ECCX08 Serial Number = ");
  // Serial.println(serialNumber);
  // Serial.println();

  bool newPrivateKey = false;

  if (!ECCX08.locked()) {
    // String lock = promptAndReadLine("The ECCX08 on your board is not locked, would you like to PERMANENTLY configure and lock it now? (y/N)", "N");
    // lock.toLowerCase();

    // if (!lock.startsWith("y")) {
    //   Serial.println("Unfortunately you can't proceed without locking it :(");
    //   while (1);
    // }

    if (!ECCX08.writeConfiguration(ECCX08_DEFAULT_TLS_CONFIG)) {
      Serial.println("Writing ECCX08 configuration failed!");
      while (1)
        ;
    }

    if (!ECCX08.lock()) {
      Serial.println("Locking ECCX08 configuration failed!");
      while (1)
        ;
    }

    // Serial.println("ECCX08 locked successfully");
    // Serial.println();

    newPrivateKey = true;
  }

  // Serial.println("Hi there, in order to generate a new CSR for your board, we'll need the following information ...");
  // Serial.println();

  // String country            = promptAndReadLine("Country Name (2 letter code)", "");
  // String stateOrProvince    = promptAndReadLine("State or Province Name (full name)", "");
  // String locality           = promptAndReadLine("Locality Name (eg, city)", "");
  // String organization       = promptAndReadLine("Organization Name (eg, company)", "");
  // String organizationalUnit = promptAndReadLine("Organizational Unit Name (eg, section)", "");
  // String common             = promptAndReadLine("Common Name (e.g. server FQDN or YOUR name)", serialNumber.c_str());
  // String slot               = promptAndReadLine("What slot would you like to use? (0 - 4)", "0");
  // String generateNewKey     = promptAndReadLine("Would you like to generate a new private key? (Y/n)", "Y");

  // Serial.println();

  // generateNewKey.toLowerCase();

  // if (!ECCX08CSR.begin(slot.toInt(), generateNewKey.startsWith("y"))) {
  //   Serial.println("Error starting CSR generation!");
  //   while (1);
  // }

  String country = "US";
  String stateOrProvince = "CA";
  String locality = "Los Angeles";
  String organization = "Waive";
  String organizationalUnit = "Engineering";
  String common = serialNumber;
  String slot = "0";

  if (!ECCX08CSR.begin(slot.toInt(), newPrivateKey)) {
    Serial.println("Error starting CSR generation!");
    while (1)
      ;
  }

  ECCX08CSR.setCountryName(country);
  ECCX08CSR.setStateProvinceName(stateOrProvince);
  ECCX08CSR.setLocalityName(locality);
  ECCX08CSR.setOrganizationName(organization);
  ECCX08CSR.setOrganizationalUnitName(organizationalUnit);
  ECCX08CSR.setCommonName(common);

  String csr = ECCX08CSR.end();

  if (!csr) {
    Serial.println("Error generating CSR!");
    while (1)
      ;
  }

  csr.replace("\n", "\\n");

  // Serial.println("Here's your CSR, enjoy!");
  // Serial.println();
  Serial.println("{\"serial\":\"" + serialNumber + "\",\"csr\":\"" + csr + "\"}");
}

void loop() {
  // do nothing
}

String promptAndReadLine(const char *prompt, const char *defaultValue) {
  Serial.print(prompt);
  Serial.print(" [");
  Serial.print(defaultValue);
  Serial.print("]: ");

  String s = readLine();

  if (s.length() == 0) {
    s = defaultValue;
  }

  Serial.println(s);

  return s;
}

String readLine() {
  String line;

  while (1) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r') {
        // ignore
        continue;
      } else if (c == '\n') {
        break;
      }

      line += c;
    }
  }

  return line;
}
