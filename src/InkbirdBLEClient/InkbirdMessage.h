#pragma once

#include <ArduinoJson.h>

struct InkbirdMessage {
  String MAC;
  float TempC;
  float TempF;
  float BatteryPercent;
  float Humidity;
  int RSSI;
  JsonDocument ToJSON() {
    JsonDocument d;
    d["MAC"] = MAC;
    d["TempC"] = TempC;
    d["TempF"] = TempF;
    d["BatteryPct"] = BatteryPercent;
    d["Humidity"] = Humidity;
    d["RSSI"] = RSSI;
    return d;
  }
  String ToJSONString() {
    JsonDocument d = ToJSON();
    String jsonString;
    serializeJson(d, jsonString);
    return jsonString;
  }
};
