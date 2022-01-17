#ifndef DHT11_H
#define DHT11_H

// DHT Sensor includes and defines
#include <Adafruit_Sensor.h>
#include <DHT.h>

// DHT Temperature Probe class
// Simple wrapper for Adafruit DHT class

class tempSensor : public DHT {
  
  public:
    tempSensor(uint8_t pin, uint8_t type ) : DHT( pin, type ){}
    
    float temperature = 0.0;
    int readCount = 0; 
    int errorCount = 0;

    bool readFahrenheit(){
      float tempF = DHT::readTemperature(TEMP_FAHRENHEIT);
      if (!isnan(tempF)) {
        temperature = (float)((int)(tempF * 10 + .5))/10;
        readCount++;
        return true;
      } else {
        errorCount++;
        return false;
      }
    }
};

class sensorPair {
  uint8_t _pinA;
  uint8_t _pinB;
  uint8_t _type;
  protected:
    tempSensor sensorA;
    tempSensor sensorB;

  public:
    float temperatureA = 0.0;
    float temperatureB = 0.0;
    int errorCount = 0;
    int readCount = 0;

    sensorPair(uint8_t pinA, uint8_t pinB, uint8_t type ) : 
                  sensorA(pinA, type), sensorB(pinB, type) {
      _pinA = pinA;
      _pinB = pinB;
    }

    void begin(){
       sensorA.begin();
       sensorB.begin();
    }

    bool readFahrenheit(){
      if (sensorA.readFahrenheit())
        temperatureA=sensorA.temperature;
      else {
        errorCount++;
        return false;
      }      
      if (sensorB.readFahrenheit())
        temperatureB=sensorB.temperature;
      else {
        errorCount++;
        return false;
      }
       readCount++;      
       return true;
    }

};


#endif //DHT11_H