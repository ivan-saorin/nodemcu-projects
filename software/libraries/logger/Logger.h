/**************************************************************
   logger is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

 #ifndef Logger_h
 #define Logger_h

 #include <stdio.h> // for size_t

 #include <WString.h>
 #include <Printable.h>

 #include <stdarg.h>
 #include <Arduino.h>

 class Logger
 {
   public:
     Logger();
     Logger(char const *header);

     void          begin();
     size_t        print(const __FlashStringHelper *);
     size_t        print(const String &);
     size_t        print(const char[]);
     size_t        print(char);
     size_t        print(unsigned char, int = DEC);
     size_t        print(int, int = DEC);
     size_t        print(unsigned int, int = DEC);
     size_t        print(long, int = DEC);
     size_t        print(unsigned long, int = DEC);
     size_t        print(double, int = 2);
     size_t        print(const Printable&);

     size_t        printf(char *str, ...);
     size_t        printf(String str, ...) {
        va_list argv;
        return printf((String) str, argv);
     }

     void          end();
     size_t        println(const __FlashStringHelper *);
     size_t        println(const String &);
     size_t        println(const char[]);
     size_t        println(char);
     size_t        println(unsigned char, int = DEC);
     size_t        println(int, int = DEC);
     size_t        println(unsigned int, int = DEC);
     size_t        println(long, int = DEC);
     size_t        println(unsigned long, int = DEC);
     size_t        println(double, int = 2);
     size_t        println(const Printable&);
     size_t        printfln(char *str, ...);
     size_t        printfln(String str, ...) {
        va_list argv;
        return printfln((String) str, argv);
     }

   private:
     const char* DEFAULT_HEADER              = "";

     const char* _header = "";
 };
 #endif
