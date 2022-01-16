#ifndef DHT11_H
#define DHT11_H

// DHT Sensor includes and defines
#include <Adafruit_Sensor.h>
#include <DHT.h>

// DHT Temperature Probe class
// Simple wrapper for Adafruit DHT class

class myDHT : public DHT {
  
  public:
    myDHT(uint8_t pin, uint8_t type ) : DHT( pin, type ){}
    
    float temperature = 0.0;
    int readCount = 0; 
    int errorCount = 0;

    float readFahrenheit(){
      float tempF = DHT::readTemperature(true);
      if (!isnan(tempF)) {
        temperature = (float)((int)(tempF * 10 + .5)/10);
        readCount++;
        return true;
      } else {
        errorCount++;
        return false;
      }
    }
};

#endif //DHT11_H