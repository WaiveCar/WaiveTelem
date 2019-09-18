#include <ArduinoECCX08.h>
#include <JsonLogger.h>

#include "System.h"

#define CRASH_REPORT_START_BYTE 48
#define CRASH_REPORT_START_HEADER 6
#define CRASH_REPORT_START_DATA 0xa5

void shutdown() {
  Serial.print("shutdown()");
  uint32_t top;
  uint8_t data[72];
  int ret = ECCX08.readSlot(13, data, 72);
  Serial.println(" ret: " + String(ret));
  int j = CRASH_REPORT_START_BYTE + CRASH_REPORT_START_HEADER;
  uint32_t value = 0;
  for (int i = 1; j < 72 && (uint32_t)(&top + i) < 0x20008000; i++) {
    if (value == ((*(&top + i)) & 0xfffffe)) {  // skip repeated value
      continue;
    }
    value = *(&top + i);
    if (value >= 0x6000 && value < 0x20000 && value & 0x1) {
      value &= 0xfffffe;
      data[j] = value;
      data[j + 1] = value >> 8;
      data[j + 2] = value >> 16;
      j += 3;
      char str[7];
      sprintf(str, "%lx", value);
      Serial.println(str);
    }
  }
  int num = (j - CRASH_REPORT_START_BYTE - CRASH_REPORT_START_HEADER) / 3;
  if (num > 0) {
    data[CRASH_REPORT_START_BYTE] = CRASH_REPORT_START_DATA;
    data[CRASH_REPORT_START_BYTE + 1] = num;
    uint32_t time = System.getTime();
    data[CRASH_REPORT_START_BYTE + 2] = time;
    data[CRASH_REPORT_START_BYTE + 3] = time >> 8;
    data[CRASH_REPORT_START_BYTE + 4] = time >> 16;
    data[CRASH_REPORT_START_BYTE + 5] = time >> 24;
    ret = ECCX08.writeSlot(13, data, 72);
    Serial.println("ret: " + String(ret) + ",j: " + String(j));
  }
}

void checkCrashReport() {
  int maxLen = (72 - CRASH_REPORT_START_BYTE - CRASH_REPORT_START_HEADER) / 3;
  char* list[maxLen] = {NULL};
  uint8_t data[72];
  int ret = ECCX08.readSlot(13, data, 72);
  uint8_t len = data[CRASH_REPORT_START_BYTE + 1];
  logDebug("i|ret", ret, "i|len", len);
  if (data[CRASH_REPORT_START_BYTE] == CRASH_REPORT_START_DATA && len < 7 && len > 0) {
    int i = 0;
    int j = CRASH_REPORT_START_BYTE + CRASH_REPORT_START_HEADER;
    for (; i < len; i++) {
      uint32_t value = data[j] | data[j + 1] << 8 | data[j + 2] << 16;
      j += 3;
      list[i] = (char*)malloc(7);
      sprintf(list[i], "%lx", value & 0xffffff);
      // logDebug("list[i]", list[i]);
    }
    char reported[256];
    // logDebug("i|i", i);
    // logDebug("list[0]", list[0]);
    uint32_t time = data[CRASH_REPORT_START_BYTE + 2] |
                    data[CRASH_REPORT_START_BYTE + 3] << 8 |
                    data[CRASH_REPORT_START_BYTE + 4] << 16 |
                    data[CRASH_REPORT_START_BYTE + 5] << 24;
    json(reported, "{|crash", "i|time", time, "s[backtrace", len, list, "}|");
    System.report(reported);

    data[CRASH_REPORT_START_BYTE] = 0;
    data[CRASH_REPORT_START_BYTE + 1] = 0;
    ret = ECCX08.writeSlot(13, data, 72);
    logDebug("i|ret", ret);

    for (int k = 0; k < maxLen; k++) {
      free(list[k]);
    }
  }
}
