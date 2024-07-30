#include "esp_log.h"
#include "nvs_flash.h"
#include "NimBLEDevice.h"
#include <BLETempHandler.h>
#include <WiFiUtils.h>
#include <MQTTClientUtils.h>
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "freertos/ringbuf.h"

#undef TAG

static const char *const TAG = "esp32-temp-reporter-main";
static const u_int DELAY_MS = 500;
static const u_int MAIN_DELAY_MS = 30000;

extern "C"
{
    void app_main(void);
}

void scanTask(void *parameter)
{
    ESP_LOGI(TAG, "BLE Scan Task Init");
    for (;;)
    {
        // Delay  between loops.
        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS); 
        ESP_LOGD(TAG, "Rerunning scan\n");
        // TODO: See if there is a better way to restart
        BLEDevice::getScan()->start(0);
    }
}

void mainTask(void* parameter)
{
    RingbufHandle_t msg_buf_handle;
    msg_buf_handle = xRingbufferCreate(1028, RINGBUF_TYPE_NOSPLIT);
    if (msg_buf_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create ring buffer");
    }
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
    BLEScan *pBLEScan = BLEDevice::getScan();
    ESP_LOGD(TAG, "Got Scan Object\n");
    pBLEScan->setScanCallbacks(new BLETempHandler(mqtt_client));
    ESP_LOGD(TAG, "Set Callback Function\n");
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    ESP_LOGD(TAG, "Set scan paramters\n");
    pBLEScan->start(5 * 1000, false);
    xTaskCreate(scanTask, "scanTask", 5000, NULL, 1, NULL);
    vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS); 
    for (;;)
    {
        ESP_LOGI(TAG, "Main Task Execution Loop");
        if (wifi == NULL)
        {
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
            ESP_LOGW(TAG, "Wifi is no longer connected. Reconnecting");
            ESP_ERROR_CHECK(wifi->Connect());
            ESP_LOGI(TAG, "Connected!\n");
            ESP_LOGI(TAG, "MAC Address is: %s", wifi->GetMACAddrStr().c_str());
        }
        if (mqtt_client == NULL)
        {
            ESP_LOGW(TAG, "MQTT Client Object was NULL. Re-initializing");
            mqtt_client = new MQTTClientUtils(MQTT_URI);
            mqtt_client->Connect();
        }
        if(!mqtt_client->IsConnected())
        {
            ESP_LOGW(TAG, "MQTT Client is no longer connected");
            if(!wifi->IsConnected())
            {
                ESP_LOGW(TAG, "Wifi isn't connected so waiting for that to get sorted.");
            }
            else
            {
                ESP_LOGW(TAG, "Wifi is connected. Attempting to reconnect MQTT.");
                //mqtt_client->Connect();
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
    
    BLEDevice::init("");
    ESP_LOGD(TAG, "BLE Initialized\n");
    
    
    
    
    ESP_LOGD(TAG, "Creating FreeRTOS Task\n");
    xTaskCreate(mainTask, "mainTask", 5000, NULL, 1, NULL);
    ESP_LOGD(TAG, "Task exit\n");
}