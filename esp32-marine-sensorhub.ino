#include "nvs_flash.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include <src/InkbirdBLEClient/InkbirdBLEClient.h>
#include <src/MQTTClient/MQTTClient.h>
#include <src/Prefs/Prefs.h>

#undef TAG

IPAddress ip;

// Constants
static const u16_t DELAY_MS = 500;
static const u16_t MAIN_DELAY_MS = 30000;

// Globals
static bool hasResetMQTT = false;
static u16_t wifiReconnectCount = 0;
static u16_t mqttReconnectCount = 0;

// Global Pointers
msh::InkbirdBLEClient *handler;
msh::Prefs *prefs;
msh::MQTTClient *mqtt;

void OnScanComplete(BLEScanResults r) {
  Log.trace("OnScanComplete");
  handler->Scanning(false);
}

void BLETask(void *parameter) {
  msh::MQTTClient *mqtt_client = NULL;
  mqtt_client = (msh::MQTTClient *)parameter;
  BLEDevice::init("");
  Log.trace("BLE Initialized\n");
  Log.notice("BLE Scan Task Init");
  BLEScan *pBLEScan = BLEDevice::getScan();
  Log.trace("Got Scan Object\n");
  handler = new msh::InkbirdBLEClient(mqtt_client);
  pBLEScan->setAdvertisedDeviceCallbacks(handler);
  Log.trace("Set Callback Function");
  pBLEScan->setActiveScan(true);
  Log.trace("Set scan paramters");
  for (;;) {
    Log.notice("Free Heap: %l", esp_get_free_heap_size());
    vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    Log.trace("Rerunning scan");
    if (!handler->Scanning()) {
      Log.notice("BLE Scanner is not scanning. Starting scan.");
      handler->Scanning(true);
      pBLEScan->start(2, &OnScanComplete, false);
    } else {
      Log.notice("BLE Scanner is scanning. Continuing.");
    }
  }
}

void WatchdogTask(void *parameter) {}

void setup() {
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {
  }
  Log.setPrefix(printPrefix); // set prefix similar to NLog
  Log.setSuffix(printSuffix); // set suffix
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.notice("Setup");
  prefs = new msh::Prefs();
  prefs->Load();
  Log.setLevel(prefs->LogLevel());
  ConnectToWiFi(prefs);
  mqtt = new msh::MQTTClient(prefs->MQTTUri());
  mqtt->RootTopic(prefs->RootTopic());
  if (prefs->UsePrivateCA()) {
    mqtt->MQTTCA(prefs->MQTTPrivateCA());
  }
  mqtt->Connect();
  if (prefs->BLEEnabled()) {
    xTaskCreate(BLETask, "BLETask", 5000, mqtt, 1, NULL);
  }
  vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
}

void loop() {

  Log.notice("Main Task Execution Loop");
  Log.notice("Free SRAM: %z", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  Log.notice("Free Heap: %l", esp_get_free_heap_size());
  Log.notice("Free PSRAM: %z", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  Log.notice("MQTT has been re-initialized? %s",
             hasResetMQTT ? "true" : "false");
  Log.notice("WiFi Reconnect Count: %u", wifiReconnectCount);
  Log.notice("MQTT Reconnect Count: %u", mqttReconnectCount);
  if (!WiFi.isConnected()) {
    wifiReconnectCount++;
    ConnectToWiFi(prefs);
  }
  if (mqtt == NULL) {
    hasResetMQTT = true;
    Log.warning("MQTT Client Object was NULL. Re-initializing");
    mqtt = new msh::MQTTClient(prefs->MQTTUri());
    if (prefs->UsePrivateCA()) {
      mqtt->MQTTCA(prefs->MQTTPrivateCA());
    }
    mqtt->Connect();
    vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
  }
  if (!mqtt->Connected()) {
    Log.warning("MQTT Client is no longer connected");
    if (!WiFi.isConnected()) {
      Log.warning("Wifi isn't connected so waiting for that to get sorted.");
    } else {
      Log.warning("Wifi is connected. Attempting to reconnect MQTT.");
      mqttReconnectCount++;
      mqtt->Connect();
      vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
    }
  }
#ifdef RGB_BUILTIN
  // digitalWrite(RGB_BUILTIN, HIGH);  // Turn the RGB LED white
  // delay(1000);
  // digitalWrite(RGB_BUILTIN, LOW);  // Turn the RGB LED off
  // delay(1000);

  neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0); // Red
  delay(1000);
  neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0); // Green
  delay(1000);
  neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS); // Blue
  delay(1000);
  neopixelWrite(RGB_BUILTIN, 0, 0, 0); // Off / black
  delay(1000);
#endif
}

void ConnectToWiFi(msh::Prefs *prefs) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setAutoReconnect(true);
  if (prefs->StaticIPEnabled()) {
    Log.notice("Setting Static IP Info");
    IPAddress ip = IPAddress(prefs->IPAddress().c_str());
    IPAddress gateway = IPAddress(prefs->IPGateway().c_str());
    IPAddress netmask = IPAddress(prefs->Netmask().c_str());
    IPAddress dns1 = IPAddress((u32_t)0);
    IPAddress dns2 = IPAddress((u32_t)0);
    if (prefs->DNS1() != NULL) {
      dns1 = IPAddress(prefs->DNS1().c_str());
    } else {
      Log.notice("DNS1 is NULL");
    }
    if (prefs->DNS2() != NULL) {
      dns2 = IPAddress(prefs->DNS2().c_str());
    } else {
      Log.notice("DNS2 is NULL");
    }
    Log.notice("Configuring IP Info: IP: %s, GW: %s, SubnetMask: %s, DNS1: %s, "
               "DNS2: %s",
               ip.toString(), gateway.toString(), netmask.toString(),
               dns1.toString(), dns2.toString());
    WiFi.config(ip, gateway, netmask, dns1, dns2);
  }
  WiFi.begin(prefs->SSID(), prefs->SSIDPassword());
  Log.notice("Connecting to WiFi SSID: %s", prefs->SSID().c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Log.notice(".");
    delay(1000);
  }
  Log.notice("Got IP: %s", WiFi.localIP().toString());
  Log.notice("MAC Address: %s", WiFi.macAddress().c_str());
}

void printPrefix(Print *_logOutput, int logLevel) {
  printTimestamp(_logOutput);
  printLogLevel(_logOutput, logLevel);
}

void printTimestamp(Print *_logOutput) {

  // Division constants
  const unsigned long MSECS_PER_SEC = 1000;
  const unsigned long SECS_PER_MIN = 60;
  const unsigned long SECS_PER_HOUR = 3600;
  const unsigned long SECS_PER_DAY = 86400;

  // Total time
  const unsigned long msecs = millis();
  const unsigned long secs = msecs / MSECS_PER_SEC;

  // Time in components
  const unsigned long MilliSeconds = msecs % MSECS_PER_SEC;
  const unsigned long Seconds = secs % SECS_PER_MIN;
  const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
  const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

  // Time as string
  char timestamp[20];
  sprintf(timestamp, "%02d:%02d:%02d.%03d ", Hours, Minutes, Seconds,
          MilliSeconds);
  _logOutput->print(timestamp);
}

void printLogLevel(Print *_logOutput, int logLevel) {
  /// Show log description based on log level
  switch (logLevel) {
  default:
  case 0:
    _logOutput->print("SILENT ");
    break;
  case 1:
    _logOutput->print("FATAL ");
    break;
  case 2:
    _logOutput->print("ERROR ");
    break;
  case 3:
    _logOutput->print("WARNING ");
    break;
  case 4:
    _logOutput->print("INFO ");
    break;
  case 5:
    _logOutput->print("TRACE ");
    break;
  case 6:
    _logOutput->print("VERBOSE ");
    break;
  }
}

void printSuffix(Print *_logOutput, int logLevel) { _logOutput->print("\n"); }
