#pragma once 

#include "esp_log.h"
#include "NimBLEDevice.h"
#include "NimBLEUtils.h"
#include "NimBLEUUID.h"
#include <sstream>
#include <utils.h>
#include <MQTTClientUtils.h>

static const char *const SERVICE_UUID = "FFF0";
static const char *const DEVICE_NAME = "tps";

class OnAdvertisedDevice : public BLEAdvertisedDeviceCallbacks
{
    public:
    OnAdvertisedDevice();
    OnAdvertisedDevice(MQTTClientUtils *mqtt);

    private:
    MQTTClientUtils *mqtt;
    void onResult(BLEAdvertisedDevice *advertisedDevice);

};
