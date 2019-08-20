#include "JsonBuilder.h"

#define KV_STR(key, value) json += String("\"") + key + "\":\"" + value + "\""
#define KV_NUM(key, value) json += String("\"") + key + "\":" + String(value).c_str()
#define KV_OTHER(key, value) json += String("\"") + key + "\":" + value

void JsonBuilderClass::build(String& json, const char* key, ...) {
  va_list args;
  va_start(args, key);
  vbuild(json, key, args);
  va_end(args);
}

void JsonBuilderClass::vbuild(String& json, const char* key, va_list arg) {
  bool enclose = true;
  if (key[1] == '|' && key[0] == '-') {
    enclose = false;
    key = va_arg(arg, const char*);
  }
  if (enclose) {
    json += "{";
  }
  bool firstKey = true;
  while (key) {
    if (firstKey) {
      firstKey = false;
    } else {
      json += ",";
    }
    if (key[1] == '|') {
      if (key[0] == 'i') {  // integer
        key = key + 2;
        long value = va_arg(arg, long);
        KV_NUM(key, value);
      } else if (key[0] == 'f') {  // floating number
        key = key + 2;
        double value = va_arg(arg, double);
        KV_NUM(key, value);
      } else if (key[0] == 'b') {  // boolean
        key = key + 2;
        bool value = va_arg(arg, int);
        KV_NUM(key, value);
      } else if (key[0] == 'o') {  // others "{object}", "[array]", "null"
        key = key + 2;
        const char* value = va_arg(arg, const char*);
        KV_OTHER(key, value);
      } else if (key[0] == '+') {  // insert fragment
        key = key + 2;
        json += key;
      }
    } else {  // string
      bool isNoMore = false;
      const char* value = va_arg(arg, const char*);
      if (!value) {
        value = key;
        key = "_";
        isNoMore = true;
      }
      if (strchr(value, '"')) {
        String newValue = value;
        newValue.replace("\"", "\\\"");
        KV_STR(key, newValue.c_str());
      } else {
        KV_STR(key, value);
      }
      if (isNoMore) {
        break;
      }
    }
    key = va_arg(arg, const char*);
  }
  if (enclose) {
    json += "}";
  }
}

JsonBuilderClass JsonBuilder;
