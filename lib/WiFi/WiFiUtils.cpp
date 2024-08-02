#include "WiFiUtils.h"

static const char *const TAG = "esp32-temp-reporter-wifi-utils";

WiFiInfo::WiFiInfo()
{
    this->intDescription = "Default Interface";
    this->sema_get_ip_addrs = NULL;
    this->connected = false;
}

void WiFiInfo::SetSSID(const std::string &ssid)
{
    this->ssid = ssid;
}

void WiFiInfo::SetPassword(const std::string &password)
{
    this->password = password;
}

std::string &WiFiInfo::GetSSID()
{
    return this->ssid;
}

std::string &WiFiInfo::GetPassword()
{
    return this->password;
}

void WiFiInfo::IncrementRetries()
{
    this->connect_retry_count++;
}

int WiFiInfo::GetRetries()
{
    return this->connect_retry_count;
}

void WiFiInfo::ClearRetries()
{
    this->connect_retry_count = 0;
}

SemaphoreHandle_t WiFiInfo::GetIPSemaphore()
{
    return this->sema_get_ip_addrs;
}

void WiFiInfo::SetInterfaceDescription(const std::string &description)
{
    this->intDescription = description;
}

std::string &WiFiInfo::GetInterfaceDescription()
{
    return this->intDescription;
}

bool WiFiInfo::IsConnected()
{
    return this->connected;
}

void WiFiInfo::SetConnected(bool connected)
{
    this->connected = connected;
}

esp_netif_t *WiFiInfo::GetInterface()
{
    return this->wifiInterface;
}

std::string WiFiInfo::GetMACAddrStr()
{
    std::stringstream ss;
    ss<<std::hex<<std::setfill('0');
    for (int i=0; i<sizeof(this->mac); i++)
    {
        ss << std::hex << std::setw(2) << static_cast<int>(mac[i]) << ':';
    }
    std::string macstr = ss.str();
    // remove trailing colon
    return macstr.erase(macstr.length()-1);
}

static void OnWiFiDisconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "OnWifiDisconnect");
    WiFiInfo *info = NULL;
    info = (WiFiInfo *)arg;
    info->IncrementRetries();
    info->SetConnected(false);
    ESP_LOGI(TAG, "Retries: %d", info->GetRetries());
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED)
    {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void OnWifiGotIP(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "OnWifiGotIP");
    WiFiInfo *info = NULL;
    info = (WiFiInfo *)arg;
    info->ClearRetries();
    info->SetConnected(true);
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!strncmp(info->GetInterfaceDescription().c_str(), esp_netif_get_desc(event->esp_netif), strlen(info->GetInterfaceDescription().c_str())- 1)==0)
     {
        return;
    }
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    if (info->GetIPSemaphore()) {
        xSemaphoreGive(info->GetIPSemaphore());
    } else {
        ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
}

static void OnWiFiConnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "OnWifiConnect");
    WiFiInfo *info = NULL;
    info = (WiFiInfo *)arg;
    info->ClearRetries();

    #ifdef USE_STATICIP
    ESP_LOGI(TAG, "Will use static IP configuration");
    esp_netif_t *netif = info->GetInterface();
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));
    ip_info.ip.addr = ipaddr_addr(IP_ADDRESS);
    ip_info.netmask.addr = ipaddr_addr(NET_MASK);
    ip_info.gw.addr = ipaddr_addr(GATEWAY);
    if (esp_netif_set_ip_info(netif, &ip_info) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", IP_ADDRESS, NET_MASK, GATEWAY);
    esp_netif_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = ipaddr_addr(DNS_1);
    dns.ip.type = IPADDR_TYPE_V4;
    ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns));
    #ifdef DNS_2
    dns.ip.u_addr.ip4.addr = ipaddr_addr(DNS_2);
    ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_BACKUP, &dns));
    #endif

    #endif
}

esp_err_t WiFiInfo::Connect()
{
    ESP_LOGI(TAG, "Starting WiFi");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_LOGD(TAG, "WiFi Init Complete");
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = this->GetInterfaceDescription().c_str();
    this->wifiInterface = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    ESP_LOGD(TAG, "WiFi Interface Set");
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_LOGD(TAG, "WiFi PreStart");
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi Started");
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, this->mac));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), this->ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password), this->password.c_str(), sizeof(wifi_config.sta.password));
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.listen_interval = 0;
    wifi_config.sta.threshold.rssi = -127;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    sema_get_ip_addrs = xSemaphoreCreateBinary();
    if (sema_get_ip_addrs == NULL)
    {
        return ESP_ERR_NO_MEM;
    }
    this->ClearRetries();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &OnWiFiDisconnect, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &OnWifiGotIP, this));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &OnWiFiConnect, this));
    ESP_LOGI(TAG, "WiFi Config Initialized");

    ESP_LOGI(TAG, "Connecting to %s", this->ssid.c_str());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Waiting for IP(s)");
    xSemaphoreTake(sema_get_ip_addrs, portMAX_DELAY);
    return ESP_OK;
}