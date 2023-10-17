#pragma once

struct WifiSetupData;

struct WifiSetupOpts {
  int port = 80;
  String ssid = "Sensor setup";
  String psk = "12345678";
  String dataPath = "/ssid.json";
};

class WifiSetup {
public:
  WifiSetup(const WifiSetupOpts& opts = WifiSetupOpts());
  ~WifiSetup();

  void start();
  void stop();

private:
  WifiSetupOpts opts;
  void* impl;
};
