#include <Logger.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <WiFiClient.h>
                                  
//for LED status
#include <Ticker.h>
Ticker ticker;

#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <pins.h>

#include "Zeroconf.h"

#include "rgbSensorTypes.h"

#include "FS.h"

#define DHTPIN D10     // what digital pin we're connected to

#define OSS 1 -- oversampling setting (0-3)
#define SDA_PIN D4  // sda pin
#define SCL_PIN D5 // scl pin

const char* server_version = "1.0.0";

//Color Capture flags
char capturedColorRGB[12];

Zeroconf zeroConf("Wireless RGB Sensor", "Ivan Saorin", "ISCLS001", "1.0.5");
// multicast DNS responder

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

const int led = D0;
const int sensor_led = D1;

Logger logger("*rgbsesor.v2:");

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

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
  ESP8266WebServer* server = zeroConf.getWebServer();
  server->sendHeader("Connection", "close");
  server->sendHeader("Access-Control-Allow-Origin", "*");
  server->send(status, contentType, msg);
}

// Program

void handleRoot() {
  logger.println("Handle root!");
  ESP8266WebServer* server = zeroConf.getWebServer();
  // Just serve the index page from SPIFFS when asked for
  //File index = SPIFFS.open("/index.html", "r");
  File index = SPIFFS.open("/index.html", "r");
  logger.begin();
  logger.print("File handler: ");
  logger.print(index);
  logger.end();
  server->streamFile(index, "text/html");
  index.close();
}

void versionHandler() {
  digitalWrite(led, LOW);
  String message = "ESP8266 Server \r\nversion:";
  message += server_version;
  serverSend(200, "text/plain", message);
  digitalWrite(led, HIGH);
}

void textError404handler(){
  ESP8266WebServer* server = zeroConf.getWebServer();
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i=0; i<server->args(); i++){
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
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

  SPIFFS.begin();
  
  sensor_data_type sensor_data;
  memset(capturedColorRGB,0,16);
 
  delay(mt);  // takes 50ms to read 
  
  colorSensor.getRawData(&sensor_data.red, &sensor_data.green, &sensor_data.blue, &sensor_data.clear);

  sensor_data.color_temperature = colorSensor.calculateColorTemperature(sensor_data.red, sensor_data.green, sensor_data.blue);
  sensor_data.lux= colorSensor.calculateLux(sensor_data.red, sensor_data.green, sensor_data.blue);

  sprintf(capturedColorRGB,"%d,%d,%d-%d", (int)sensor_data.red, (int)sensor_data.green, (int)sensor_data.blue, (int)sensor_data.clear) ;
  logger.print("RAW capturedColorRGB: ");
  logger.println(capturedColorRGB);

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
  logger.print("capturedColorRGB: ");
  logger.println(capturedColorRGB);

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
  handleRoot();
  digitalWrite(led, HIGH);
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  logger.print("WiFi.softAPIP: ");
  logger.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  logger.print("ConfigPortalSSID: ");
  logger.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(sensor_led, OUTPUT);
  digitalWrite(sensor_led, LOW);

  zeroConf.setAPCallback(configModeCallback);
  zeroConf.autoConnect("rgbsensor", 80);

  colorSensor.begin();
  sensor_data_type sensor_data = captureColor();  //do it to turn off the LED and as a sanity check
  
 //  colorSensor.setInterrupt(true);  // turn off LED  

  ESP8266WebServer* server = zeroConf.getWebServer();
  server->onNotFound(error404handler);
  
  zeroConf.on("/", htmlSensorHandler);

  zeroConf.on("/index.html", htmlSensorHandler);
  zeroConf.on("/sensor.json", jsonSensorsHandler);

  zeroConf.on("/version", versionHandler);

  zeroConf.begin();
  logger.println("HTTP server started");
  
  ticker.detach();
  digitalWrite(BUILTIN_LED, HIGH);
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
    logger.println("WARN msg was truncated to length 255");
  }
  msg.toCharArray(buffer, 255);
}

void loop() {
  // put your main code here, to run repeatedly:
  zeroConf.refresh();
}
