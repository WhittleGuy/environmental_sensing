/*
Author: Brandon Whittle
Date: 2022-1028
References: 
https://wouterdeschuyter.be/blog/sensor-readings-as-json-api-using-an-esp8266-nodemcu-dev-kit
https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
https://github.com/airgradienthq/arduino/blob/master/AirGradient.cpp
*/

#include <ctime>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"
#include "PMS.h"

// Unit information
#define ID 1

// Configuration
#define WIFI_SSID "no"
#define WIFI_PASS "no"
#define ONE_WIRE_BUS 4 //D2
#define HUMIDITY_PIN 2 //D4
#define DHT_TYPE 22
#define CO2_TX_PIN 15 // CO2 SoftwareSerial Tx 
#define CO2_RX_PIN 13 // CO2 SoftwareSerial Rx
#define PM_TX_PIN 12 // PM SoftwareSerial Tx
#define PM_RX_PIN 14 // PM SoftwareSerial Rx

SoftwareSerial Serial_CO2(CO2_RX_PIN, CO2_TX_PIN);
SoftwareSerial Serial_PMS(PM_RX_PIN, PM_TX_PIN);

// Setup Sensor Libraries //////////////////////////////////////////////////////
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dallas(&oneWire);
DHT dht(HUMIDITY_PIN, DHT_TYPE);
PMS pms(Serial_PMS);
PMS::DATA data;



// Web server //////////////////////////////////////////////////////////////////
ESP8266WebServer webServer(80);


// Main Program ////////////////////////////////////////////////////////////////
void setup() {
  setupSerial();
  setupWiFi();
  setupWebServer(); 
  setupSensors(); 
}

void loop() {
  webServer.handleClient();
}


// Webserver and Wifi Setup Functions //////////////////////////////////////////
void handleRoot() {
  Serial.println("[+] Request: /");

  // Get unit and sensor data
  float temperature_0 = readDallas();
  float temperature_1 = dht.readTemperature();
  float humidity = dht.readHumidity();
  int co2 = getCO2();
  readPMS();

  // Create json response
  String res = "";
  res += "{";
  res += "\"id\":";
  res += ID;
  res += ",\"temp_0\":";
  res += temperature_0;
  res += ",\"temp_1\":";
  res += temperature_1;
  res += ",\"hum\":";
  res += humidity;
  res += ",\"co2\":";
  res += co2;
  res += ",\"pm1.0\":";
   res += data.PM_AE_UG_1_0;
   res += ",\"pm2.5\":";
   res += data.PM_AE_UG_2_5;
   res += ",\"pm10.0\":";
   res += data.PM_AE_UG_10_0;
  res += "}";

  // Send response
  webServer.send(200, "application/json", res);
}

void setupWiFi() {
  Serial.print("[+] Configuring wifi..");  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[+] Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("[+] IP: ");
  Serial.println(WiFi.localIP());
}

void setupSerial() {
  pinMode(CO2_RX_PIN, INPUT);
  pinMode(CO2_TX_PIN, OUTPUT);
  pinMode(PM_RX_PIN, INPUT);
  pinMode(PM_TX_PIN, OUTPUT);
  Serial.begin(9600);
  Serial_CO2.begin(9600);
  Serial_PMS.begin(9600);
}

void setupWebServer() {
  Serial.println("[+] Configuring webserver...");
  webServer.on("/", handleRoot);
  Serial.println("[+] Starting server...");
  webServer.begin();
  Serial.println("[+] Server started successfully");
}

void setupSensors() {
  Serial.println("[+] Starting sensors...");
  dallas.begin();
  dht.begin();
  pms.passiveMode();
  pms.wakeUp();
  Serial.println("Warming up for 10 seconds...");
  //delay(10000);
  Serial.println("[+] Sensors enabled");
}

float readDallas() {
  dallas.requestTemperatures();
  return dallas.getTempCByIndex(0);
}

int getCO2() {
  while(Serial_CO2.available()) {
    Serial_CO2.read();
  }

  const byte CO2_cmd[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};
  byte CO2_res[] = {0,0,0,0,0,0,0};

  int datapos = -1;
  const int cmdSize = 7;

  int numberOfBytesWritten = Serial_CO2.write(CO2_cmd, cmdSize);
  if (numberOfBytesWritten != cmdSize) {
    return -2;
  }

  int timeoutCounter = 0;
  while (Serial_CO2.available() < cmdSize) {
    timeoutCounter++;
    if (timeoutCounter > 10) {
      return -3;
    }
    delay(50);
  }

  for (int i = 0; i < cmdSize; i++) {
    CO2_res[i] = Serial_CO2.read();
    if ((CO2_res[i] == 0xFE) && (datapos == -1)) {
      datapos = i;
    }
    Serial.print(CO2_res[i], HEX);
    Serial.print(":");
  }
  Serial.println();
  return (CO2_res[datapos+3]*256 + CO2_res[datapos+4]);
}

void readPMS() {
  pms.requestRead();
}
