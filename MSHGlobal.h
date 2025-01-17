#pragma once

// Constants
static const u16_t BLE_DELAY_MS = 500;
static const u16_t RTD_DELAY_MS = 100;
static const u16_t MAIN_START_DELAY_MS = 3000;
static const u16_t MAIN_DELAY_MS = 5000;

#ifndef MSH_VERSION
// It's the answer to everything
#define MSH_VERSION "42.42.42"
#endif

// Globals
static bool hasResetMQTT = false;
static u16_t wifiReconnectCount = 0;