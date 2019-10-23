#ifndef ARDUINO_SAMD_MKR1000
#include <Arduino.h>
#include <JsonLogger.h>
#include <MKRNB.h>
#include <Modem.h>
#include <WDTZero.h>

#include "Config.h"
#include "Internet.h"
#include "System.h"

static NB nbAccess;

static GPRS gprs;
static NBScanner nbScanner;

void saveConfigAndReboot() {
  MODEM.send("AT+CFUN=15");
  delay(8000);
}

void checkModemFirmwareUpdate() {
  // https://www.u-blox.com/sites/default/files/SARA-R4-FW-Update_AppNote_%28UBX-17049154%29.pdf
  delay(16000);
  // confirm OTA firmware update is disabled
  String modemResponse = "";
  MODEM.send("AT+UFOTACONF=2");
  MODEM.waitForResponse(5000, &modemResponse);
  if (modemResponse != "+UFOTACONF: 2, -1") {
    logWarn("AT+UFOTACONF=2", modemResponse.c_str());

    // disable OTA firmware update
    int ret = 0;
    while (ret != 1 || modemResponse == "ERROR") {
      MODEM.send("AT+UFOTACONF=2,-1");
      ret = MODEM.waitForResponse(1000, &modemResponse);
    }
  }
}

void checkNetwork() {
  String modemResponse = "";
  MODEM.send("AT+UMNOPROF?");
  MODEM.waitForResponse(5000, &modemResponse);
  if (modemResponse != "+UMNOPROF: 0") {
    logWarn("AT+UMNOPROF?", modemResponse.c_str());

    MODEM.send("AT+COPS=2");  // unregister
    MODEM.waitForResponse(1000);
    // https://www.u-blox.com/sites/default/files/SARA-R4-N4-Application-Development_AppNote_%28UBX-18019856%29.pdf
    // 0: SW default profile, 1: ICCID Select, 2: AT&T, 3: Verizon (I'm unable to set it to 1 or 2)
    MODEM.send("AT+UMNOPROF=0");
    MODEM.waitForResponse(1000);

    saveConfigAndReboot();
  }
}

void checkEdrx() {
  // to fix long mqtt cmd delay:
  // https://portal.u-blox.com/s/question/0D52p00008RlYDrCAN/long-delays-using-sarar41002b-with-att
  // https://github.com/botletics/SIM7000-LTE-Shield/wiki/Current-Consumption
  String modemResponse = "";
  MODEM.send("AT+CEDRXS?");
  MODEM.waitForResponse(5000, &modemResponse);
  if (modemResponse.indexOf("+CEDRXS:\n") != -1) {
    logWarn("AT+CEDRXS?", modemResponse.c_str());

    // MODEM.send("AT+CEDRXS=1,4,\"0000\"");  // asleep for 5.12 sec, then awake for 10.24 sec
    MODEM.send("AT+CEDRXS=0");  // no delay, but more power consumption
    MODEM.waitForResponse(1000);

    saveConfigAndReboot();
  }
}

int InternetClass::begin() {
  Watchdog.setup(WDT_SOFTCYCLE1M);

  MODEM.debug(Serial);
  int ret = MODEM.begin(true);
  if (ret != 1) {
    logError("i|ret", ret);
    return 0;
  }

  checkModemFirmwareUpdate();
  checkNetwork();
  checkEdrx();

  Watchdog.setup(WDT_SOFTCYCLE8S);
  return 1;
}

bool InternetClass::connect() {
  JsonObject nb = Config.get()["nb"];
  const char* apn = nb["apn"] | "internet.swir";
  logInfo("apn", apn);
  nbAccess.setTimeout(20000);
  gprs.setTimeout(20000);
  Watchdog.setup(WDT_SOFTCYCLE32S);
  MODEM.debug(Serial);

  int start = millis();
  if ((nbAccess.begin(nb["pin"].as<char*>(), apn) != NB_READY) || (gprs.attachGPRS() != GPRS_READY)) {
    logWarn("Failed to connect, try later");
    Watchdog.setup(WDT_SOFTCYCLE8S);
    return false;
  }

  MODEM.noDebug();

  System.setTimes(getTime());
  logInfo("carrier", nbScanner.getCurrentCarrier().c_str(), "i|connectTime", millis() - start, "i|signal", getSignalStrength());
  Watchdog.setup(WDT_SOFTCYCLE8S);
  return true;
}

bool InternetClass::isConnected() {
  return nbAccess.status() == NB_READY && gprs.status() == GPRS_READY;
}

unsigned long InternetClass::getTime() {
  return nbAccess.getTime();
}

int InternetClass::getSignalStrength() {
  return nbScanner.getSignalStrength().toInt();
}

String InternetClass::getCarrier() {
  return nbScanner.getCurrentCarrier();
}

String InternetClass::getModemVersion() {
  String modemResponse = "";
  MODEM.send("ATI9");
  MODEM.waitForResponse(1000, &modemResponse);
  return modemResponse;
}

InternetClass Internet;

#endif
