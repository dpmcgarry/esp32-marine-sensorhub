#pragma once

#include <ArduinoJson.h>

struct ESPMessage {
  String MAC;
  String DeviceTime;
  size_t FreeSRAM;
  uint32_t FreeHeap;
  size_t FreePSRAM;
  String IPAddress;
  uint16_t WifiReconnectCount;
  uint16_t MQTTReconnectCount;
  bool BLEEnabled;
  bool RTDEnabled;
  int8_t WiFiRSSI;
  bool HasTime;
  bool HasResetMQTT;
  String MSHVersion;

  JsonDocument ToJSON() {
    JsonDocument d;
    d["MAC"] = MAC;
    d["DeviceTime"] = DeviceTime;
    d["FreeSRAM"] = FreeSRAM;
    d["FreeHeap"] = FreeHeap;
    d["FreePSRAM"] = FreePSRAM;
    d["IPAddress"] = IPAddress;
    d["WiFiReconnectCount"] = WifiReconnectCount;
    d["MQTTReconnectCount"] = MQTTReconnectCount;
    d["BLEEnabled"] = BLEEnabled;
    d["RTDEnabled"] = RTDEnabled;
    d["WiFiRSSI"] = WiFiRSSI;
    d["HasTime"] = HasTime;
    d["HasResetMQTT"] = HasResetMQTT;
    d["MSHVersion"] = MSHVersion;
    return d;
  }
  String ToJSONString() {
    JsonDocument d = ToJSON();
    String jsonString;
    serializeJson(d, jsonString);
    return jsonString;
  }
};
