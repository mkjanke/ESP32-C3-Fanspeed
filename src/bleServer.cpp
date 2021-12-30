#include "settings.h"
#include "fan.h"
#include "bleInterface.h"

// Bluetooth related defines and classes
#include <NimBLEDevice.h>

bool deviceConnected = false;

extern char uptimeBuffer[20];
extern float dhtTempA;
extern float dhtTempB;

extern PWMFan fan;

BLEServer *pServer;
BLEService *pService;
BLEAdvertising *pAdvertising;

BLECharacteristic* temperatureA_BLEC;
BLECharacteristic* temperatureB_BLEC;
BLECharacteristic* uptimeBLEC;
BLECharacteristic* fanspeedBLEC;
BLECharacteristic* fanStartTempBLEC;
BLECharacteristic* fanMaxTempBLEC;
BLECharacteristic* fanLowSpeedBLEC;
BLECharacteristic* fanOverrideBLEC;
BLECharacteristic* fanRelayBLEC;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// General purpose callback for handling Integer characteristic writes
class intCallbacks: public BLECharacteristicCallbacks {
  private:
    void *intValue;
    size_t size;

  public:
    intCallbacks(void *val, size_t sz){
      intValue = val;
      size = sz;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      memcpy((void *)intValue, (void *)value.c_str(), size );
    }
};

void bleInterface::update(){    
    temperatureB_BLEC->setValue((uint8_t*)&dhtTempB, sizeof(float));
    temperatureA_BLEC->setValue((uint8_t*)&dhtTempA, sizeof(float));
    fanspeedBLEC->setValue((uint8_t*)&fan.fanSpeed, sizeof(uint8_t));
    fanStartTempBLEC->setValue((uint8_t*)&fan.startTemp, sizeof(uint8_t));
    fanMaxTempBLEC->setValue((uint8_t*)&fan.maxTemp, sizeof(uint8_t));
    fanLowSpeedBLEC->setValue(fan.lowSpeed);
    fanOverrideBLEC->setValue(fan.override);
    fanRelayBLEC->setValue(fan.relayOut);
    uptimeBLEC->setValue(uptimeBuffer);
}

void bleInterface::stopAdvertising(){
    pAdvertising->stop();
    pAdvertising->setScanResponse(false);
}

void bleInterface::begin(){

  BLEDevice::init(DEVICE_NAME);   //Create BLE device 
  pServer = BLEDevice::createServer();   //Create BLE server 
  pServer->setCallbacks(new MyServerCallbacks());   //Set the callback function of the server 
  pService = pServer->createService(SERVICE_UUID); //Create BLE service 

  fanStartTempBLEC = pService->createCharacteristic(
           FANSTART_UUID,
           NIMBLE_PROPERTY::READ|
           NIMBLE_PROPERTY::WRITE);
  fanMaxTempBLEC = pService->createCharacteristic(
           FANMAX_UUID,
           NIMBLE_PROPERTY::READ|
           NIMBLE_PROPERTY::WRITE);
  temperatureA_BLEC = pService->createCharacteristic(
           TEMPERATURE_UUID,
           NIMBLE_PROPERTY::READ);
  temperatureB_BLEC = pService->createCharacteristic(
           HUMIDITY_UUID,
           NIMBLE_PROPERTY::READ);
  uptimeBLEC = pService->createCharacteristic(
           UPTIME_UUID,
           NIMBLE_PROPERTY::READ);
  fanspeedBLEC = pService->createCharacteristic(
           FANSPEED_UUID,
           NIMBLE_PROPERTY::READ);
  fanLowSpeedBLEC = pService->createCharacteristic(
           FANLOW_UUID,
           NIMBLE_PROPERTY::READ|
           NIMBLE_PROPERTY::WRITE);
  fanOverrideBLEC = pService->createCharacteristic(
           FANOVERRIDE_UUID,
           NIMBLE_PROPERTY::READ|
           NIMBLE_PROPERTY::WRITE);
  fanRelayBLEC = pService->createCharacteristic(
           RELAY_UUID,
           NIMBLE_PROPERTY::READ|
           NIMBLE_PROPERTY::WRITE);

  fanStartTempBLEC->setCallbacks(new intCallbacks(&fan.startTemp, sizeof(uint8_t)) );    //Fan Start Temperature 
  fanMaxTempBLEC->setCallbacks(new intCallbacks(&fan.maxTemp, sizeof(uint8_t)) );    //Fan Max Temperature 
  fanLowSpeedBLEC->setCallbacks(new intCallbacks(&fan.lowSpeed, sizeof(uint8_t)) );
  fanOverrideBLEC->setCallbacks(new intCallbacks(&fan.override, sizeof(bool)) );
  fanRelayBLEC->setCallbacks(new intCallbacks(&fan.relayOut, sizeof(bool)) );

  NimBLEDescriptor* temperatureBLEDesc;
  NimBLEDescriptor* humidityBLEDesc;
  NimBLEDescriptor* uptimeBLEDesc;
  NimBLEDescriptor* fanspeedBLEDesc;
  NimBLEDescriptor* fanStartTempBLEDesc;
  NimBLEDescriptor* fanLowSpeedBLECDesc;
  NimBLEDescriptor* fanMaxTempBLEDesc;
  NimBLEDescriptor* fanOverrideBLEDesc;
  NimBLEDescriptor* fanRelayBLEDesc;

  temperatureBLEDesc = temperatureA_BLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                          25);
  temperatureBLEDesc->setValue("DHT11 A Temp");

  humidityBLEDesc = temperatureB_BLEC->createDescriptor("2901", 
                                         NIMBLE_PROPERTY::READ,
                                           25);
  humidityBLEDesc->setValue("DHT11 B Temp");
  
  uptimeBLEDesc = uptimeBLEC->createDescriptor("2901", 
                                         NIMBLE_PROPERTY::READ,
                                           25);
  uptimeBLEDesc->setValue("Device Uptime");
  
  fanspeedBLEDesc = fanspeedBLEC->createDescriptor("2901", 
                                         NIMBLE_PROPERTY::READ,
                                           25);
  fanspeedBLEDesc->setValue("Fan Speed");
  
  fanStartTempBLEDesc = fanStartTempBLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                            25);
  fanStartTempBLEDesc->setValue("Fan Start Temp");
  
  fanLowSpeedBLECDesc = fanLowSpeedBLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                            25);
  fanLowSpeedBLECDesc->setValue("Fan Low Speed");
  
  fanMaxTempBLEDesc = fanMaxTempBLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                            25);
  fanMaxTempBLEDesc->setValue("Fan Max Temp");
  
  fanOverrideBLEDesc = fanOverrideBLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                            25);
  fanOverrideBLEDesc->setValue("Fan Override");
  
  fanRelayBLEDesc = fanRelayBLEC->createDescriptor("2901", 
                                          NIMBLE_PROPERTY::READ,
                                            25);
  fanRelayBLEDesc->setValue("Relay");


  pService->start();
  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  pAdvertising->setScanResponse(true);
}
