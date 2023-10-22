#pragma once

#include <ESPAsyncWebServer.h>
#include "wifi-data-manager.h"

namespace WifiSetup {

  class WebServer {
  public:
    void start(int port, WifiDataManager* dataManager);
    void stop();

  private:
    AsyncWebServer* server;
  };

}
