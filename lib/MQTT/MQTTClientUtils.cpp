#include "MQTTClientUtils.h"

static const char *const TAG = "esp32-temp-reporter-mqtt";

MQTTClientUtils::MQTTClientUtils()
{
    this->connected = false;
}

MQTTClientUtils::MQTTClientUtils(const std::string &connect_uri)
{
    this->connect_uri = connect_uri;
    this->connected = false;
}

std::string MQTTClientUtils::GetURI()
{
    return this->connect_uri;
}

void MQTTClientUtils::SetURI(const std::string &connect_uri)
{
    this->connect_uri = connect_uri;
}

void MQTTClientUtils::SetConnected(bool connected)
{
    this->connected = connected;
}

bool MQTTClientUtils::IsConnected()
{
    return this->connected;
}

int  MQTTClientUtils::Publish(const std::string &topic, const std::string &data, int len, int qos, int retain)
{
    return esp_mqtt_client_publish(this->client, topic.c_str(), data.c_str(), len, qos, retain);
}

int MQTTClientUtils::Subscribe(const std::string &topic, int qos)
{
    return esp_mqtt_client_subscribe(this->client, topic.c_str(), qos);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data)
{
    MQTTClientUtils *mqtt = NULL;
    mqtt = (MQTTClientUtils*)arg;
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        ESP_LOGI(TAG, "Connected to URI: %s", mqtt->GetURI().c_str());
        mqtt->SetConnected(true);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt->SetConnected(false);
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        mqtt->SetConnected(false);
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        mqtt->SetConnected(false);
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void MQTTClientUtils::Connect()
{
    ESP_LOGI(TAG, "Connecting MQTT");
    struct esp_mqtt_client_config_t mqtt_cfg = {0};
    ESP_LOGI(TAG, "Connecting to endpoint: %s", this->GetURI().c_str());
    mqtt_cfg.broker.address.uri = this->connect_uri.c_str();
    this->client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_LOGI(TAG, "MQTT Client Init\n");
    esp_mqtt_client_register_event(this->client, MQTT_EVENT_ANY, mqtt_event_handler, this);
    ESP_LOGI(TAG, "MQTT Register Event\n");
    esp_mqtt_client_start(this->client);

}