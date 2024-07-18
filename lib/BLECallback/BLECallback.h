#pragma once 

#include "esp_log.h"
#include "NimBLEDevice.h"
#include "NimBLEUtils.h"
#include "NimBLEUUID.h"
#include <utils.h>

static const char *const SERVICE_UUID = "FFF0";
static const char *const DEVICE_NAME = "tps";

class OnAdvertisedDevice : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice *advertisedDevice);
};
