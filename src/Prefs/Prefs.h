#pragma once

#include <ArduinoLog.h>
#include <Preferences.h>

static const String PREFS_NAMESPACE = "esp32mshprefs";
namespace msh {
class Prefs {
public:
  Prefs();
  bool Load();
  String SSID();
  String SSIDPassword();
  String MQTTUri();
  String RootTopic();
  bool BLEEnabled();
  bool StaticIPEnabled();
  String IPAddress();
  String IPGateway();
  String Netmask();
  String DNS1();
  String DNS2();
  bool UsePrivateCA();
  String MQTTPrivateCA();
  int LogLevel();

private:
  String ssid;
  String ssid_password;
  String mqtt_uri;
  String root_topic;
  bool enable_ble;
  bool static_ip_en;
  String ip_address;
  String ip_gateway;
  String netmask;
  String dns1;
  String dns2;
  bool use_private_ca;
  String mqtt_private_ca;
  u8_t log_level;
};
}; // namespace msh