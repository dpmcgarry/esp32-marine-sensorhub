#include <BLETempHandler.h>

using json = nlohmann::json;

static const char *const TAG = "esp32-temp-reporter-ble-callback";
static const std::string bleTemp_subtopic = "ble/temperature";

BLETempHandler::BLETempHandler()
{
    this->mqtt = NULL;
}

BLETempHandler::BLETempHandler(MQTTClientUtils *mqtt)
{
    this->mqtt = mqtt;
}

void BLETempHandler::onResult(BLEAdvertisedDevice *advertisedDevice)
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
        float temp_f = CtoF(temp);
        ESP_LOGI(TAG, "Temperature F: %f", temp_f);
        memcpy(foo, &mfrData[14],2);
        float battery = (float)std::stoul(foo, NULL, 16);
        ESP_LOGI(TAG, "Battery: %f", battery);
        TemperatureMessage tMsg;
        tMsg.MAC = address;
        tMsg.TempC = temp;
        tMsg.TempF = temp_f;
        tMsg.BatteryPercent = battery;
        json j = tMsg;
        std::string j_str = nlohmann::to_string(j);
        ESP_LOGI(TAG,"JSON: %s", j_str.c_str());
        std::string topic_str = ROOT_TOPIC;
        topic_str = topic_str + "/" + bleTemp_subtopic + "/" + address;
        if (this->mqtt != NULL)
        {
            if (this->mqtt->IsConnected())
            {
                ESP_LOGI(TAG, "MQTT is Connected. Will publish to topic: %s", topic_str.c_str());
                this->mqtt->Publish(topic_str, j_str, 0, 1, 0);
            }
            else
            {
                ESP_LOGI(TAG, "MQTT is NOT Connected.");
            }
            
        }
    }
}