#include "Logger.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include "DHT.h"
#include "BasicWeatherServerTypes.h"

//Attention: Connect a 10K resistor from DHT pin 2 (data) to pin 1 (power) of the sensor

// ESP8266 ro Arduino GPIO number
#define D0  16
//#define D1  5
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

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(12345);

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

Logger logger("*WeatherStation:");

DHT dht(DHTPIN, DHTTYPE);

#define READ_TIMEOUT 5000

const char* server_version = "1.0.1";

const char* ssid = "IvanWireless";
const char* password = "unbreakable";

MDNSResponder mdns;

ESP8266WebServer server(80);

const int led = D0;

// Variables

float_value _pressure = {"Pressure:", "pressure", 0.0, "hPa"};
float_value _temperature_c = {"Temperature:", "temperature_c", 0.0, "*C"};
float_value _temperature_f = {"Temperature:", "temperature_f", 0.0, "*F"};
float_value _heat_index_c = {"Heat index:", "heat_index_c", 0.0, "*C"};
float_value _heat_index_f = {"Heat index:", "heat_index_f", 0.0, "*F"};
long_value _humidity = {"Humidity:", "humidity", 0, "%"};

sensor_data_type sensor_data = {_pressure, _temperature_c, _temperature_f, _heat_index_c, _heat_index_f, _humidity};

long lastReadingTime = 0;

// Utilities

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

String textLine(float_value v1, float_value v2) {
  String message = "";
  message += v1.label;
  message += " \t\t";
  message += v1.value;  
  message += " ";
  message += v1.UoM;  
  message += "\t\t";  
  message += v2.value;  
  message += " ";
  message += v2.UoM;
  message += "\r\n\r\n";
  
  return message;  
}

void textSensorsHandler() {
  digitalWrite(led, LOW);

  String message = "Weather Station report: \r\n\r\n\r\n";
  message += sensor_data.humidity.label;
  message += " \t\t";
  message += sensor_data.humidity.value;  
  message += " ";
  message += sensor_data.humidity.UoM;  
  message += "\r\n\r\n";

  message += textLine(sensor_data.temperature_c, sensor_data.temperature_f);
  message += textLine(sensor_data.heat_index_c, sensor_data.heat_index_f);

  message += sensor_data.pressure.label;
  message += " \t\t";
  message += sensor_data.pressure.value;  
  message += " ";
  message += sensor_data.pressure.UoM;  
 
  serverSend(200, "text/plain", message);
  digitalWrite(led, HIGH);
}

String jsonLine(float_value v) {
  String message = "";
  message += "\"";
  message += v.field;
  message += "\":{\"value\":";
  message += v.value;  
  message += ",\"UoM\":\"";
  message += v.UoM;  
  message += "\"},";
  
  return message;  
}

void jsonSensorsHandler() {
  digitalWrite(led, LOW);

  String message = "{\"";
  message += sensor_data.humidity.field;
  message += "\":{\"value\":";
  message += sensor_data.humidity.value;  
  message += ",\"UoM\":\"";
  message += sensor_data.humidity.UoM;  
  message += "\"},";

  message += jsonLine(sensor_data.temperature_c);
  message += jsonLine(sensor_data.temperature_f);
  message += jsonLine(sensor_data.heat_index_c);
  message += jsonLine(sensor_data.heat_index_f);

  message += "\"";
  message += sensor_data.pressure.field;
  message += "\":{\"value\":";
  message += sensor_data.pressure.value;  
  message += ",\"UoM\":\"";
  message += sensor_data.pressure.UoM;  
  message += "\"}}";
  serverSend(200, "application/json", message);
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


void getData() {
  logger.begin();
  logger.print(F("Free heap:"));
  logger.print(ESP.getFreeHeap(), DEC);
  logger.end();
  /* Get a new sensor event */ 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    sensor_data.pressure.value = event.pressure;
  }
    
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    blinkLed(3, 200, 200);
    logger.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  sensor_data.temperature_c.value = t;
  sensor_data.temperature_f.value = f;
  sensor_data.heat_index_c.value = hic;
  sensor_data.heat_index_f.value = hif;
  sensor_data.humidity.value = h;

  blinkLed(3, 100, 100); 
}
 
void setup(void){
  Serial.begin(115200);

//  Serial.begin(9600);
  logger.println("DHTxx test!");

  dht.begin();

  Wire.begin(SDA_PIN, SCL_PIN);

  bmp.begin();

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  logger.println("");
  logger.print("Connected to ");
  logger.println(ssid);
  logger.print("IP address: ");
  logger.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    logger.println("MDNS responder started");
  }
  
  server.on("/", textSensorsHandler);

  server.on("/sensors.txt", textSensorsHandler);
  server.on("/sensors.json", jsonSensorsHandler);

  server.on("/version", versionHandler);

/*
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });
*/

  server.onNotFound(error404handler);
  
  server.begin();
  logger.println("HTTP server started");
  blinkLed(10, 50, 100);
}
 
void loop(void){
  if (millis() - lastReadingTime > READ_TIMEOUT) {
    // if there's a reading ready, read it:
    // don't do anything until the data ready pin is high:
    //if (digitalRead(dataReadyPin) == HIGH) {
      getData();
      // timestamp the last time you got a reading:
      lastReadingTime = millis();
    //}
  }
  
  server.handleClient();
} 
