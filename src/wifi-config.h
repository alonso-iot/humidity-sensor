struct WifiConfiguration {
  bool valid;
  char ssid[36]; // 32 + padding
  char psk[68]; // 64 + padding
};

extern size_t getWifiConfigurationSize();
extern WifiConfiguration loadWifiConfiguration(int address);
extern void saveWifiConfiguration(int address, WifiConfiguration config);
