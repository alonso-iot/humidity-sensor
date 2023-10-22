#include <ESP8266WiFi.h>
#include "webserver.h"
#include "data-manager.h"
#include "wifi-setup.h"

using namespace WifiSetup;

static bool connectToSSID(const String& hostname, const String& ssid, const String& psk) {
  WiFi.hostname(hostname.c_str());

  Serial.printf(">:Connecting to SSID '%s'...\n", ssid.c_str());
  WiFi.begin(ssid, psk);

  do {
    delay(100);

    switch (WiFi.status()) {
      case WL_CONNECTED: {
        Serial.println(">:Connected!");
        return true;
      }

      case WL_DISCONNECTED: {
        break;
      }

      default:
        return false;
    }
  }
  while (true);
}

class WifiSetupImpl {
public:
  ~WifiSetupImpl() {
    stop();
  }

  void start(const String& dataPath) {
    dataManager.start(dataPath);
  }

  void launchConfiguration(const WifiSetupOpts& opts) {
    createAP(opts.ssid, opts.psk);
    webServer.start(opts.port, &dataManager);
  }

  void stop() {
    webServer.stop();
  }

  bool tryConnect() {
    WiFi.mode(WIFI_STA);
    const String& hostname = dataManager.getHostname();
    for (const auto wifi : dataManager.getList()) {
      if (connectToSSID(hostname, wifi.ssid, wifi.psk)) {
        return true;
      }
    }

    return false;
  }

  const String& getHostname() const {
    return dataManager.getHostname();
  }

  void setHostname(const String& hostname) {
    return dataManager.setHostname(hostname);
  }

private:
  void createAP(const String& ssid, const String& psk) {
    WiFi.softAP(ssid, psk);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(">:Setup IP address: ");
    Serial.println(myIP);
  }

private:
  DataManager dataManager;
  WebServer webServer;
};


Setup::Setup() {
  impl = new WifiSetupImpl();
}

Setup::~Setup() {
  delete static_cast<WifiSetupImpl*>(impl);
}

void Setup::launchConfiguration(const WifiSetupOpts& opts) {
  return static_cast<WifiSetupImpl*>(impl)->launchConfiguration(opts);
}

void Setup::start(const String& dataPath) {
  return static_cast<WifiSetupImpl*>(impl)->start(dataPath);
}

void Setup::stop() {
  return static_cast<WifiSetupImpl*>(impl)->stop();
}

bool Setup::tryConnect() {
  return static_cast<WifiSetupImpl*>(impl)->tryConnect();
}

const String& Setup::getHostname() const {
  return static_cast<WifiSetupImpl*>(impl)->getHostname();
}

void Setup::setHostname(const String& hostname) {
  return static_cast<WifiSetupImpl*>(impl)->setHostname(hostname);
}
