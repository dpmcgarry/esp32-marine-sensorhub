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

void InkbirdBLEClient::Scanning(bool scan) {
  this->isScanning = scan;
}

bool InkbirdBLEClient::Scanning() {
  return this->isScanning;
}

void InkbirdBLEClient::onResult(BLEAdvertisedDevice advertisedDevice) {
  String mfrData;
  if (advertisedDevice.haveServiceUUID()) {
    Log.trace("BLE Service ID: %s",
              advertisedDevice.getServiceUUID().toString().c_str());
  }
  if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID)) && advertisedDevice.haveName() && std::find(DEVICE_NAMES.begin(), DEVICE_NAMES.end(), advertisedDevice.getName()) != DEVICE_NAMES.end()) {
    Log.notice("BLE Device Found with matching service and name.");
    Log.trace("Discovered BLE Advertised Device: Name: %s, Address: %s",
              advertisedDevice.getName().c_str(),
              advertisedDevice.getAddress().toString().c_str());
    Log.trace("BLE Has Data: %d", advertisedDevice.haveManufacturerData());
    mfrData = advertisedDevice.getManufacturerData();
    for (int i = 0; i < mfrData.length(); i++) {
      Log.trace("BLE Mfr Data at byte [%d]", mfrData[i]);
    }
    String address = advertisedDevice.getAddress().toString();
    Log.trace("BLE Address: %s", address.c_str());
    long lsb = (long)mfrData.charAt(0);
    Log.trace("BLE LSB: %l", lsb);
    long msb = (long)mfrData.charAt(1);
    Log.trace("BLE MSB: %l", msb);
    msb = msb << 8;
    Log.trace("BLE MSB: %l", msb);
    float temp = (float)(lsb + msb) / 100.0;
    Log.trace("BLE Temperature C: %F", temp);
    float temp_f = ((temp * 9 / 5) + 32);
    Log.trace("BLE Temperature F: %F", temp_f);
    float battery = (float)mfrData.charAt(7);
    Log.trace("BLE Battery: %F", battery);
    lsb = (long)mfrData.charAt(2);
    Log.trace("BLE LSB: %l", lsb);
    msb = (long)mfrData.charAt(3);
    Log.trace("BLE MSB: %l", msb);
    msb = msb << 8;
    Log.trace("BLE MSB: %l", msb);
    float humidity = (float)(lsb + msb) / 100.0;
    Log.trace("BLE Humidity: %F", humidity);
    int rssi = advertisedDevice.getRSSI();
    Log.trace("BLE RSSI: %d", rssi);
    InkbirdMessage tMsg;
    tMsg.MAC = address;
    tMsg.TempC = temp;
    tMsg.TempF = temp_f;
    tMsg.BatteryPercent = battery;
    tMsg.Humidity = humidity;
    tMsg.RSSI = rssi;
    String j_str = tMsg.ToJSONString();
    Log.trace("BLE JSON: %s", j_str.c_str());
    String topic_str = bleTemp_subtopic + "/" + address;
    Log.trace("Using BLE subtopic: %s", topic_str.c_str());
    if (this->mqtt != NULL) {
      if (this->mqtt->Connected()) {
        Log.notice("MQTT is Connected. Will publish BLE message to topic: %s",
                   topic_str.c_str());
        this->mqtt->Publish(topic_str, j_str, 0, 1, 1);
      } else {
        Log.warning("MQTT is NOT Connected to send BLE.");
      }
    } else {
      Log.warning("MQTT is NULL to send BLE.");
    }
  }
}