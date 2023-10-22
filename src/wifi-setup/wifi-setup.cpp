#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <list>
#include <functional>
#include <LittleFS.h>
#include "webserver.h"
#include "wifi-data-manager.h"
#include "wifi-setup.h"

using namespace WifiSetup;

class WifiSetupImpl {
public:
  ~WifiSetupImpl() {
    stop();
  }

  void start(const WifiSetupOpts& opts) {
    createAP(opts.ssid, opts.psk);

    dataManager.start(opts.dataPath);
    webServer.start(opts.port, &dataManager);
  }

  void stop() {
    webServer.stop();
  }

private:
  void createAP(const String& ssid, const String& psk) {
    WiFi.softAP(ssid, psk);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(">:Setup IP address: ");
    Serial.println(myIP);
  }

private:
  WifiDataManager dataManager;
  WebServer webServer;
};


Setup::Setup(const WifiSetupOpts& opts) : opts(opts) {
  impl = new WifiSetupImpl();
}

Setup::~Setup() {
  delete static_cast<WifiSetupImpl*>(impl);
}

void Setup::start() {
  static_cast<WifiSetupImpl*>(impl)->start(opts);
}

void Setup::stop() {
  static_cast<WifiSetupImpl*>(impl)->stop();
}
