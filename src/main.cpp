#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <numeric>
#include <vector>
#include "wifi-setup/wifi-setup.h"

std::vector<unsigned long> flashPattern = { 0 };
static auto blinkLedForConfiguration = []() { flashPattern = { 100, 900 }; };

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

static WifiSetup::Setup wifiSetup;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  String deviceType = "humidity-sensor";
  String deviceId = getDeviceId();

  Serial.println(">:Initializing LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println(">:Cannot initialize LittleFS, restarting...");
    ESP.reset();
    return;
  }

  wifiSetup.start();

  String hostname = wifiSetup.getHostname();
  if (hostname.isEmpty()) {
    hostname = deviceType + "-" + deviceId;
    wifiSetup.setHostname(hostname);
    Serial.printf(">:Hostname not found. Defaulting to '%s'.\n", hostname.c_str());
  }

  Serial.println("\n---------------------------------------");
  Serial.println("IoT sensor");
  Serial.printf("Device id: %s\n", deviceId.c_str());
  Serial.printf("Hostname: %s\n", hostname.c_str());
  Serial.println("---------------------------------------");

  WifiSetup::WifiSetupOpts opts;
  opts.ssid = hostname;

  if (MDNS.begin(hostname)) { Serial.println(">:MDNS responder started"); }

  EEPROM.begin(sizeof(bool));
  bool shouldStartInSetupMode;
  EEPROM.get(0, shouldStartInSetupMode);

  if (shouldStartInSetupMode) {
    EEPROM.put(0, false);
    EEPROM.commit();
    Serial.println(">:Forcing to start in setup mode...");
    wifiSetup.launchConfiguration(opts);
    blinkLedForConfiguration();
  }
  else {
    if (!wifiSetup.tryConnect()) {
      Serial.println(">:Cannot connect to any wifi. Starting in setup mode...");
      wifiSetup.launchConfiguration(opts);
      blinkLedForConfiguration();
    }
    else {
      pinMode(0, INPUT_PULLUP);
    }
  }
}

void loop() {
  MDNS.update();
  
  if (!digitalRead(0)) {
    Serial.println(">:Flash button pressed, restarting in configuration mode...");
    EEPROM.put(0, true);
    EEPROM.commit();
    EEPROM.end();
    ESP.reset();
  }

  auto totalFlashDuration = std::reduce(flashPattern.begin(), flashPattern.end());
  if (!totalFlashDuration) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    auto elapsed = millis() % totalFlashDuration;
    auto i = 0;
    auto n = 0UL;

    for (auto step : flashPattern) {
      n += step;
      if (elapsed <= n) {
        digitalWrite(LED_BUILTIN, (i & 1) ? HIGH : LOW);
        break;
      }
      ++i;
    }
  }
}
