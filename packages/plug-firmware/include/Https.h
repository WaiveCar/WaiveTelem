#ifndef Http_h
#define Http_h

class HttpClass {
 public:
  int32_t download(const char* host, const char* from, const char* to);
};

extern HttpClass Http;

#endif  // Http_h