#include <stddef.h>
#include <EEPROM.h>
#include "wifi-config.h"

#define EEPROM_MAGIC 0xDEADC0DE

struct WifiConfigurationData {
  unsigned int magic;
  size_t ssid_length;
  char ssid[32];
  size_t psk_length;
  char psk[64];
};

size_t getWifiConfigurationSize() { return sizeof(WifiConfigurationData); }

WifiConfiguration loadWifiConfiguration(int address) {
  WifiConfiguration config;
  WifiConfigurationData data;

  EEPROM.get(address, data);

  memset(config.ssid, 0, sizeof(config.ssid));
  memset(config.psk, 0, sizeof(config.psk));

  if (data.magic != EEPROM_MAGIC) {
    config.valid = false;
  }
  else {
    config.valid = true;
    memcpy(config.ssid, data.ssid, sizeof(char) * data.ssid_length);
    memcpy(config.psk,  data.psk,  sizeof(char) * data.psk_length);
  }
  
  return config;
}

void saveWifiConfiguration(int address, WifiConfiguration config) {
  WifiConfigurationData data;
  data.magic = EEPROM_MAGIC;
  
  #define SAFE_COPY(x, y) \
    x ## _length = strlen(y); \
    memset(x, 0, sizeof(x)); \
    strncpy(x, y, x ## _length)

  SAFE_COPY(data.ssid, config.ssid);
  SAFE_COPY(data.psk, config.psk);

  #undef SAFE_COPY

  EEPROM.put(address, data);
  EEPROM.commit();
}
