#include "EmonLib.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_PIN 5

ESP8266WebServer server(80);

void handleNotFound();
void handlePower();
void hanldeTemperature();

const char* SSID = "WoodHouse";
const char* SSID_PASSWORD = "casarodriguezsanchezreyman";

const char* serverAddress = "http://192.168.1.16:2000/main_voltage";

IPAddress ip(192, 168, 1, 32);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


EnergyMonitor energyMonitor;

OneWire oneWire(TEMP_PIN);
DallasTemperature DS18B20(&oneWire);

// Voltaje de nuestra red eléctrica
char temperatureCString[7];

int voltage;
double Irms;


void setup()

{
  StaticJsonBuffer<200> jsonBuffer;
  Serial.begin(9600);
  
  energyMonitor.current(0, 81);
  
 
  delay(10);
  DS18B20.begin();
  DS18B20.setResolution(10);
  WiFi.config(ip, gateway, subnet, gateway);
  WiFi.begin(SSID, SSID_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("Ip address: \t");
    Serial.println(WiFi.localIP());
    
    
    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin(serverAddress);  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode > 0) { //Check the returning code

      String payload = http.getString();   //Get the request response payload
      JsonObject& root = jsonBuffer.parseObject(payload);
      voltage = root["result"];
      Serial.println(voltage);
      //Serial.println("Voltage -> " + voltage);

    }

  }
  server.on("/power", HTTP_GET, handlePower);
  server.on("/temperature", HTTP_GET, handleTemperature);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server started");



}

void loop() {
  server.handleClient();
}


void getTemperature() {
  float tempC;
  do{
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    dtostrf(tempC, 2, 2, temperatureCString);
  } while (tempC == 85.0 || tempC == (-127.0));
}

void handlePower() {
  server.send(200, "text/plain", String(energyMonitor.calcIrms(2048), 2));
}
void handleTemperature() {
  getTemperature();
  Serial.println(temperatureCString);
  server.send(200, "text/plain", temperatureCString);
}
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}
