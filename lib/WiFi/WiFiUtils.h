#pragma once

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"

#include <string>
#include <string.h>
#include <sstream>
#include <netdb.h>
#include <iomanip>

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
    bool IsConnected();
    void SetConnected(bool connected);
    esp_netif_t *GetInterface();
    std::string GetMACAddrStr();

    private:
    std::string ssid;
    std::string password;
    std::string intDescription;
    esp_netif_t* wifiInterface;
    SemaphoreHandle_t sema_get_ip_addrs;
    int connect_retry_count;
    bool connected;
    u_int8_t mac[6];
    
};