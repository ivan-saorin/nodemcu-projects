/**************************************************************
   Zeroconf is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

 #ifndef Zeroconf_h
 #define Zeroconf_h

 #include <Arduino.h>
 #include <Logger.h>
 
 #include <ESP8266WiFi.h>
 #include <ESP8266WebServer.h>
 #include <DNSServer.h>
 #include <ESP8266mDNS.h>
 #include <WiFiManager.h>


 //const char HTTP_END[] PROGMEM = "</div></body></html>";

 //#define WIFI_MANAGER_MAX_PARAMS 10

 class Zeroconf
 {
   public:
     Zeroconf(const char* model, const char* vendor, const char* serial, const char* server_version);

     void          autoConnect(const char* dns, int port);

     void          printWifiStatus();

     //called when AP mode and config portal is started
     void          setAPCallback( void (*func)(WiFiManager* myWiFiManager) );

     void          refresh();

     ESP8266WebServer* getWebServer();
     void          begin();
     void          on(const char* uri, ESP8266WebServer::THandlerFunction handler);
     void          on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn);
     void          on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn, ESP8266WebServer::THandlerFunction ufn);

     const char*         getDNS();

   private:
     //std::unique_ptr<Logger> logger;
     Logger* logger = new Logger("Zeroconf:");
     WiFiManager wifiManager;
     ESP8266WebServer server;
     MDNSResponder mdns;

     const int DEFAULT_PORT               = 80;

     const char* _dns = "defaultdns";
     const char* _server_version = "1.0.0";
     const char* _model = "";
     const char* _vendor = "";
     const char* _serial = "";

     const char* _separator = "_";
     const char* _count_suffix = "number";
     const char* _key = "features";
     int _index = 0;
     
     char myIp[16];
     byte mac[6];
     char MAC_char[18];

     void                addUri(const char* uri);
 };

 #endif

