#pragma once

namespace WifiSetup {

  struct WifiSetupData;

  struct WifiSetupOpts {
    int port = 80;
    String ssid = "Sensor setup";
    String psk = "12345678";
    String dataPath = "/ssid.json";
  };

  class Setup {
  public:
    Setup(const WifiSetupOpts& opts = WifiSetupOpts());
    ~Setup();

    void start();
    void stop();

  private:
    WifiSetupOpts opts;
    void* impl;
  };

}
