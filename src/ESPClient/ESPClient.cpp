#include <src/ESPClient/ESPClient.h>

using namespace msh;

static const String esp_subtopic = "esp/status";

ESPClient::ESPClient() {
  this->mqtt = NULL;
  this->prefs = NULL;
}

ESPClient::ESPClient(msh::MQTTClient *mqtt, msh::Prefs *prefs) {
  this->mqtt = mqtt;
  this->prefs = prefs;
}

bool ESPClient::Read() {
  ESPMessage msg = ESPMessage();
  msg.FreeSRAM = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  Log.notice("Free SRAM: %z", msg.FreeSRAM);
  msg.FreeHeap = esp_get_free_heap_size();
  Log.notice("Free Heap: %l", msg.FreeHeap);
  msg.FreePSRAM = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  Log.notice("Free PSRAM: %z", msg.FreePSRAM);
  msg.HasResetMQTT = hasResetMQTT;
  Log.notice("MQTT has been re-initialized? %s",
             msg.HasResetMQTT ? "true" : "false");
  msg.WifiReconnectCount = wifiReconnectCount;
  Log.notice("WiFi Reconnect Count: %u", msg.WifiReconnectCount);
  msg.MQTTReconnectCount = mqttReconnectCount;
  Log.notice("MQTT Reconnect Count: %u", msg.MQTTReconnectCount);
  msg.IPAddress = WiFi.localIP().toString();
  Log.notice("Local IP: %s", msg.IPAddress);
  msg.WiFiRSSI = WiFi.RSSI();
  Log.notice("WiFi RSSI: %d", msg.WiFiRSSI);
  msg.MAC = WiFi.macAddress();
  Log.notice("MAC: %s", msg.MAC.c_str());
  struct tm timeinfo;
  msg.HasTime = getLocalTime(&timeinfo);
  Log.trace("Got time? %s", msg.HasTime ? "true" : "false");
  char timestamp[100];
  strftime(timestamp, sizeof(timestamp), "%FT%TZ", &timeinfo);
  msg.DeviceTime = timestamp;
  Log.notice("Time is: %s", msg.DeviceTime.c_str());
  msg.BLEEnabled = prefs->BLEEnabled();
  msg.RTDEnabled = prefs->RTDEnabled();
  msg.MSHVersion = MSH_VERSION;
  Log.notice("Version is: %s", msg.MSHVersion.c_str());
  if (mqtt != NULL) {
    if (mqtt->Connected()) {
      Log.trace("MQTT is connected. Will send ESP Message");
      String j_str = msg.ToJSONString();
      Log.trace("ESP JSON: %s", j_str.c_str());
      String macstrip;
      for (int i = 0; i < msg.MAC.length(); i++) {
        if (msg.MAC[i] != ':') {
          macstrip += msg.MAC[i];
        }
      }
      String topic_str = esp_subtopic + "/" + macstrip;
      Log.trace("Using ESP Subtopic: %s", topic_str);
      this->mqtt->Publish(topic_str, j_str, 0, 1, 1);
    } else {
      Log.warning("MQTT is not connected to send ESP.");
    }
  } else {
    Log.warning("MQTT is null to send ESP.");
  }
  return true;
}