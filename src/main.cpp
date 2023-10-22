#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "wifi-setup/wifi-setup.h"

static String getDeviceId() {
  char deviceId[8];
  const auto mac = WiFi.macAddress().c_str();
  auto idx = 0;
  for (auto i : { 16, 15, 13, 12, 10, 9, 7}) {
    deviceId[idx++] = tolower(mac[i]);
  }
  deviceId[idx] = 0;
  return deviceId;
}

void setup() {
  String deviceType = "humidity-sensor";
  String deviceId = getDeviceId();

  Serial.begin(115200);
  Serial.println("\n---------------------------------------");
  Serial.println("IoT sensor");
  Serial.printf("Device id: %s\n", deviceId.c_str());
  Serial.println("---------------------------------------");

  Serial.println(">:Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println(">:Cannot initialize LittleFS, restarting...");
    ESP.reset();
    return;
  }

  String mdnsId = deviceType + "-" + deviceId;
  if (MDNS.begin(mdnsId)) { Serial.println(">:MDNS responder started"); }

  WifiSetup::WifiSetupOpts opts;
  opts.ssid = deviceType + "-" + deviceId;

  auto* wifiSetup = new WifiSetup::Setup(opts);
  wifiSetup->start();
}

void loop() {
  MDNS.update();
}
