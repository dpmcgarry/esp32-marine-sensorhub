#pragma once 

#include "esp_log.h"
#include "NimBLEDevice.h"
#include "NimBLEUtils.h"
#include "NimBLEUUID.h"
#include <sstream>
#include <utils.h>
#include <MQTTClientUtils.h>
#include <TemperatureMsg.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <algorithm>

static const char *const SERVICE_UUID = "FFF0";
static std::vector<std::string> DEVICE_NAMES = {"tps", "sps"};

class BLETempHandler : public BLEAdvertisedDeviceCallbacks
{
    public:
    BLETempHandler();
    BLETempHandler(MQTTClientUtils *mqtt);

    private:
    MQTTClientUtils *mqtt;
    void onResult(BLEAdvertisedDevice *advertisedDevice);

};
