#pragma once

#include "esp_wifi.h"
#include "esp_log.h"

#include <string>
#include <string.h>

class WiFiInfo{
    public:
    WiFiInfo();
    void SetSSID(const std::string &ssid);
    void SetPassword(const std::string &password);
    void SetInterfaceDescription(const std::string &desc);
    void IncrementRetries();
    void ClearRetries();
    std::string &GetSSID();
    std::string &GetPassword();
    std::string &GetInterfaceDescription();
    SemaphoreHandle_t GetIPSemaphore();
    int GetRetries();
    esp_err_t Connect();

    private:
    std::string ssid;
    std::string password;
    std::string intDescription;
    esp_netif_t* wifiInterface;
    SemaphoreHandle_t sema_get_ip_addrs;
    int connect_retry_count;
    
};