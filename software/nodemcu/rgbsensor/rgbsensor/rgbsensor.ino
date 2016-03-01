#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "rgbSensorTypes.h"

// ESP8266 ro Arduino GPIO number
#define D0  16
#define D1  5
//#define D2  4
//#define D3  0
#define D4  2
#define D5  14
//#define D6  12
//#define D7  13
//#define D8  15
//#define D9  3
#define D10 1
//#define D11 9
//#define D12 10

#define DHTPIN D10     // what digital pin we're connected to

#define OSS 1 -- oversampling setting (0-3)
#define SDA_PIN D4  // sda pin
#define SCL_PIN D5 // scl pin


const char* server_version = "1.0.0";

const char* ssid = "IvanWireless";
const char* password = "unbreakable";

const char* iotfId = "iotfId:test";

int status = WL_IDLE_STATUS;

unsigned int localPort = 2390;      // local port to listen for UDP packets

byte packetBuffer[512]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

//Color Capture flags

char capturedColorRGB[12];

char myIp[16];

MDNSResponder mdns;

/*
INTEGRATIONTIME_2MS=0xFF   --<  2.4ms - 1 cycle    - Max Count: 1024  
INTEGRATIONTIME_24MS=0xF6   --<  24ms  - 10 cycles  - Max Count: 10240 
INTEGRATIONTIME_50MS=0xEB   --<  50ms  - 20 cycles  - Max Count: 20480 
INTEGRATIONTIME_101MS=0xD5   --<  101ms - 42 cycles  - Max Count: 43008 
INTEGRATIONTIME_154MS=0xC0   --<  154ms - 64 cycles  - Max Count: 65535 
INTEGRATIONTIME_700MS=0x00   --<  700ms - 256 cycles - Max Count: 65535 

GAIN_1X=0x00   --<  No gain  
GAIN_4X=0x01   --<  2x gain  
GAIN_16X=0x02   --<  16x gain 
GAIN_60X=0x03   --<  60x gain 
*/

const int TIME_2MS=3;
const int TIME_24MS=30;
const int TIME_50MS=55;
const int TIME_101MS=110;
const int TIME_154MS=160;
const int TIME_700MS=710;

const int MV_2MS=1024;
const int MV_24MS=10240;
const int MV_50MS=20480;
const int MV_101MS=4308;
const int MV_154MS=65535;
const int MV_700MS=65535;

Adafruit_TCS34725 colorSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_4X);
int mt = TIME_154MS;
int mv = MV_154MS;

ESP8266WebServer server(80);

const int led = D0;
const int sensor_led = D1;

void blinkLedOnce(int onTime, int offTime) {
  digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(onTime);              // wait for a onTime ms
  digitalWrite(led, HIGH);    // turn the LED off by making the voltage LOW
  delay(offTime);              // wait for a offTime ms
}

void blinkLed(int times, int onTime, int offTime) {
  for (int i = 1; i < times; i++) {
    blinkLedOnce(onTime, offTime);
  }
}

void serverSend(int status, String contentType, String msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(status, contentType, msg);
}

// Program
void versionHandler() {
  digitalWrite(led, LOW);
  String message = "ESP8266 Server \r\nversion:";
  message += server_version;
  serverSend(200, "text/plain", message);
  digitalWrite(led, HIGH);
}

void textError404handler(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  serverSend(404, "text/plain", message);
  blinkLed(3, 200, 200);
}

void error404handler() {
  textError404handler();
}

String jsonLineUInt(String field, uint16_t value) {
  String message = "";
  message += "\"";
  message += field;
  message += "\":";
  message += value;  
  
  return message;  
}

String jsonLineString(String field, String value) {
  String message = "";
  message += "\"";
  message += field;
  message += "\":\"";
  message += value;  
  message += "\"";
  
  return message;  
}

sensor_data_type captureColor()
{  
  digitalWrite(sensor_led, HIGH);   // turn the LED on (HIGH is the voltage level)
  sensor_data_type sensor_data;
  memset(capturedColorRGB,0,16);
 
  delay(mt);  // takes 50ms to read 
  
  colorSensor.getRawData(&sensor_data.red, &sensor_data.green, &sensor_data.blue, &sensor_data.clear);

  sensor_data.color_temperature = colorSensor.calculateColorTemperature(sensor_data.red, sensor_data.green, sensor_data.blue);
  sensor_data.lux= colorSensor.calculateLux(sensor_data.red, sensor_data.green, sensor_data.blue);

  sprintf(capturedColorRGB,"%d,%d,%d-%d", (int)sensor_data.red, (int)sensor_data.green, (int)sensor_data.blue, (int)sensor_data.clear) ;
  Serial.print("RAW capturedColorRGB: ");
  Serial.println(capturedColorRGB);

  colorSensor.setInterrupt(true);  // turn off LED
  
  // Figure out some basic hex code for visualization
  uint32_t sum = sensor_data.red + sensor_data.green + sensor_data.blue;

  float r, g, b, c;
  r = sensor_data.red ;
  g = sensor_data.green;
  b = sensor_data.blue;
  c = sensor_data.clear;

  
  r *= 255; r /= c;
  g *= 255; g /= c; 
  b *= 255; b /= c; 

  //hexColor = hex(color(wRed, wGreen, wBlue), 6);

  g *= 2;
  b *= 3;
 
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;

  
  

  sensor_data.red = r;
  sensor_data.green = g;
  sensor_data.blue = b;
  
  memset(capturedColorRGB,0,16);
  sprintf(capturedColorRGB,"%d,%d,%d", (int)r ,(int)g ,(int)b);
  Serial.print("capturedColorRGB: ");
  Serial.println(capturedColorRGB);

  digitalWrite(sensor_led, LOW);   // turn the LED on (HIGH is the voltage level)
  return sensor_data;
}

void jsonSensorsHandler() {
  digitalWrite(led, LOW);
  sensor_data_type sensor_data = captureColor();
  String message = "{";
  message += jsonLineString("server", "rgb-sensor-server");
  message += ",";
  message += jsonLineString("version", server_version);
  message += ",";
  message += jsonLineUInt("red", sensor_data.red);
  message += ",";
  message += jsonLineUInt("green", sensor_data.green);
  message += ",";
  message += jsonLineUInt("blue", sensor_data.blue);
  message += ",";
  message += jsonLineUInt("temperature", sensor_data.color_temperature);
  message += ",";
  message += jsonLineUInt("lux", sensor_data.lux);
  message += "}";

  serverSend(200, "application/json", message);
  digitalWrite(led, HIGH);
}

void htmlSensorHandler() {
  digitalWrite(led, LOW);
  String message =  "<html><head><title>RGB Sensor</title><script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.4.3/jquery.min.js\"></script>";
  message += "<script>";
  message += "function rgb2hex(rgb) {";
  message += "var rgb = Array.apply(null, arguments).join().match(/\\d+/g);";
  message += "rgb = ((rgb[0] << 16) + (rgb[1] << 8) + (+rgb[2])).toString(16);";
  // for (var i = rgb.length; i++ < 6;) rgb = '0' + rgb;
  message += "return rgb;";
  message += "};"  ;
  message += "function updateColor() {";
  message += "$.getJSON(\"http://192.168.1.162/sensor.json\", function( resp ) {";
  message += "$(\"body\").css(\"background-color\", \"rgb(\" + resp.red + \", \" + resp.green + \", \" + resp.blue + \")\");";
  message += "var color_name = \"#\" + rgb2hex(resp.red, resp.green, resp. blue);";  
  message += "$(\".color_name\").html(color_name);";  
  message += "$(\".color_value\").html(\"(\" + resp.red + \", \" + resp.green + \", \" + resp.blue + \")\");";  
  message += "setTimeout(updateColor, 3000);";  
  message += "});";
  message += "};";
  message += "function RelayClient(config, handler) {";
  message += "var connected = false;";
  message += "var connectHandler = handler;";
  message += "var socket = new WebSocket(config.relayURL);";
  message += "socket.onopen = function() {";
  message += "socket.send('open ' + config.remoteHost + ' ' + config.remotePort);";
  message += "};";
  message += "socket.onmessage = function(event) {";
  message += "if (!connected && event.data == 'connected') {";
  message += "connected = true;";
  message += "handler(socket);";
  message += "}}}"; 
  message += "$(function() {";
  message += "updateColor();";
  message += "});";
  message += "</script>";
  message += "<style>";
  message += "</style>";
  message += "</head><body><h1 class=\"color_name\"></h1><h1 class=\"color_value\"></h1></body></html>";

  serverSend(200, "text/html", message);
  digitalWrite(led, HIGH);
}

void printWifiStatus() {
// print the SSID of the network you're attached to:

  Serial.print("Connected to SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  byte oct1 = ip[0];
  byte oct2 = ip[1];
  byte oct3 = ip[2];
  byte oct4 = ip[3];
  sprintf(myIp, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);  

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Serial.begin(9600);
  Serial.println("iot server starting...");


  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(led, OUTPUT);
  pinMode(sensor_led, OUTPUT);
  digitalWrite(led, HIGH);
  digitalWrite(sensor_led, LOW);
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.print("Udp server started at port ");
  Serial.println(localPort);
  Udp.begin(localPort);
  colorSensor.begin();
  sensor_data_type sensor_data = captureColor();  //do it to turn off the LED and as a sanity check
  
 //  colorSensor.setInterrupt(true);  // turn off LED  
  
  server.on("/", htmlSensorHandler);

  server.on("/sensors.html", htmlSensorHandler);
  server.on("/sensor.json", jsonSensorsHandler);

  server.on("/version", versionHandler);

  server.onNotFound(error404handler);
  
  server.begin();
  Serial.println("HTTP server started");
  blinkLed(10, 50, 100);
}

String byteArrayToString(byte* byteArray, int noBytes) {
  char bfr[noBytes];
  strncpy(bfr, (char *)byteArray, noBytes);
  String myString = String(bfr);  
  
  return myString.substring(0,noBytes);
}

void stringToByteArray(String msg, char* buffer) {
  if (msg.length() > 255) {
    msg = msg.substring(0, 255);
    Serial.println("WARN msg was truncated to length 255");
  }
  msg.toCharArray(buffer, 255);
}

void loop() {
  String req = "";
  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    Serial.print(millis() / 1000);
    Serial.print(":Packet of ");
    Serial.print(noBytes);
    Serial.print(" received from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.println(Udp.remotePort());
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,noBytes); // read the packet into the buffer

    // display the packet contents in HEX
    for (int i=1;i<=noBytes;i++){
      Serial.print(packetBuffer[i-1],HEX);
      if (i % 32 == 0){
        Serial.println();
      }
      else Serial.print(' ');
    } // end for
    Serial.println();
    req += byteArrayToString(packetBuffer, noBytes);
    Serial.println(req);

      // send a reply, to the IP address and port that sent us the packet we received
    Serial.println("1");
    //String ip = new String(myIp);
    String response = "http://" + (String) myIp + "/";
    byte message[response.length()];
    for (int i = 0; i < response.length(); i++) {
      message[i] = response.charAt(i);
    }
    message[response.length()] = 0;
    
    //memset(replyBuffer,0,response.length()+1);
    //response.toCharArray(replyBuffer, response.length()-1);
    Serial.println("2");
    Serial.println("3");
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Serial.println("4");
    Udp.write(message, sizeof(message));
    Serial.println("5");
    Udp.endPacket(); 
    Serial.println("6"); 
  } // end if  

  server.handleClient();
}
