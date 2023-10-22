#pragma once

#include <Arduino.h>
#include <list>

namespace WifiSetup {

  struct WifiData {
    const String ssid;
    const String psk;
  };

  typedef std::list<WifiData> WifiList;

  class DataManager {
  public:
    void start(const String& dataPath);

    void addWifi(const String& ssid, const String& psk);
    void removeWifi(const String& ssid);

    void setHostname(const String& hostname);

    const String& getHostname() const { return hostname; }
    const WifiList& getList() const { return list; }

  private:
    String dataPath;
    String hostname;
    WifiList list;
  };
  
}
