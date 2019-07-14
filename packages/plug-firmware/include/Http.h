#ifndef Http_h
#define Http_h

class HttpClass {
 public:
  void download(String& host, String& file);
};

extern HttpClass Http;

#endif  // Http_h