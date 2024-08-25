#pragma once

#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <WiFi.h>
#include <Adafruit_MAX31865.h>

#include <src/MQTTClient/MQTTClient.h>
#include "RTDMessage.h"

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

namespace msh {
class RTDClient {
public:
  RTDClient();
  RTDClient(int RTDPin, msh::MQTTClient *mqtt);
  bool HasErrors();
  uint8_t LastError();
  uint8_t SequentialErrors();
  uint16_t CumulativeErrors();
  void LogLastError();
  void LogCounters();
  bool Read();


private:
  msh::MQTTClient *mqtt;
  int rtdPin;
  bool hasErrors;
  uint8_t lastError;
  uint8_t seqErrors;
  uint16_t cumErrors;
  Adafruit_MAX31865 *thermo;
};
};  // namespace msh