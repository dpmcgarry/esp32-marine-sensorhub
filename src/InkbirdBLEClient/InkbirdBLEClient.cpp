#include <src/InkbirdBLEClient/InkbirdBLEClient.h>

using namespace msh;

static const String bleTemp_subtopic = "ble/temperature";

InkbirdBLEClient::InkbirdBLEClient() {
  this->mqtt = NULL;
  this->isScanning = false;
}

InkbirdBLEClient::InkbirdBLEClient(msh::MQTTClient *mqtt) {
  this->mqtt = mqtt;
  this->isScanning = false;
}

void InkbirdBLEClient::Scanning(bool scan) { this->isScanning = scan; }

bool InkbirdBLEClient::Scanning() { return this->isScanning; }

void InkbirdBLEClient::onResult(BLEAdvertisedDevice advertisedDevice) {
  String mfrData;
  if (advertisedDevice.haveServiceUUID()) {
    Log.trace("Service ID: %s",
              advertisedDevice.getServiceUUID().toString().c_str());
  }
  if (advertisedDevice.haveServiceUUID() &&
      advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID)) &&
      advertisedDevice.haveName() &&
      std::find(DEVICE_NAMES.begin(), DEVICE_NAMES.end(),
                advertisedDevice.getName()) != DEVICE_NAMES.end()) {
    Log.notice("Device Found with matching service and name.");
    Log.notice("Discovered Advertised Device: Name: %s, Address: %s",
               advertisedDevice.getName().c_str(),
               advertisedDevice.getAddress().toString().c_str());
    // mfrData = BLEUtils::buildHexData(nullptr,
    // (uint8_t*)advertisedDevice.getManufacturerData().data(),
    // advertisedDevice.getManufacturerData().length());
    Log.notice("Has Data: %d", advertisedDevice.haveManufacturerData());
    mfrData = advertisedDevice.getManufacturerData();
    for (int i = 0; i < mfrData.length(); i++) {
      Log.trace("Mfr Data at byte [%d]", mfrData[i]);
    }
    String address = advertisedDevice.getAddress().toString();
    Log.notice("Address: %s", address.c_str());
    long lsb = (long)mfrData.charAt(0);
    Log.trace("LSB: %l", lsb);
    long msb = (long)mfrData.charAt(1);
    Log.trace("MSB: %l", msb);
    msb = msb << 8;
    Log.trace("MSB: %l", msb);
    float temp = (float)(lsb + msb) / 100.0;
    Log.notice("Temperature C: %F", temp);
    float temp_f = ((temp * 9 / 5) + 32);
    Log.notice("Temperature F: %F", temp_f);
    float battery = (float)mfrData.charAt(7);
    Log.notice("Battery: %F", battery);
    lsb = (long)mfrData.charAt(2);
    Log.trace("LSB: %l", lsb);
    msb = (long)mfrData.charAt(3);
    Log.trace("MSB: %l", msb);
    msb = msb << 8;
    Log.trace("MSB: %l", msb);
    float humidity = (float)(lsb + msb) / 100.0;
    Log.notice("Humidity: %F", humidity);
    TemperatureMessage tMsg;
    tMsg.MAC = address;
    tMsg.TempC = temp;
    tMsg.TempF = temp_f;
    tMsg.BatteryPercent = battery;
    tMsg.Humidity = humidity;
    String j_str = tMsg.ToJSONString();
    Log.notice("JSON: %s", j_str.c_str());
    String topic_str = bleTemp_subtopic + "/" + address;
    Log.notice("Using subtopic: %s", topic_str.c_str());
    if (this->mqtt != NULL) {
      if (this->mqtt->Connected()) {
        Log.notice("MQTT is Connected. Will publish to topic: %s",
                   topic_str.c_str());
        this->mqtt->Publish(topic_str, j_str, 0, 1, 0);
      } else {
        Log.notice("MQTT is NOT Connected.");
      }
    } else {
      Log.notice("MQTT is NULL");
    }
  }
}