#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include "wifi-config.h"

#define WIFI_CONFIG_ADDRESS 0

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\n---------------------------------------\nIoT sensor\n---------------------------------------");
  EEPROM.begin(getWifiConfigurationSize());

  if (MDNS.begin("humidity-sensor")) { Serial.println(">:MDNS responder started"); }

  Serial.println(">:Loading WiFi configuration...");
  WifiConfiguration wifiConfig = loadWifiConfiguration(WIFI_CONFIG_ADDRESS);
  
  if (!wifiConfig.valid) {
    Serial.println(">:WiFi configuration not found, starting in configuration mode...");
  }
  else {
    Serial.printf(">:WiFi configuration found. SSID=%s\n", wifiConfig.ssid);
  }
}

void loop() {
  server.handleClient();
  MDNS.update();
}
