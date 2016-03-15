/**************************************************************
   Zeroconf is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

 #ifndef Zeroconf_h
 #define Zeroconf_h

 #include <Arduino.h>

 #include <ESP8266WiFi.h>
 #include <ESP8266WebServer.h>
 #include <DNSServer.h>
 #include <WiFiManager.h>


 const char HTTP_END[] PROGMEM = "</div></body></html>";

 #define WIFI_MANAGER_MAX_PARAMS 10

 class Zeroconf
 {
   public:
     Zeroconf();

     boolean       autoConnect();
     boolean       autoConnect(char const *apName, char const *apPassword = NULL);

     //if you want to always start the config portal, without trying to connect first
     boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

     // get the AP name of the config portal, so it can be used in the callback
     String        getConfigPortalSSID();

     void          resetSettings();

     //sets timeout before webserver loop ends and exits even if there has been no setup.
     //usefully for devices that failed to connect at some point and got stuck in a webserver loop
     //in seconds setConfigPortalTimeout is a new name for setTimeout
     void          setConfigPortalTimeout(unsigned long seconds);
     void          setTimeout(unsigned long seconds);

     //sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
     void          setConnectTimeout(unsigned long seconds);


     void          setDebugOutput(boolean debug);
     //defaults to not showing anything under 8% signal quality if called
     void          setMinimumSignalQuality(int quality = 8);
     //sets a custom ip /gateway /subnet configuration
     void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
     //sets config for a static IP
     void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
     //called when AP mode and config portal is started
     void          setAPCallback( void (*func)(Zeroconf*) );
     //called when settings have been changed and connection was successful
     void          setSaveConfigCallback( void (*func)(void) );
     //adds a custom parameter
     void          addParameter(ZeroconfParameter *p);
     //if this is set, it will exit after config, even if connection is unsucessful.
     void          setBreakAfterConfig(boolean shouldBreak);
     //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)

   private:
     std::unique_ptr<WiFiManager>        wifiManager;
     std::unique_ptr<MDNSResponder>      mdns;

     const int DEFAULT_PORT               = 80;
     const char* DEFAULT_DNS              = "defaultdns";
     const char* DEFAULT_SERVER_VERSION   = "1.0.0";

     char* dns = "";
     char* server_version = "";

     char myIp[16];
     byte mac[6];
     char MAC_char[18];

     //const int     WM_DONE                 = 0;
     //const int     WM_WAIT                 = 10;

     //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

     void          setupConfigPortal();
     void          startWPS();

     const char*   _apName                 = "no-net";
     const char*   _apPassword             = NULL;
     String        _ssid                   = "";
     String        _pass                   = "";
     unsigned long _configPortalTimeout    = 0;
     unsigned long _connectTimeout         = 0;
     unsigned long _configPortalStart      = 0;

     IPAddress     _ap_static_ip;
     IPAddress     _ap_static_gw;
     IPAddress     _ap_static_sn;
     IPAddress     _sta_static_ip;
     IPAddress     _sta_static_gw;
     IPAddress     _sta_static_sn;

     int           _paramsCount            = 0;
     int           _minimumQuality         = -1;
     boolean       _shouldBreakAfterConfig = false;
     boolean       _tryWPS                 = false;

     //String        getEEPROMString(int start, int len);
     //void          setEEPROMString(int start, int len, String string);

     int           status = WL_IDLE_STATUS;
     int           connectWifi(String ssid, String pass);
     uint8_t       waitForConnectResult();

     void          handleRoot();
     void          handleWifi(boolean scan);
     void          handleWifiSave();
     void          handleInfo();
     void          handleReset();
     void          handleNotFound();
     void          handle204();
     boolean       captivePortal();

 };

 #endif
