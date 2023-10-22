#pragma once

#include <ESPAsyncWebServer.h>
#include "data-manager.h"

namespace WifiSetup {

  class WebServer {
  public:
    void start(int port, DataManager* dataManager);
    void stop();

  private:
    AsyncWebServer* server;
  };

}
