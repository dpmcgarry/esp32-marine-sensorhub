#pragma once

#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <WiFi.h>
#include "time.h"


#include <src/MQTTClient/MQTTClient.h>
#include <src/Prefs/Prefs.h>
#include "ESPMessage.h"
#include "MSHGlobal.h"

namespace msh {
class ESPClient {
public:
  ESPClient();
  ESPClient(msh::MQTTClient *mqtt, msh::Prefs *prefs);
  bool Read();


private:
  msh::MQTTClient *mqtt;
  msh::Prefs *prefs;
};
};  // namespace msh