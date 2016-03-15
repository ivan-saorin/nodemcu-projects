/**************************************************************
   logger is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

#include "Logger.h"

#define ARDBUFFER 16

Logger::Logger() {
  _header = DEFAULT_HEADER;
}

Logger::Logger(char const *header) {
  _header = header;
}

void Logger::begin() {
  size_t s = Serial.print(_header);
  Serial.print(" ");
}
size_t Logger::print(const __FlashStringHelper *ifsh){
  return Serial.print(ifsh);
}
size_t Logger::print(const String &str) {
  return Serial.print(str);
}
size_t Logger::print(const char str[]) {
  return Serial.print(str);
}
size_t Logger::print(char c) {
  return Serial.print(c);
}
size_t Logger::print(unsigned char c, int b) {
  return Serial.print(c, b);
}
size_t Logger::print(int i, int b) {
  return Serial.print(i, b);
}
size_t Logger::print(unsigned int i, int b) {
  return Serial.print(i, b);
}
size_t Logger::print(long l, int b) {
  return Serial.print(l, b);
}
size_t Logger::print(unsigned long l, int b) {
  return Serial.print(l, b);
}
size_t Logger::print(double d, int b) {
  return Serial.print(d, b);
}
size_t Logger::print(const Printable& x) {
  return Serial.print(x);
}

size_t  Logger::printf(char *str, ...) {
  int i, count=0, j=0, flag=0;
    char temp[ARDBUFFER+1];
    for(i=0; str[i]!='\0';i++)  if(str[i]=='%')  count++;

    va_list argv;
    va_start(argv, count);
    for(i=0,j=0; str[i]!='\0';i++)
    {
      if(str[i]=='%')
      {
        temp[j] = '\0';
        Serial.print(temp);
        j=0;
        temp[0] = '\0';

        switch(str[++i])
        {
          case 'd': Serial.print(va_arg(argv, int));
                    break;
          case 'l': Serial.print(va_arg(argv, long));
                    break;
          case 'f': Serial.print(va_arg(argv, double));
                    break;
          case 'c': Serial.print((char)va_arg(argv, int));
                    break;
          case 's': Serial.print(va_arg(argv, char *));
                    break;
          default:  ;
        };
      }
      else
      {
        temp[j] = str[i];
        j = (j+1)%ARDBUFFER;
        if(j==0)
        {
          temp[ARDBUFFER] = '\0';
          Serial.print(temp);
          temp[0]='\0';
        }
      }
    };
    return count;
}
void Logger::end() {
  Serial.println("");
}
size_t Logger::println(const __FlashStringHelper *ifsh){
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(ifsh);
}
size_t Logger::println(const String &str) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(str);
}
size_t Logger::println(const char str[]) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(str);
}
size_t Logger::println(char c) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(c);
}
size_t Logger::println(unsigned char c, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(c, b);
}
size_t Logger::println(int i, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(i, b);
}
size_t Logger::println(unsigned int i, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(i, b);
}
size_t Logger::println(long l, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(l, b);
}
size_t Logger::println(unsigned long l, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(l, b);
}
size_t Logger::println(double d, int b) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(d, b);
}
size_t Logger::println(const Printable& x) {
  Serial.print(_header);
  Serial.print(" ");
  return Serial.println(x);
}
size_t  Logger::printfln(char *str, ...) {
  Serial.print(_header);
  Serial.print(" ");
  size_t s = printf(str);
  Serial.println("");
  return ++s;
}
#undef ARDBUFFER
