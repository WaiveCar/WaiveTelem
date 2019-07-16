#ifndef Http_h
#define Http_h

class HttpClass {
 public:
  void download(const String& host, const String& file);
};

extern HttpClass Http;

#endif  // Http_h