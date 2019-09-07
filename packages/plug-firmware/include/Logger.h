#ifndef Logger_h
#define Logger_h

#include <SD.h>

class LoggerClass {
 public:
  int begin();
  File& getWriteFile();

 private:
  File writeFile;
};

extern LoggerClass Logger;

#endif  // Logger_h
