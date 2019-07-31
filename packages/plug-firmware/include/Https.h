#ifndef Https_h
#define Https_h

class HttpsClass {
 public:
  int32_t download(const char* host, const char* from, const char* to);
};

extern HttpsClass Https;

#endif  // Https_h