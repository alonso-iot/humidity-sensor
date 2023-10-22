#pragma once

namespace WifiSetup {

  struct WifiSetupData;

  struct WifiSetupOpts {
    int port = 80;
    String ssid = "Sensor setup";
    String psk = "12345678";
  };

  class Setup {
  public:
    Setup();
    ~Setup();

    bool tryConnect();
    void launchConfiguration(const WifiSetupOpts& opts);

    const String& getHostname() const;
    void setHostname(const String& hostname);
    
    void start(const String& dataPath = "/config.json");
    void stop();

  private:
    void* impl;
  };

}
