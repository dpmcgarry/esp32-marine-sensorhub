#include "esp_netif_sntp.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include <src/InkbirdBLEClient/InkbirdBLEClient.h>
#include <src/MQTTClient/MQTTClient.h>
#include <src/Prefs/Prefs.h>
#include <src/RTDClient/RTDClient.h>
#include <src/ESPClient/ESPClient.h>
#include <MSHGlobal.h>


// Global Pointers
msh::InkbirdBLEClient *handler;
msh::Prefs *prefs;
msh::MQTTClient *mqtt;
msh::RTDClient *rtd;
msh::ESPClient *esp;

void OnScanComplete(BLEScanResults r) {
  Log.trace("OnScanComplete");
  handler->Scanning(false);
}

void BLETask(void *parameter) {
  BLEDevice::init("");
  Log.trace("BLE Initialized\n");
  Log.notice("BLE Scan Task Init");
  BLEScan *pBLEScan = BLEDevice::getScan();
  Log.trace("Got Scan Object\n");
  handler = new msh::InkbirdBLEClient(mqtt);
  pBLEScan->setAdvertisedDeviceCallbacks(handler);
  Log.trace("Set Callback Function");
  pBLEScan->setActiveScan(true);
  Log.trace("Set scan paramters");
  for (;;) {
    Log.trace("BLE Free Heap: %l", esp_get_free_heap_size());
    vTaskDelay(BLE_DELAY_MS / portTICK_PERIOD_MS);
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

void RTDTask(void *parameter) {
  rtd = new msh::RTDClient(prefs->RTDPin(), mqtt);
  while (true) {
    Log.info("Reading RTD");
    Log.trace("RTD Free Heap: %l", esp_get_free_heap_size());
    rtd->Read();
    rtd->LogCounters();
    if (rtd->HasErrors()) {
      Log.warning("RTD has errors");
      rtd->LogLastError();
    }
    vTaskDelay(RTD_DELAY_MS / portTICK_PERIOD_MS);
  }
}

void ESPTask(void *paramter) {
  while (true) {
    Log.notice("Main Task Execution Loop");
    if (!WiFi.isConnected()) {
      wifiReconnectCount++;
      neopixelWrite(RGB_BUILTIN, 255, 0, 0);
      ConnectToWiFi(prefs);
    }
    if (mqtt == NULL) {
      hasResetMQTT = true;
      Log.warning("MQTT Client Object was NULL. Re-initializing");
      neopixelWrite(RGB_BUILTIN, 255, 255, 0);
      mqtt = new msh::MQTTClient(prefs->MQTTUri());
      if (prefs->UsePrivateCA()) {
        mqtt->MQTTCA(prefs->MQTTPrivateCA());
      }
      mqtt->Connect();
      vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
    }
    if (!mqtt->Connected()) {
      Log.warning("MQTT Client is no longer connected");
      neopixelWrite(RGB_BUILTIN, 255, 255, 0);
      if (!WiFi.isConnected()) {
        Log.warning("Wifi isn't connected so waiting for that to get sorted.");
      } else {
        Log.warning("Wifi is connected. Waiting for MQTT to autoreconnect.");
      }
    }
    if (WiFi.isConnected() && mqtt != NULL && mqtt->Connected()) {
      neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
      delay(100);
      neopixelWrite(RGB_BUILTIN, 0, 0, 0);
      delay(100);
      neopixelWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
    }
    esp->Read();
    vTaskDelay(MAIN_DELAY_MS / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && !Serial.available()) {
  }
  Log.setPrefix(printPrefix);  // set prefix similar to NLog
  Log.setSuffix(printSuffix);  // set suffix
  // Default to verbose logging until the level is set from prefs
  Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
  // Set LED to Blue for cold boot
  neopixelWrite(RGB_BUILTIN, 0, 0, 255);
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
  esp = new msh::ESPClient(mqtt, prefs);
  if (prefs->BLEEnabled()) {
    xTaskCreate(BLETask, "BLETask", 5000, NULL, 1, NULL);
  }

  if (prefs->RTDEnabled()) {
    xTaskCreate(RTDTask, "RTDTask", 5000, NULL, 1, NULL);
  }
  // There is something screwy with directly setting the NTP server
  // Copying the string locally and then converting to c string works
  String ntpServer = prefs->NTPServer();
  Log.notice("Setting NTP Server to %s", ntpServer.c_str());
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(ntpServer.c_str());
  esp_netif_sntp_init(&config);
  vTaskDelay(MAIN_START_DELAY_MS / portTICK_PERIOD_MS);
  xTaskCreate(ESPTask, "ESPTask", 5000, NULL, 1, NULL);
}

void loop() {
  vTaskDelete(NULL);
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

  // Set RGB to yellow
  neopixelWrite(RGB_BUILTIN, 255, 255, 0);
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

void printSuffix(Print *_logOutput, int logLevel) {
  _logOutput->print("\n");
}
