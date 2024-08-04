#include "esp_log.h"
#include "nvs_flash.h"
#include "NimBLEDevice.h"
#include <BLETempHandler.h>
#include <WiFiUtils.h>
#include <MQTTClientUtils.h>
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#undef TAG

static const char *const TAG = "esp32-marine-sensorhub-main";
static const u_int DELAY_MS = 500;
static const u_int MAIN_DELAY_MS = 30000;

extern "C"
{
    void app_main(void);
}

void BLETask(void *parameter)
{
    MQTTClientUtils *mqtt_client = NULL;
    mqtt_client = (MQTTClientUtils*)parameter;
    BLEDevice::init("");
    ESP_LOGD(TAG, "BLE Initialized\n");
    ESP_LOGI(TAG, "BLE Scan Task Init");
    BLEScan *pBLEScan = BLEDevice::getScan();
    ESP_LOGD(TAG, "Got Scan Object\n");
    pBLEScan->setScanCallbacks(new BLETempHandler(mqtt_client));
    ESP_LOGD(TAG, "Set Callback Function\n");
    pBLEScan->setActiveScan(true);
    ESP_LOGD(TAG, "Set scan paramters\n");
    for (;;)
    {
        ESP_LOGI(TAG, "Free Heap: %lu", esp_get_free_heap_size());
        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
        ESP_LOGD(TAG, "Rerunning scan\n");
        if (!pBLEScan->isScanning())
        {
            ESP_LOGI(TAG, "BLE Scanner is not scanning. Starting scan.");
            pBLEScan->start(5000, false);
        }
        else
        {
            ESP_LOGI(TAG, "BLE Scanner is scanning. Continuing.");
        }
    }
}

void mainTask(void *parameter)
{
    bool hasResetWifi = false;
    bool hasResetMQTT = false;
    u16_t wifiReconnectCount = 0;
    u16_t mqttReconnectCount = 0;
    ESP_LOGI(TAG, "Connecting to WiFi\n");
    WiFiInfo *wifi = new WiFiInfo();
    wifi->SetSSID(SSID_NAME);
    wifi->SetPassword(SSID_PASWORD);
    ESP_ERROR_CHECK(wifi->Connect());
    ESP_LOGI(TAG, "Connected!\n");
    ESP_LOGI(TAG, "MAC Address is: %s", wifi->GetMACAddrStr().c_str());

    ESP_LOGI(TAG, "MQTT Client Init\n");
    MQTTClientUtils *mqtt_client = new MQTTClientUtils(MQTT_URI);
    mqtt_client->Connect();
    #ifdef ENABLE_BLE
    xTaskCreate(BLETask, "BLETask", 5000, mqtt_client, 1, NULL);
    #endif
    vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
    for (;;)
    {
        ESP_LOGI(TAG, "Main Task Execution Loop");
        ESP_LOGI(TAG, "Free SRAM: %zu", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        ESP_LOGI(TAG, "Free Heap: %lu", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Free PSRAM: %zu", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGI(TAG, "WiFi has been re-initialized? %s", hasResetWifi ? "true" : "false");
        ESP_LOGI(TAG, "MQTT has been re-initialized? %s", hasResetMQTT ? "true" : "false");
        ESP_LOGI(TAG, "WiFi Reconnect Count: %u", wifiReconnectCount);
        ESP_LOGI(TAG, "MQTT Reconnect Count: %u", mqttReconnectCount);
        if (wifi == NULL)
        {
            hasResetWifi = true;
            ESP_LOGW(TAG, "Wifi Object was NULL. Re-initializing");
            wifi = new WiFiInfo();
            wifi->SetSSID(SSID_NAME);
            wifi->SetPassword(SSID_PASWORD);
            ESP_ERROR_CHECK(wifi->Connect());
            ESP_LOGI(TAG, "Connected!\n");
            ESP_LOGI(TAG, "MAC Address is: %s", wifi->GetMACAddrStr().c_str());
        }
        if (!wifi->IsConnected())
        {
            wifiReconnectCount++;
            ESP_LOGW(TAG, "Wifi is no longer connected. Reconnecting");
            ESP_ERROR_CHECK(wifi->Connect());
            ESP_LOGI(TAG, "Connected!\n");
            ESP_LOGI(TAG, "MAC Address is: %s", wifi->GetMACAddrStr().c_str());
        }
        if (mqtt_client == NULL)
        {
            hasResetMQTT = true;
            ESP_LOGW(TAG, "MQTT Client Object was NULL. Re-initializing");
            mqtt_client = new MQTTClientUtils(MQTT_URI);
            mqtt_client->Connect();
        }
        if (!mqtt_client->IsConnected())
        {
            ESP_LOGW(TAG, "MQTT Client is no longer connected");
            if (!wifi->IsConnected())
            {
                ESP_LOGW(TAG, "Wifi isn't connected so waiting for that to get sorted.");
            }
            else
            {
                ESP_LOGW(TAG, "Wifi is connected. Attempting to reconnect MQTT.");
                mqttReconnectCount++;
                mqtt_client->Connect();
            }
        }
        vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BLE Client application.");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);
    ESP_LOGD(TAG, "ESP Init Complete\n");
    ESP_LOGD(TAG, "Creating FreeRTOS Main Task\n");
    xTaskCreate(mainTask, "mainTask", 5000, NULL, 1, NULL);
    ESP_LOGD(TAG, "Task exit\n");
}