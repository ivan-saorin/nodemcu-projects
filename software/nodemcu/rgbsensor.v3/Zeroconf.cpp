/**************************************************************
   Zeroconf is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

#include "Zeroconf.h";

Zeroconf::Zeroconf(const char* model, const char* vendor, const char* serial, const char* server_version) {
  _model = model;
  _vendor = vendor;
  _serial = serial;
  _server_version = server_version;
}

void Zeroconf::autoConnect(const char* dns, int port) {
  _dns = dns;
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    logger->println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  if (mdns.begin(dns, WiFi.localIP())) {
    logger->begin();
    logger->print("mDNS responder started on: ");
    logger->print(dns);
    logger->print(".local");
    logger->end();
    
  }

  //if you get here you have connected to the WiFi
  printWifiStatus();  

  _index = 0;
  mdns.addService("http", "tcp", DEFAULT_PORT);
  mdns.addServiceTxt("http", "tcp", "version", _server_version);    
  mdns.addServiceTxt("http", "tcp", "model", _model);    
  mdns.addServiceTxt("http", "tcp", "vendor", _vendor);      
  mdns.addServiceTxt("http", "tcp", "serial", _serial);
  mdns.addServiceTxt("http", "tcp", "mac_address", (const char*) &MAC_char);

  ESP8266WebServer server(port);
}

void Zeroconf::printWifiStatus() {
  logger->println("Connected to SSID: " + WiFi.SSID());

  // print your WiFi shield's MAC address:
  WiFi.macAddress(mac);
  int sm = sizeof(mac);
  const char* e = "%s%02x:";
  for (int i = 0; i < sm; ++i){
    if (i == (sm-1)) {
      e = "%s%02x";
    }
    sprintf(MAC_char, e, MAC_char, mac[i]);
  }
  logger->begin();
  logger->print("MAC Address: ");
  logger->print(MAC_char); //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3587#sthash.uwW0kzel.dpuf  
  logger->end();
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  logger->begin();
  logger->print("IP Address: ");
  logger->print(ip);
  logger->end();
  
  byte oct1 = ip[0];
  byte oct2 = ip[1];
  byte oct3 = ip[2];
  byte oct4 = ip[3];
  sprintf(myIp, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);  

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  logger->begin();
  logger->print("RSSI: ");
  logger->print(rssi);
  logger->print(" dBm");
  logger->end();
}

void Zeroconf::setAPCallback( void (*func)(WiFiManager* myWiFiManager) ) {
  wifiManager.setAPCallback(func);
}

void Zeroconf::refresh(void) {
  mdns.update();
  server.handleClient();  
}

ESP8266WebServer* Zeroconf::getWebServer() {
  return &server;
}

void Zeroconf::addUri(const char* uri) {

  String k = (String) _key + (String) _separator + (String) _index;
  String v = "http://" + (String) _dns + ".local" + (String) uri;
  
  logger->println("mdns.addServiceTxt");
  logger->begin();
  logger->print(k);
  logger->print("=");
  logger->print(v);
  logger->end();

  mdns.addServiceTxt("http", "tcp", k, v);


 _index++;

}

void Zeroconf::on(const char* uri, ESP8266WebServer::THandlerFunction handler) {
  addUri(uri);
  server.on(uri, handler);
}

void Zeroconf::on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn) {
  addUri(uri);
  server.on(uri, method, fn);
}

void Zeroconf::on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn, ESP8266WebServer::THandlerFunction ufn) {
  addUri(uri);
  server.on(uri, method, fn, ufn);
}

void Zeroconf::begin() {
  String k = (String) _key + (String) _separator + (String) _count_suffix;
  String v = (String) _index;
  
  logger->println("mdns.addServiceTxt");
  logger->begin();
  logger->print(k);
  logger->print("=");
  logger->print(v);
  logger->end();

  mdns.addServiceTxt("http", "tcp", k, v);
  
  server.begin();
}

const char* Zeroconf::getDNS() {
  return _dns;
}
