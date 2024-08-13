#pragma once

#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <BLEUUID.h>
#include <BLEUtils.h>
#include <algorithm>
#include <vector>

#include <src/MQTTClient/MQTTClient.h>
#include <src/Types/TemperatureMsg.h>

static const char *const SERVICE_UUID = "FFF0";
static std::vector<String> DEVICE_NAMES = {"tps", "sps"};
namespace msh {
class InkbirdBLEClient : public BLEAdvertisedDeviceCallbacks {
public:
  InkbirdBLEClient();
  InkbirdBLEClient(msh::MQTTClient *mqtt);
  void onResult(BLEAdvertisedDevice advertisedDevice);
  void Scanning(bool scan);
  bool Scanning();

private:
  msh::MQTTClient *mqtt;
  bool isScanning;
};
}; // namespace msh