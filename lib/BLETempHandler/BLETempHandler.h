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

static const char *const SERVICE_UUID = "FFF0";
static const char *const DEVICE_NAME = "tps";

class BLETempHandler : public BLEAdvertisedDeviceCallbacks
{
    public:
    BLETempHandler();
    BLETempHandler(MQTTClientUtils *mqtt);

    private:
    MQTTClientUtils *mqtt;
    void onResult(BLEAdvertisedDevice *advertisedDevice);

};
