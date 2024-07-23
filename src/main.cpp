#include "esp_log.h"
#include "nvs_flash.h"
#include "NimBLEDevice.h"
#include <BLECallback.h>
#include <WiFiUtils.h>
#include <MQTTClientUtils.h>

#undef TAG

static const char *const TAG = "esp32-temp-reporter-main";
static const u_int DELAY_MS = 500;

extern "C"
{
    void app_main(void);
}

void scanTask(void *parameter)
{
    for (;;)
    {
        // Delay  between loops.
        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS); 
        ESP_LOGD(TAG, "Rerunning scan\n");
        // TODO: See if there is a better way to restart
        BLEDevice::getScan()->start(0);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BLE Client application...\n");
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
    
    BLEDevice::init("");
    ESP_LOGD(TAG, "BLE Initialized\n");
    ESP_LOGI(TAG, "Connecting to WiFi\n");
    WiFiInfo *wifi = new WiFiInfo();
    wifi->SetSSID(SSID_NAME);
    wifi->SetPassword(SSID_PASWORD);
    ESP_ERROR_CHECK(wifi->Connect());
    ESP_LOGI(TAG, "Connected!\n");
    ESP_LOGI(TAG, "MQTT Client Init\n");
    MQTTClientUtils *mqtt_client = new MQTTClientUtils(MQTT_URI);
    mqtt_client->Connect();
    BLEScan *pBLEScan = BLEDevice::getScan();
    ESP_LOGD(TAG, "Got Scan Object\n");
    pBLEScan->setScanCallbacks(new OnAdvertisedDevice(mqtt_client));
    ESP_LOGD(TAG, "Set Callback Function\n");
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    ESP_LOGD(TAG, "Set scan paramters\n");
    pBLEScan->start(5 * 1000, false);
    ESP_LOGD(TAG, "Creating FreeRTOS Task\n");
    xTaskCreate(scanTask, "scanTask", 5000, NULL, 1, NULL);
    ESP_LOGD(TAG, "Task exit\n");
}