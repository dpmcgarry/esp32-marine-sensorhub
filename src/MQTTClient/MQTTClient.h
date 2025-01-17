#pragma once

#include "mqtt_client.h"
#include <ArduinoLog.h>
namespace msh {
class MQTTClient {
public:
  MQTTClient();
  MQTTClient(const String &connect_uri);
  String URI();
  void URI(const String &connect_uri);
  void Connect();
  void Connected(bool);
  bool Connected();
  void RootTopic(String root_topic);
  String RootTopic();
  void MQTTCA(const String &mqtt_ca);
  int Publish(const String &topic, const String &data, int len, int qos,
              int retain);
  int Subscribe(const String &topic, int qos);
  int ConnectAttempts();
  int DisconnectEvents();
  void OnConnectAttempt();
  void OnDisconnectEvent();

private:
  String connect_uri;
  esp_mqtt_client_handle_t client;
  bool connected;
  String root_topic;
  String mqtt_pem;
  u16_t conn_attempts;
  u16_t discon_events;
};

typedef struct {
  String topic;
  String data;
  int qos;
  int retain;
} mqtt_message;
};  // namespace msh