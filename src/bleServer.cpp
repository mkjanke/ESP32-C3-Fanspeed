#include "bleInterface.h"
#include "fan.h"
#include "settings.h"

// Bluetooth related defines and classes
#include <NimBLEDevice.h>

extern PWMFan fan;

BLEServer* pServer;
BLEService* pService;
BLEAdvertising* pAdvertising;

BLECharacteristic* temperatureA_BLEC;
BLECharacteristic* temperatureB_BLEC;
BLECharacteristic* uptimeBLEC;
BLECharacteristic* statusBLEC;
BLECharacteristic* fanspeedBLEC;
BLECharacteristic* fanStartTempBLEC;
BLECharacteristic* fanMaxTempBLEC;
BLECharacteristic* fanLowSpeedBLEC;
BLECharacteristic* fanOverrideBLEC;
BLECharacteristic* fanRelayBLEC;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { 
    bleInterface::deviceConnected = true; 
    pAdvertising->start();
    pAdvertising->setScanResponse(true);
};

  void onDisconnect(BLEServer* pServer) {
    bleInterface::deviceConnected = false;
  }
};

// General purpose callback for handling Integer characteristic writes
class intCallbacks : public BLECharacteristicCallbacks {
 private:
  void* intValue;
  size_t size;

 public:
  intCallbacks(void* val, size_t sz) {
    intValue = val;
    size = sz;
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    memcpy((void*)intValue, (void*)value.c_str(), size);
  }
};

// Update fan related characteristics with current fan parameters
void bleInterface::updateFan() {
  fanspeedBLEC->setValue(fan.fanSpeed);
  fanspeedBLEC->notify();
  fanStartTempBLEC->setValue(fan.startTemp);
  fanMaxTempBLEC->setValue(fan.maxTemp);
  fanLowSpeedBLEC->setValue(fan.lowSpeed);
  fanOverrideBLEC->setValue(fan.override);
  fanOverrideBLEC->notify();
  fanRelayBLEC->setValue(fan.relayOut);
  fanRelayBLEC->notify();
}

// Update temperature sensor characteristics with current temperature
void bleInterface::updateTemperature(float dhtTempA, float dhtTempB) {
  temperatureA_BLEC->setValue(dhtTempA);
  temperatureA_BLEC->notify();
  temperatureB_BLEC->setValue(dhtTempB);
  temperatureB_BLEC->notify();
}

// Update 'Uptime' characteristic with current device uptime
void bleInterface::updateUptime(char* buff) {
  uptimeBLEC->setValue((std::string)buff);
  uptimeBLEC->notify();
}

// Device Status
void bleInterface::updateStatus(const char* buff) {
  statusBLEC->setValue((std::string)buff);
  statusBLEC->notify();
}

void bleInterface::stopAdvertising() {
  pAdvertising->stop();
  pAdvertising->setScanResponse(false);
}

void bleInterface::begin() {
  // Create BLE device
  BLEDevice::init(DEVICE_NAME);
  // Create BLE server               
  pServer = BLEDevice::createServer();        

  // Set the callback function for this BLE instance
  pServer->setCallbacks(
      new MyServerCallbacks());

  // Create BLE service
  pService = pServer->createService(SERVICE_UUID); 

  // Temperature Characteristics
  temperatureA_BLEC = pService->createCharacteristic(
      TEMPERATURE_A_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor* temperatureA_BLEDesc;
  temperatureA_BLEDesc =
      temperatureA_BLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  temperatureA_BLEDesc->setValue("Sensor A Temp");

  temperatureB_BLEC = pService->createCharacteristic(
      TEMPERATURE_B_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor* temperatureB_BLEDesc;
  temperatureB_BLEDesc =
      temperatureB_BLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  temperatureB_BLEDesc->setValue("Sensor B Temp");

  // Uptime Characteristic
  uptimeBLEC = pService->createCharacteristic(
      UPTIME_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor* uptimeBLEDesc;
  uptimeBLEDesc =
      uptimeBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  uptimeBLEDesc->setValue("Device Uptime");

  // Characteristic to hold misc ESP32 Status messages
  statusBLEC = pService->createCharacteristic(
      STATUS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor* statusBLEDesc;
  statusBLEDesc =
      statusBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  statusBLEDesc->setValue("Device Status");


  // Fan Speed
  fanspeedBLEC = pService->createCharacteristic(
      FANSPEED_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor* fanspeedBLEDesc;
  fanspeedBLEDesc =
      fanspeedBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanspeedBLEDesc->setValue("Fan Speed");

  // Fan Start Temperature
  fanStartTempBLEC = pService->createCharacteristic(
      FANSTART_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  fanStartTempBLEC->setCallbacks(
      new intCallbacks(&fan.startTemp, sizeof(uint8_t)));

  NimBLEDescriptor* fanStartTempBLEDesc;
  fanStartTempBLEDesc =
      fanStartTempBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanStartTempBLEDesc->setValue("Fan Start Temp");

  // Fan Max Temperature
  fanMaxTempBLEC = pService->createCharacteristic(
      FANMAX_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  fanMaxTempBLEC->setCallbacks(new intCallbacks(&fan.maxTemp, sizeof(uint8_t)));
  NimBLEDescriptor* fanMaxTempBLEDesc;
  fanMaxTempBLEDesc =
      fanMaxTempBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanMaxTempBLEDesc->setValue("Fan Max Temp");

  // Fan low speed
  fanLowSpeedBLEC = pService->createCharacteristic(
      FANLOW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  fanLowSpeedBLEC->setCallbacks(
      new intCallbacks(&fan.lowSpeed, sizeof(uint8_t)));
  NimBLEDescriptor* fanLowSpeedBLECDesc;
  fanLowSpeedBLECDesc =
      fanLowSpeedBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanLowSpeedBLECDesc->setValue("Fan Low Speed");

  // Fan override
  fanOverrideBLEC = pService->createCharacteristic(
      FANOVERRIDE_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  fanOverrideBLEC->setCallbacks(new intCallbacks(&fan.override, sizeof(bool)));
  NimBLEDescriptor* fanOverrideBLEDesc;
  fanOverrideBLEDesc =
      fanOverrideBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanOverrideBLEDesc->setValue("Fan Override");

  // Fan relay
  fanRelayBLEC = pService->createCharacteristic(
      RELAY_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  fanRelayBLEC->setCallbacks(new intCallbacks(&fan.relayOut, sizeof(bool)));
  NimBLEDescriptor* fanRelayBLEDesc;
  fanRelayBLEDesc =
      fanRelayBLEC->createDescriptor("2901", NIMBLE_PROPERTY::READ, 25);
  fanRelayBLEDesc->setValue("Relay");

  pService->start();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID); 

  pAdvertising->start();
  pAdvertising->setScanResponse(true);
}
