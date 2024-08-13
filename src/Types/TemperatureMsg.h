#pragma once

#include <ArduinoJson.h>

struct TemperatureMessage {
  String MAC;
  float TempC;
  float TempF;
  float BatteryPercent;
  float Humidity;
  JsonDocument ToJSON() {
    JsonDocument d;
    d["MAC"] = MAC;
    d["TempC"] = TempC;
    d["TempF"] = TempF;
    d["BatteryPct"] = BatteryPercent;
    d["Humidity"] = Humidity;
    return d;
  }
  String ToJSONString() {
    JsonDocument d = ToJSON();
    String jsonString;
    serializeJson(d, jsonString);
    return jsonString;
  }
};
