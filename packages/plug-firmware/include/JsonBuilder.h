#ifndef JsonBuilder_h
#define JsonBuilder_h

#include <Arduino.h>
#include <cstdarg>

#define json(...) JsonBuilder.build(__VA_ARGS__, NULL)

class JsonBuilderClass {
 public:
  void build(String& json, const char* key, ...);
  void vbuild(String& json, const char* key, va_list args);
};

extern JsonBuilderClass JsonBuilder;

#endif  // JsonBuilder_h