#include <ArduinoJson.h>
#include <LittleFS.h>
#include "wifi-data-manager.h"

using namespace WifiSetup;

static void loadWifiData(const String& dataPath, WifiList* v) {
  auto file = LittleFS.open(dataPath, "r");
  if (file) {
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, file);
    file.close();

    v->clear();
    for (const auto wifi : doc["ssids"].as<JsonArray>()) {
      v->push_back({ wifi["ssid"], wifi["psk"] });
    }
  }

  Serial.printf(">:Loading %d SSIDs from %s...\n", v->size(), dataPath.c_str());
}

static void saveWifiData(const String& dataPath, const WifiList& v) {
  auto file = LittleFS.open(dataPath, "w");
  StaticJsonDocument<1024> doc;
  JsonArray array = doc.createNestedArray("ssids");

  for (const auto wifi : v) {
    auto obj = array.createNestedObject();
    obj["ssid"] = wifi.ssid;
    obj["psk"] = wifi.psk;
  }

  serializeJson(doc, file);
  file.close();

  Serial.printf(">:Saving %d SSIDs to %s...\n", v.size(), dataPath.c_str());
}

void WifiDataManager::start(const String& dataPath) {
  this->dataPath = dataPath;
  loadWifiData(dataPath, &list);
}

void WifiDataManager::addWifi(const String& ssid, const String& psk) {
  list.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
  list.push_back({ ssid, psk });
  saveWifiData(dataPath, list);
}

void WifiDataManager::removeWifi(const String& ssid) {
  list.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
  saveWifiData(dataPath, list);
}
