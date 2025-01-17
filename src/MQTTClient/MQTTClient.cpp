#include "src/MQTTClient/MQTTClient.h"
using namespace msh;

MQTTClient::MQTTClient() {
  this->connected = false;
}

MQTTClient::MQTTClient(const String &connect_uri) {
  this->connect_uri = connect_uri;
  this->connected = false;
}

String MQTTClient::URI() {
  return this->connect_uri;
}

void MQTTClient::URI(const String &connect_uri) {
  this->connect_uri = connect_uri;
}

void MQTTClient::Connected(bool connected) {
  this->connected = connected;
}

bool MQTTClient::Connected() {
  return this->connected;
}

void MQTTClient::MQTTCA(const String &mqtt_ca) {
  this->mqtt_pem = mqtt_ca;
}

void MQTTClient::RootTopic(String root_topic) {
  this->root_topic = root_topic;
}

String MQTTClient::RootTopic() {
  return this->root_topic;
}

int MQTTClient::ConnectAttempts(){
    return this->conn_attempts;
}

int MQTTClient::DisconnectEvents(){
    return this->discon_events;
}

void MQTTClient::OnConnectAttempt(){
    this->conn_attempts++;
}

void MQTTClient::OnDisconnectEvent(){
    this->discon_events++;
}

int MQTTClient::Publish(const String &topic, const String &data, int len,
                        int qos, int retain) {
  if (this->root_topic != NULL) {
    String constructed_topic = this->root_topic + "/" + topic;
    return esp_mqtt_client_publish(this->client, constructed_topic.c_str(),
                                   data.c_str(), len, qos, retain);
  } else {
    return esp_mqtt_client_publish(this->client, topic.c_str(), data.c_str(),
                                   len, qos, retain);
  }
}

int MQTTClient::Subscribe(const String &topic, int qos) {
  return esp_mqtt_client_subscribe(this->client, topic.c_str(), qos);
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    Log.error("Last error %s: 0x%x", message, error_code);
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
static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  MQTTClient *mqtt = NULL;
  mqtt = (MQTTClient *)arg;
  // Log.trace("Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
  //           base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      Log.notice("MQTT_EVENT_CONNECTED");
      Log.notice("Connected to URI: %s", mqtt->URI().c_str());
      mqtt->Connected(true);
      break;
    case MQTT_EVENT_DISCONNECTED:
      mqtt->Connected(false);
      mqtt->OnDisconnectEvent();
      Log.notice("MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      Log.notice("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      Log.notice("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      Log.notice("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_DATA:
      Log.notice("MQTT_EVENT_DATA");
      Log.notice("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      Log.notice("DATA=%.*s\r\n", event->data_len, event->data);
      break;
    case MQTT_EVENT_ERROR:
      Log.error("MQTT_EVENT_ERROR");
      if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
        log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
        log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
        log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
        ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
      }
      break;
    case MQTT_EVENT_BEFORE_CONNECT:
        Log.notice("MQTT_EVENT_BEFORE_CONNECT");
        mqtt->OnConnectAttempt();
    default:
      Log.notice("Other event id:%d", event->event_id);
      break;
  }
}

void MQTTClient::Connect() {
  Log.notice("Connecting MQTT");
  struct esp_mqtt_client_config_t mqtt_cfg;
  memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
  if (this->mqtt_pem != NULL) {
    mqtt_cfg.broker.verification.certificate = this->mqtt_pem.c_str();
  }
  Log.notice("Connecting to endpoint: %s", this->connect_uri.c_str());
  mqtt_cfg.broker.address.uri = this->connect_uri.c_str();
  this->client = esp_mqtt_client_init(&mqtt_cfg);
  Log.notice("MQTT Client Init\n");
  esp_mqtt_client_register_event(this->client, MQTT_EVENT_ANY,
                                 mqtt_event_handler, this);
  Log.notice("MQTT Register Event\n");
  esp_mqtt_client_start(this->client);
}