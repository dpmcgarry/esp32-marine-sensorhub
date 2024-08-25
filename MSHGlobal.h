#pragma once

// Constants
static const u16_t BLE_DELAY_MS = 500;
static const u16_t RTD_DELAY_MS = 100;
static const u16_t MAIN_START_DELAY_MS = 3000;
static const u16_t MAIN_DELAY_MS = 5000;
static const String MSH_VERSION = "1.0.0";

// Globals
static bool hasResetMQTT = false;
static u16_t wifiReconnectCount = 0;
static u16_t mqttReconnectCount = 0;