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

//Color Capture flags

char capturedColorRGB[12];

MDNSResponder mdns;

Adafruit_TCS34725 colorSensor = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

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
  delay(50);  
  sensor_data_type sensor_data;

  memset(capturedColorRGB,0,12);
 
  delay(60);  // takes 50ms to read 
  
  colorSensor.getRawData(&sensor_data.red, &sensor_data.green, &sensor_data.blue, &sensor_data.clear);

  sensor_data.color_temperature = colorSensor.calculateColorTemperature(sensor_data.red, sensor_data.green, sensor_data.blue);
  sensor_data.lux= colorSensor.calculateLux(sensor_data.red, sensor_data.green, sensor_data.blue);
  
  colorSensor.setInterrupt(true);  // turn off LED
  
  // Figure out some basic hex code for visualization
  uint32_t sum = sensor_data.red + sensor_data.green + sensor_data.blue;
 
  float r, g, b;
  r = sensor_data.red; r /= sum;
  g = sensor_data.green; g /= sum;
  b = sensor_data.blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
   if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
  
  sprintf(capturedColorRGB,"%d,%d,%d", (int)r ,(int)g ,(int)b) ;
  Serial.print("capturedColorRGB: ");
  Serial.println(capturedColorRGB);

  delay(50);  
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
  String message =  "<html><head><title>RGB Sensor</title><script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.4.3/jquery.min.js\"></script><script>";
  message += "function updateColor() {";
  message += "$.getJSON(\"http://192.168.1.162/sensor.json\", function( resp ) {";
  message += "$(\"body\").css(\"background-color\", \"rgb(\" + resp.red + \", \" + resp.green + \", \" + resp.blue + \")\");";
  message += "setTimeout(updateColor, 3000);";  
  message += "});";
  message += "};";
  message += "$(function() {";
  message += "updateColor();";
  message += "});";
  message += "</script></head><body></body></html>";

  serverSend(200, "text/html", message);
  digitalWrite(led, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Serial.begin(9600);
  Serial.println("DHTxx test!");


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

void loop() {
  server.handleClient();
}
