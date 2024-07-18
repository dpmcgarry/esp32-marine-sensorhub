#include <BLECallback.h>

static const char *const TAG = "esp32-temp-reporter-ble-callback";

void OnAdvertisedDevice::onResult(BLEAdvertisedDevice *advertisedDevice)
{
    char *mfrData;
    if (advertisedDevice->haveServiceUUID())
    {
        ESP_LOGD(TAG, "Service ID: %s",
                 advertisedDevice->getServiceUUID().toString().c_str());
    }
    if (advertisedDevice->haveServiceUUID() &&
        advertisedDevice->isAdvertisingService(NimBLEUUID(SERVICE_UUID)) &&
        advertisedDevice->haveName() &&
        strcmp(advertisedDevice->getName().c_str(), DEVICE_NAME) == 0)
    {
        ESP_LOGI(TAG, "Device Found with matching service and name.");
        BLEDevice::getScan()->stop();
        ESP_LOGI(TAG, "Discovered Advertised Device: %s", advertisedDevice->toString().c_str());
        mfrData = NimBLEUtils::buildHexData(nullptr, (uint8_t*)advertisedDevice->getManufacturerData().data(), advertisedDevice->getManufacturerData().length());
        ESP_LOGI(TAG, "Manufacturer Data: %s", mfrData);
        std::string address = advertisedDevice->getAddress().toString();
        ESP_LOGI(TAG, "Address: %s", address.c_str());
        char foo[2];
        foo[2]='\0';
        memcpy(foo, mfrData,2);
        long lsb = std::stoul(foo,NULL,16);
        ESP_LOGD(TAG, "LSB: %ld", lsb);
        memcpy(foo, &mfrData[2],2);
        long msb = std::stoul(foo,NULL,16);
        ESP_LOGD(TAG, "MSB: %ld", msb);
        msb = msb<<8;
        ESP_LOGD(TAG, "MSB: %ld", msb);
        float temp = (float)(lsb + msb) / 100.0;
        ESP_LOGI(TAG, "Temperature C: %f", temp);
        ESP_LOGI(TAG, "Temperature F: %f", CtoF(temp));
        memcpy(foo, &mfrData[14],2);
        float battery = (float)std::stoul(foo, NULL, 16);
        ESP_LOGI(TAG, "Battery: %f", battery);
    }
}