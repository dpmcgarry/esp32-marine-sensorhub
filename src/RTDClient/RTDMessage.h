#pragma once

#include <ArduinoJson.h>

struct RTDMessage {
  String MAC;
  float TempC;
  float TempF;
  JsonDocument ToJSON() {
    JsonDocument d;
    d["MAC"] = MAC;
    d["TempC"] = TempC;
    d["TempF"] = TempF;
    return d;
  }
  String ToJSONString() {
    JsonDocument d = ToJSON();
    String jsonString;
    serializeJson(d, jsonString);
    return jsonString;
  }
};
