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

// Unit information
#define ID 2

// Configuration
#define WIFI_SSID "no"
#define WIFI_PASS "no"
#define CO2_TX_PIN 15 // CO2 SoftwareSerial Tx 
#define CO2_RX_PIN 13 // CO2 SoftwareSerial Rx

SoftwareSerial Serial_CO2(CO2_RX_PIN, CO2_TX_PIN);

// Web server //////////////////////////////////////////////////////////////////
ESP8266WebServer webServer(80);

// Main Program ////////////////////////////////////////////////////////////////
void setup() {
  setupSerial();
  setupWiFi();
  setupWebServer(); 
}

void loop() {
  webServer.handleClient();
}


// Webserver and Wifi Setup Functions //////////////////////////////////////////
void handleRoot() {
  Serial.println("[+] Request: /");

  // Get unit and sensor data
  int co2 = getCO2();

  // Create json response
  String res = "";
  res += "{";
  res += "\"id\":";
  res += ID;
  res += ",\"co2\":";
  res += co2;
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
  Serial.begin(9600);
  Serial_CO2.begin(9600);
}

void setupWebServer() {
  Serial.println("[+] Configuring webserver...");
  webServer.on("/", handleRoot);
  Serial.println("[+] Starting server...");
  webServer.begin();
  Serial.println("[+] Server started successfully");
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
