#include <ArduinoJson.h>
#include <LittleFS.h>
#include "data-manager.h"

using namespace WifiSetup;

static void loadData(const String& dataPath, WifiList* v, String* hostname) {
  auto file = LittleFS.open(dataPath, "r");
  if (file) {
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, file);
    file.close();

    v->clear();
    for (const auto wifi : doc["ssids"].as<JsonArray>()) {
      v->push_back({ wifi["ssid"], wifi["psk"] });
    }

    *hostname = doc["hostname"].as<String>();
  }

  Serial.printf(">:Loading data from %s...\n", dataPath.c_str());
}

static void saveData(const String& dataPath, const WifiList& v, const String& hostname) {
  auto file = LittleFS.open(dataPath, "w");
  StaticJsonDocument<1024> doc;
  JsonArray array = doc.createNestedArray("ssids");

  for (const auto wifi : v) {
    auto obj = array.createNestedObject();
    obj["ssid"] = wifi.ssid;
    obj["psk"] = wifi.psk;
  }

  doc["hostname"] = hostname;

  serializeJson(doc, file);
  file.close();

  Serial.printf(">:Saving data to %s...\n", dataPath.c_str());
}

void DataManager::start(const String& dataPath) {
  this->dataPath = dataPath;
  loadData(dataPath, &list, &hostname);
}

void DataManager::addWifi(const String& ssid, const String& psk) {
  list.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
  list.push_back({ ssid, psk });
  saveData(dataPath, list, hostname);
}

void DataManager::removeWifi(const String& ssid) {
  list.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
  saveData(dataPath, list, hostname);
}

void DataManager::setHostname(const String& hostname) {
  this->hostname = hostname;
  saveData(dataPath, list, hostname);
}
