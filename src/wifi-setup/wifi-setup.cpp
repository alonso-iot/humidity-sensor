#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <list>
#include <functional>
#include <LittleFS.h>
#include "wifi-setup.h"

struct WifiData {
  String ssid;
  String psk;
};

class WifiSetupImpl {
public:
  ~WifiSetupImpl() {
    stop();
  }

  void start(const WifiSetupOpts& opts) {
    loadWifiData(opts.dataPath, &wifiData);
    createAP(opts.ssid, opts.psk);

    server = new AsyncWebServer(opts.port);
    setupRoutes(server, opts.dataPath);
    server->begin();
  }

  void stop() {
    if (server) {
      server->end();
      delete server;
      server = nullptr;
    }
  }

private:
  void loadWifiData(const String& dataPath, std::list<WifiData>* v) {
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

  void saveWifiData(const String& dataPath, const std::list<WifiData>& v) {
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

  void createAP(const String& ssid, const String& psk) {
    WiFi.softAP(ssid, psk);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(">:Setup IP address: ");
    Serial.println(myIP);
  }

  void setupRoutes(AsyncWebServer* server, const String& dataPath) {
    server->on("/", HTTP_GET, [](auto* req) { req->redirect("/setup"); });
    server->on("/reboot", HTTP_POST, [](auto* req) { req->send(200); ESP.reset(); });

    server->serveStatic("/setup", LittleFS, "/setup.html");
    for (auto file : {
        "axios.min.js",
        "card.css",
        "known-ssid-list.css",
        "loading.gif",
        "signal-0.png",
        "signal-1.png",
        "signal-2.png",
        "signal-3.png",
        "signal-x.png",
        "vue.global.prod.js",
        "wifi-scanner.css",
      }) {
      String path = String("/") + file;
      server->serveStatic(path.c_str(), LittleFS, file);
    }

    server->on("/scan", HTTP_GET, std::bind(&WifiSetupImpl::handleScan, this, std::placeholders::_1));
    server->on("/ssid", HTTP_GET, std::bind(&WifiSetupImpl::handleList, this, std::placeholders::_1));
   
    auto addHandler = new AsyncCallbackJsonWebHandler("/ssid", std::bind(&WifiSetupImpl::handleAdd, this, std::placeholders::_1, std::placeholders::_2, dataPath));
    addHandler->setMethod(HTTP_POST);
    server->addHandler(addHandler);

    auto deleteHandler = new AsyncCallbackJsonWebHandler("/ssid", std::bind(&WifiSetupImpl::handleDelete, this, std::placeholders::_1, std::placeholders::_2, dataPath));
    deleteHandler->setMethod(HTTP_DELETE);
    server->addHandler(deleteHandler);

    server->onNotFound([](AsyncWebServerRequest *req) {
      auto res = req->beginResponse(req->method() == HTTP_OPTIONS ? 200 : 404);
      if (req->method() == HTTP_OPTIONS) {
        res->addHeader("Access-Control-Allow-Origin", "*");
        res->addHeader("Access-Control-Allow-Methods", "*");
        res->addHeader("Access-Control-Allow-Headers", "*");
      }
      req->send(res);
    });
  }

  void handleAdd(AsyncWebServerRequest *req, const JsonVariant &json, const String& dataPath) {
    const JsonObject& jsonObj = json.as<JsonObject>();
    const String ssid = jsonObj["ssid"].as<String>();
    const String psk = jsonObj["psk"].as<String>();
    wifiData.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
    wifiData.push_back({ ssid, psk });
    saveWifiData(dataPath, wifiData);

    auto res = req->beginResponse(201);
    res->addHeader("Access-Control-Allow-Origin", "*");
    req->send(res);
  }

  void handleDelete(AsyncWebServerRequest *req, const JsonVariant &json, const String& dataPath) {
    const JsonObject& jsonObj = json.as<JsonObject>();
    const String ssid = jsonObj["ssid"].as<String>();
    wifiData.remove_if([&ssid](auto wifi) { return wifi.ssid == ssid; });
    saveWifiData(dataPath, wifiData);

    auto res = req->beginResponse(200);
    res->addHeader("Access-Control-Allow-Origin", "*");
    req->send(res);
  }

  void handleList(AsyncWebServerRequest* req) {
    String json = "[";
    bool comma = false;
    for (const auto wifi : wifiData) {
      if (comma) {
        json += ",";
      }
      comma = true;

      json += "\"" + wifi.ssid + "\"";
    }
    json += "]";

    auto res = req->beginResponse(200, "application/json", json);
    res->addHeader("Access-Control-Allow-Origin", "*");
    req->send(res);
  }

  void handleScan(AsyncWebServerRequest* req) {
    String json = "[";
    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
    }
    else if (n > 0) {
      for (int i = 0; i < n; ++i) {
        if (i > 0) {
          json += ",";
        }
        json += "{";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
        json += ",\"channel\":" + String(WiFi.channel(i));
        json += ",\"secure\":" + String(WiFi.encryptionType(i));
        json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
        json += "}";
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) {
        WiFi.scanNetworks(true);
      }
    }
    json += "]";

    auto res = req->beginResponse(200, "application/json", json);
    res->addHeader("Access-Control-Allow-Origin", "*");
    req->send(res);
  }

private:
  AsyncWebServer* server;
  std::list<WifiData> wifiData;
};


WifiSetup::WifiSetup(const WifiSetupOpts& opts) : opts(opts) {
  impl = new WifiSetupImpl();
}

WifiSetup::~WifiSetup() {
  delete static_cast<WifiSetupImpl*>(impl);
}

void WifiSetup::start() {
  static_cast<WifiSetupImpl*>(impl)->start(opts);
}

void WifiSetup::stop() {
  static_cast<WifiSetupImpl*>(impl)->stop();
}
