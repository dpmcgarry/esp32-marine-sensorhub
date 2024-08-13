#include "Prefs.h"

using namespace msh;

Prefs::Prefs() {
  this->enable_ble = false;
  this->static_ip_en = false;
  this->log_level = 4;
  this->root_topic = "esp32msh";
  this->use_private_ca = false;
}

int Prefs::LogLevel() { return (int)this->log_level; }

String Prefs::SSID() { return this->ssid; }

String Prefs::SSIDPassword() { return this->ssid_password; }

String Prefs::MQTTUri() { return this->mqtt_uri; }

String Prefs::RootTopic() { return this->root_topic; }

bool Prefs::BLEEnabled() { return this->enable_ble; }

bool Prefs::StaticIPEnabled() { return this->static_ip_en; }

String Prefs::IPAddress() { return this->ip_address; }

String Prefs::IPGateway() { return this->ip_gateway; }

String Prefs::Netmask() { return this->netmask; }

String Prefs::DNS1() { return this->dns1; }

String Prefs::DNS2() { return this->dns2; }

bool Prefs::UsePrivateCA() { return this->use_private_ca; }

String Prefs::MQTTPrivateCA() { return this->mqtt_private_ca; }

bool Prefs::Load() {
  Log.notice("Loading Preferences");
  Log.notice("Using preferences namespace: %s", PREFS_NAMESPACE);
  Preferences myPrefs;
  bool exists = myPrefs.begin(PREFS_NAMESPACE.c_str(), true);
  if (!exists) {
    Log.error("Preferences namespace does not exist");
    return false;
  }

  String pref = "ssid_name";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (!exists) {
    Log.error("%s is required and not defined", pref);
    return false;
  }
  this->ssid = myPrefs.getString(pref.c_str());
  Log.trace("Got value: %s for pref: %s", this->ssid.c_str(), pref);
  pref = "ssid_password";

  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (!exists) {
    Log.error("%s is required and not defined", pref);
    return false;
  }
  this->ssid_password = myPrefs.getString(pref.c_str());
  Log.trace("Got value: %s for pref: %s", this->ssid_password.c_str(), pref);

  pref = "mqtt_uri";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (!exists) {
    Log.error("%s is required and not defined", pref);
    return false;
  }
  this->mqtt_uri = myPrefs.getString(pref.c_str());
  Log.trace("Got value: %s for pref: %s", this->mqtt_uri.c_str(), pref);

  pref = "root_topic";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {

    this->root_topic = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->root_topic, pref);
  }

  pref = "enable_ble";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  u8_t temp_u8_pref;
  if (exists) {
    temp_u8_pref = myPrefs.getUChar(pref.c_str());
    Log.trace("Got raw value: %d for pref: %s", temp_u8_pref, pref);
    if (temp_u8_pref == 0) {
      this->enable_ble = false;
    } else if (temp_u8_pref == 1) {
      this->enable_ble = true;
    } else {
      Log.error("Invalid preference value %d for %s", temp_u8_pref, pref);
      return false;
    }
  }

  pref = "use_staticip";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    temp_u8_pref = myPrefs.getUChar(pref.c_str());
    Log.trace("Got raw value: %d for pref: %s", temp_u8_pref, pref);
    if (temp_u8_pref == 0) {
      this->static_ip_en = false;
    } else if (temp_u8_pref == 1) {
      this->static_ip_en = true;
    } else {
      Log.error("Invalid preference value %d for %s", temp_u8_pref, pref);
      return false;
    }
  }

  pref = "ip_address";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->ip_address = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->ip_address, pref);
  } else if (!exists && this->static_ip_en) {
    Log.error("Use Static IP is enabled but IP Address is not defined which is "
              "required");
    return false;
  }

  pref = "gateway";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->ip_gateway = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->ip_gateway, pref);
  } else if (!exists && this->static_ip_en) {
    Log.error("Use Static IP is enabled but IP Address is not defined which is "
              "required");
    return false;
  }

  pref = "netmask";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->netmask = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->netmask, pref);
  } else if (!exists && this->static_ip_en) {
    Log.error("Use Static IP is enabled but IP Address is not defined which is "
              "required");
    return false;
  }

  pref = "dns1";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->dns1 = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->dns1, pref);
  }

  pref = "dns2";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->dns2 = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->dns2, pref);
  }

  pref = "use_privateca";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    temp_u8_pref = myPrefs.getUChar(pref.c_str());
    Log.trace("Got raw value: %d for pref: %s", temp_u8_pref, pref);
    if (temp_u8_pref == 0) {
      this->use_private_ca = false;
    } else if (temp_u8_pref == 1) {
      this->use_private_ca = true;
    } else {
      Log.error("Invalid preference value %d for %s", temp_u8_pref, pref);
      return false;
    }
  }

  pref = "mqtt_ca";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->mqtt_private_ca = myPrefs.getString(pref.c_str());
    Log.trace("Got value: %s for pref: %s", this->mqtt_private_ca.c_str(),
              pref);
  } else if (!exists && this->use_private_ca) {
    Log.error("Use Private CA is enabled but MQTT CA is not defined which is "
              "required");
    return false;
  }

  pref = "log_level";
  Log.trace("Checking %s", pref);
  exists = myPrefs.isKey(pref.c_str());
  if (exists) {
    this->log_level = myPrefs.getUChar(pref.c_str());
    Log.trace("Got value: %d for pref: %s", this->log_level, pref);
  }

  return true;
}