#include <AsyncJson.h>
#include <LittleFS.h>
#include "webserver.h"

using namespace WifiSetup;

static void handleAdd(AsyncWebServerRequest *req, const JsonVariant &json, WifiDataManager* dataManager) {
  const JsonObject& jsonObj = json.as<JsonObject>();
  const String ssid = jsonObj["ssid"].as<String>();
  const String psk = jsonObj["psk"].as<String>();
  dataManager->addWifi(ssid, psk);

  auto res = req->beginResponse(201);
  res->addHeader("Access-Control-Allow-Origin", "*");
  req->send(res);
}

static void handleDelete(AsyncWebServerRequest *req, const JsonVariant &json, WifiDataManager* dataManager) {
  const JsonObject& jsonObj = json.as<JsonObject>();
  const String ssid = jsonObj["ssid"].as<String>();
  dataManager->removeWifi(ssid);

  auto res = req->beginResponse(200);
  res->addHeader("Access-Control-Allow-Origin", "*");
  req->send(res);
}

static void handleList(AsyncWebServerRequest* req, WifiDataManager* dataManager) {
  String json = "[";
  bool comma = false;
  for (const auto wifi : dataManager->getList()) {
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

static void handleScan(AsyncWebServerRequest* req) {
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

static void setupRoutes(AsyncWebServer* server, WifiDataManager* dataManager) {
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

  server->on("/scan", HTTP_GET, handleScan);
  server->on("/ssid", HTTP_GET, std::bind(handleList, std::placeholders::_1, dataManager));
  
  auto addHandler = new AsyncCallbackJsonWebHandler("/ssid", std::bind(handleAdd, std::placeholders::_1, std::placeholders::_2, dataManager));
  addHandler->setMethod(HTTP_POST);
  server->addHandler(addHandler);

  auto deleteHandler = new AsyncCallbackJsonWebHandler("/ssid", std::bind(handleDelete, std::placeholders::_1, std::placeholders::_2, dataManager));
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

void WebServer::start(int port, WifiDataManager* dataManager) {
  server = new AsyncWebServer(port);
  setupRoutes(server, dataManager);
  server->begin();
}

void WebServer::stop() {
  if (server) {
    server->end();
    delete server;
    server = nullptr;
  }
}
