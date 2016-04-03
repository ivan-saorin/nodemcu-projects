#include <Logger.h>
#include <Wire.h>
#include <pins.h>
#include <Zeroconf.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
#include "WeatherStationTypes.h"
#include "FS.h"



#define DHTPIN D10     // what digital pin we're connected to

#define OSS 1 -- oversampling setting (0-3)
#define SDA_PIN D4  // sda pin
#define SCL_PIN D5 // scl pin

Zeroconf zeroConf("/config.json");

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define READ_TIMEOUT 5000

DHT dht(DHTPIN, DHTTYPE);

Logger logger("*WeatherStation:");

const int led = D0;

// Variables

float_value _pressure = {"", "pressure", 0.0, "hPa"};
float_value _temperature_c = {"", "temperature_c", 0.0, "*C"};
float_value _temperature_f = {"", "temperature_f", 0.0, "*F"};
float_value _heat_index_c = {"", "heat_index_c", 0.0, "*C"};
float_value _heat_index_f = {":", "heat_index_f", 0.0, "*F"};
long_value _humidity = {"", "humidity", 0, "%"};

sensor_data_type sensor_data = {_pressure, _temperature_c, _temperature_f, _heat_index_c, _heat_index_f, _humidity};
long lastReadingTime = 0;

// Utilities


void serverSend(int status, String contentType, String msg) {
  ESP8266WebServer* server = zeroConf.getWebServer();
  server->sendHeader("Connection", "close");
  server->sendHeader("Access-Control-Allow-Origin", "*");
  server->send(status, contentType, msg);
}

// Program
void handleRoot() {
  logger.println(F("Handle root!"));
  ESP8266WebServer* server = zeroConf.getWebServer();
  // Just serve the index page from SPIFFS when asked for
  File index = SPIFFS.open("/index.html", "r");
  logger.begin();
  logger.print(F("File handler: "));
  logger.print(index);
  logger.end();
  server->streamFile(index, "text/html");
  index.close();
}

void versionHandler() {
  String message = "ESP8266 Server \r\nversion:";
  message += zeroConf.getVersion();
  serverSend(200, "text/plain", message);
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
}

void error404handler() {
  textError404handler();
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
}

void htmlSensorHandler() {
  handleRoot();
}

void getData() {
  logger.begin();
  logger.print(F("Free heap:"));
  logger.print(ESP.getFreeHeap(), DEC);
  logger.end();


  sensor_data.temperature_c.value = 0;
  sensor_data.temperature_f.value = 0;
  sensor_data.heat_index_c.value = 0;
  sensor_data.heat_index_f.value = 0;
  sensor_data.humidity.value = 0;

  sensor_data.humidity.value = dht.readHumidity();
  // Read temperature as Celsius (the default)
  sensor_data.temperature_c.value = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  sensor_data.temperature_f.value = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(sensor_data.humidity.value) || isnan(sensor_data.temperature_c.value) || isnan(sensor_data.temperature_f.value)) {
    logger.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  sensor_data.heat_index_f.value = dht.computeHeatIndex(sensor_data.heat_index_f.value, sensor_data.humidity.value);
  // Compute heat index in Celsius (isFahreheit = false)
  sensor_data.heat_index_c.value = dht.computeHeatIndex(sensor_data.temperature_c.value, sensor_data.humidity.value, false);

  Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(12345);
  bmp.begin();  

  /* Get a new sensor event */
  sensors_event_t event;
  bmp.getEvent(&event);

  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
//    sensor_data.pressure.value = event.pressure;
  }

  bmp = NULL;

}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  logger.print(F("WiFi.softAPIP: "));
  logger.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  logger.print(("ConfigPortalSSID: "));
  logger.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
}

void setup(void){
  Serial.begin(115200);
  
  logger.println(F("setup()"));
  //set led pin as output
  
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  /*
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);
  */
  
  Wire.begin(SDA_PIN, SCL_PIN);
  
  zeroConf.setAPCallback(configModeCallback);
  zeroConf.autoConnect("WeatherStation", 80);

  ESP8266WebServer* server = zeroConf.getWebServer();
  server->onNotFound(error404handler);
  
  zeroConf.on("/", htmlSensorHandler);
  
  zeroConf.on("/index.html", htmlSensorHandler);
  zeroConf.advertise("$run", "/index.html");

  //zeroConf.advertise("config", "/config.html");
  
//  zeroConf.on("/sensor.json", jsonSensorsHandler);
//  zeroConf.on("/sensors.txt", textSensorsHandler);

  //zeroConf.on("/version", versionHandler);

  zeroConf.begin();

  logger.println(F("HTTP server started"));
  
  //ticker.detach();
}

void loop(void){
  zeroConf.refresh();
  
  if (millis() - lastReadingTime > READ_TIMEOUT) {
    // if there's a reading ready, read it:
    // don't do anything until the data ready pin is high:
    //if (digitalRead(dataReadyPin) == HIGH) {
      //getData();
      // timestamp the last time you got a reading:
      lastReadingTime = millis();
      logger.println(F("lastReadingTime:"), lastReadingTime);
    //}
  }
}

