/**************************************************************
   Zeroconf is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to
   Licensed under MIT license
 **************************************************************/

#include "Zeroconf.h";
#include "FS.h";
#include "ArduinoJson.h";

/*
Zeroconf::Zeroconf(const char* model, const char* vendor, const char* serial, const char* serverthis->_version) {
  this->_model = model;
  this->_vendor = vendor;
  this->_serial = serial;
  this->_serverthis->_version = serverthis->_version;
}
*/

Zeroconf::Zeroconf(const char* configFilename) {
  this->_fn = (String) configFilename;
}

void Zeroconf::printSPIFFS() {
  logger->println("dir /");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File f = dir.openFile("r");
    logger->begin();
    logger->print(dir.fileName());
    logger->print(", size: ");
    logger->print(f.size());
    logger->end();
  }
}

void Zeroconf::printConfigJson() {
  if (SPIFFS.exists(this->_fn)) {
    File cfg = SPIFFS.open(this->_fn, "r");
    String json = cfg.readString();
    logger->println("json", json);
    cfg.close();
  }
}

JsonObject& Zeroconf::readConfigJson() {
  if (SPIFFS.exists(this->_fn)) {
    File cfg = SPIFFS.open(this->_fn, "r");
    String json = cfg.readString();

    char cc[json.length()+1];
    json.toCharArray(cc, json.length());

    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& json_parsed = jsonBuffer.parseObject(cc);
    if (!json_parsed.success())
    {
      Serial.println("parseObject() failed");
      return json_parsed;
    }

    // Make the decision to turn off or on the LED
    //if (strcmp(json_parsed["light"], "on") == 0) {

    return json_parsed;
    cfg.close();
  }
  else {
    logger->println(this->_fn + " not found on the SPIFS file system... skipping.");
  }
}

void Zeroconf::autoConnect(const char* dns, int port) {
  SPIFFS.begin();

  printSPIFFS();

  logger->println("config:" + this->_fn);
  printConfigJson();

 if (SPIFFS.exists(this->_fn)) {
    File cfg = SPIFFS.open(this->_fn, "r");
    String json = cfg.readString();

    char cc[json.length()+1];
    json.toCharArray(cc, json.length());

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json_parsed = jsonBuffer.parseObject(cc);
    if (!json_parsed.success())
    {
      Serial.println("parseObject() failed");
      return;
    }

    // Make the decision to turn off or on the LED
    //if (strcmp(json_parsed["light"], "on") == 0) {

    this->_version = json_parsed["version"];
    logger->println("version", this->_version);
    this->_model = json_parsed["model"];
    logger->println("model", this->_model);
    this->_vendor = json_parsed["vendor"];
    logger->println("vendor", this->_vendor);
    this->_serial = json_parsed["serial"];
    logger->println("serial", this->_serial);
    this->_port = json_parsed["port"];
    logger->println("port", this->_port);

    cfg.close();
  }
  else {
    logger->println(this->_fn + " not found on the SPIFS file system... skipping.");
    return;
  }

  this->_dns = dns;
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

  logger->begin();
  logger->print("Connecting to: ");
  logger->print(dns);
  logger->end();
  if (mdns.begin(dns, WiFi.localIP())) {
    logger->begin();
    logger->print("mDNS responder started on: ");
    logger->print(dns);
    logger->print(".local");
    logger->end();

  }

  //if you get here you have connected to the WiFi
  printWifiStatus();

  this->_index = 0;
  mdns.addService("http", "tcp", atoi(this->_port));
  mdns.addServiceTxt("http", "tcp", "version", this->_version);
  mdns.addServiceTxt("http", "tcp", "model", this->_model);
  mdns.addServiceTxt("http", "tcp", "vendor", this->_vendor);
  mdns.addServiceTxt("http", "tcp", "serial", this->_serial);
  mdns.addServiceTxt("http", "tcp", "macthis->_address", (const char*) &MAC_char);
  mdns.addServiceTxt("http", "tcp", "ip_address", (const char*) &myIp);
  mdns.addServiceTxt("http", "tcp", "port", _port);

  ESP8266WebServer server(atoi(_port));
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

void Zeroconf::on(const char* uri, ESP8266WebServer::THandlerFunction handler) {
  server.on(uri, handler);
}

void Zeroconf::on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn) {
  server.on(uri, method, fn);
}

void Zeroconf::on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn, ESP8266WebServer::THandlerFunction ufn) {
  server.on(uri, method, fn, ufn);
}

void Zeroconf::advertise(const char* key, const char* uri) {

  //String k = (String) _key + (String) _separator + (String) _index;
  //String v = "http://" + (String) _dns + ".local" + (String) uri;
  String v = "http://" + (String) myIp + ":" + (String) DEFAULT_PORT + (String) uri;

  logger->println("mdns.addServiceTxt");
  logger->begin();
  logger->print(key);
  logger->print("=");
  logger->print(v);
  logger->end();

  mdns.addServiceTxt("http", "tcp", key, v);


 _index++;

}

void Zeroconf::iconPngHandler(void) {
  logger->println("Handle icon!");
  // Just serve the index page from SPIFFS when asked for
  if (SPIFFS.exists("/icon.png")) {
    File icon = SPIFFS.open("/icon.png", "r");
    logger->begin();
    logger->print("File handler: ");
    logger->print(icon);
    logger->end();
    server.streamFile(icon, "image/png");
    icon.close();
  }
  else {
    logger->println("/icon.png not found on the SPIFS file system... skipping.");
  }
}

void Zeroconf::heartbeatHandler(void) {

}

void Zeroconf::begin() {
  /*
  String k = (String) _key + (String) _separator + (String) _count_suffix;
  String v = (String) _index;

  logger->println("mdns.addServiceTxt");
  logger->begin();
  logger->print(k);
  logger->print("=");
  logger->print(v);
  logger->end();

  mdns.addServiceTxt("http", "tcp", k, v);
  */


  //_HTTP->onNotFound(); - See more at: http://www.esp8266.com/viewtopic.php?f=32&t=5825#sthash.mzdz8nv9.dpuf
  server.on("/icon.png", std::bind(&Zeroconf::iconPngHandler, this));
  this->advertise("icon", "/icon.png");

  server.on("/heartbeat", std::bind(&Zeroconf::heartbeatHandler, this));
  this->advertise("heartbeat", "/heartbeat");

  server.begin();
}

const char* Zeroconf::getDNS() {
  return _dns;
}
