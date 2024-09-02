#include <src/RTDClient/RTDClient.h>

using namespace msh;

static const String rtdTemp_subtopic = "rtd/temperature";

RTDClient::RTDClient() {
  this->mqtt = NULL;
  this->rtdPin = -1;
  this->hasErrors = false;
  this->lastError = 0;
  this->seqErrors = 0;
  this->cumErrors = 0;
}

RTDClient::RTDClient(int RTDPin, msh::MQTTClient *mqtt) {
  this->mqtt = mqtt;
  this->rtdPin = RTDPin;
  this->hasErrors = false;
  this->lastError = 0;
  this->seqErrors = 0;
  this->cumErrors = 0;
  Log.info("Setting RTD Pin to %d", this->rtdPin);
  thermo = new Adafruit_MAX31865(this->rtdPin);
  thermo->begin(MAX31865_3WIRE);
}

bool RTDClient::HasErrors() {
  return this->hasErrors;
}

uint8_t RTDClient::LastError() {
  return this->lastError;
}

uint8_t RTDClient::SequentialErrors() {
  return this->seqErrors;
}

uint16_t RTDClient::CumulativeErrors() {
  return this->cumErrors;
}

void RTDClient::LogLastError() {
  Log.warning("Fault 0x%.2X", this->lastError);
  if (this->lastError & MAX31865_FAULT_HIGHTHRESH) {
    Log.warning("RTD High Threshold");
  }
  if (this->lastError & MAX31865_FAULT_LOWTHRESH) {
    Log.warning("RTD Low Threshold");
  }
  if (this->lastError & MAX31865_FAULT_REFINLOW) {
    Log.warning("REFIN- > 0.85 x Bias");
  }
  if (this->lastError & MAX31865_FAULT_REFINHIGH) {
    Log.warning("REFIN- < 0.85 x Bias - FORCE- open");
  }
  if (this->lastError & MAX31865_FAULT_RTDINLOW) {
    Log.warning("RTDIN- < 0.85 x Bias - FORCE- open");
  }
  if (this->lastError & MAX31865_FAULT_OVUV) {
    Log.warning("Under/Over voltage");
  }
}

void RTDClient::LogCounters() {
  Log.notice("Cumulative Errors: %u", this->cumErrors);
  Log.notice("Has Errors: %s", this->hasErrors ? "true" : "false");
  Log.notice("Last Error: %u", this->lastError);
  Log.notice("Sequential Errors: %u", this->seqErrors);
}

bool RTDClient::Read() {
  if (this->thermo == NULL) {
    Log.error("Thermo is not initialized!");
    return false;
  }
  uint16_t rtd = thermo->readRTD();
  Log.trace("RTD value: %u", rtd);
  float ratio = rtd;
  ratio /= 32768;
  Log.trace("Ratio = %F", ratio);
  Log.trace("Resistance = %F", RREF * ratio);

  // Check and print any faults
  uint8_t fault = thermo->readFault();
  if (fault) {
    thermo->clearFault();
    this->cumErrors++;
    this->hasErrors = true;
    this->lastError = fault;
    this->seqErrors++;
    return false;
  }
  this->hasErrors = false;
  this->seqErrors = 0;

  RTDMessage msg = RTDMessage();

  msg.TempC = thermo->temperature(RNOMINAL, RREF);
  Log.notice("RTD Temperature C = %F", msg.TempC);
  msg.TempF = ((msg.TempC * 9 / 5) + 32);
  Log.notice("RTD Temperature F = %F", msg.TempF);
  if (this->mqtt != NULL) {
    if (this->mqtt->Connected()) {
      Log.trace("MQTT is connected. Will send RTD Message");
      msg.MAC = WiFi.macAddress();
      String j_str = msg.ToJSONString();
      Log.trace("RTD JSON: %s", j_str.c_str());
      String macstrip;
      for (int i = 0; i < msg.MAC.length(); i++) {
        if (msg.MAC[i] != ':') {
          macstrip += msg.MAC[i];
        }
      }
      String topic_str = rtdTemp_subtopic + "/" + macstrip;
      Log.trace("Using RTD Subtopic: %s", topic_str);
      this->mqtt->Publish(topic_str, j_str, 0, 1, 1);
    } else {
      Log.warning("MQTT is not connected to send RTD.");
    }
  } else {
    Log.warning("MQTT is NULL to send RTD");
  }
  return true;
}