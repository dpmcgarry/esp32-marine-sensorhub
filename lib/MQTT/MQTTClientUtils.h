#pragma once

#include "mqtt_client.h"
#include "esp_log.h"

#include <string>
#include <string.h>

class MQTTClientUtils
{
    public:
    MQTTClientUtils();
    MQTTClientUtils(const std::string &connect_uri);
    std::string GetURI();
    void SetURI(const std::string &connect_uri);
    void Connect();
    void SetConnected(bool);
    bool IsConnected();
    int Publish(const std::string &topic, const std::string &data, int len, int qos, int retain);
    int Subscribe(const std::string &topic, int qos);

    private:
    std::string connect_uri;
    esp_mqtt_client_handle_t client;
    bool connected;

};