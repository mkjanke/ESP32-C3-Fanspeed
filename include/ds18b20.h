#ifndef DS18B20_H
#define DS18B20_H

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"

#define PARASITE_POWER  false
static Placeholder<OneWireNg_CurrentPlatform> _ow;

class tempSensor {
  
  public:
    tempSensor(uint8_t pin, uint8_t type )
    {
      _pin = pin;
    };
    
    float temperature = 0.0;
    int readCount = 0; 
    int errorCount = 0;
    uint8_t _pin;

    void begin()
    {      
      new (&_ow) OneWireNg_CurrentPlatform(_pin, false);
      DSTherm drv(_ow);
      drv.filterSupportedSlaves();

    }

    float readFahrenheit(){
        DSTherm drv(_ow);
        Placeholder<DSTherm::Scratchpad> _scrpd;
        long tempF;
        
        /* convert temperature on all sensors connected... */
        drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER);

        /* ...and read them one-by-one */
        for (const auto& id: (OneWireNg&)_ow) {
            if (printId(id)) {
                if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS)
                {
                    printScratchpad(_scrpd);
                    tempF = _scrpd.operator&()->getTemp();
                }
                else
                    Serial.println("  Invalid CRC!");
            }
        }
        Serial.println("----------");

      // float tempF = 42;
      if (!isnan(tempF)) {
        temperature =  (float)((tempF + 50)/100)/10 ;
//        Serial.println(temperature);
        readCount++;
        return true;
      } else {
        errorCount++;
        return false;
      }
    }


  static bool printId(const OneWireNg::Id& id)
  {
    const char *name = DSTherm::getFamilyName(id);

    Serial.print(id[0], HEX);
    for (size_t i = 1; i < sizeof(OneWireNg::Id); i++) {
        Serial.print(':');
        Serial.print(id[i], HEX);
    }
    if (name) {
        Serial.print(" -> ");
        Serial.print(name);
    }
    Serial.println();

    return (name != NULL);
  }

  static void printScratchpad(const DSTherm::Scratchpad& scrpd)
  {
    const uint8_t *scrpd_raw = scrpd.getRaw();

    Serial.print("  Scratchpad:");
    for (size_t i = 0; i < DSTherm::Scratchpad::LENGTH; i++) {
        Serial.print(!i ? ' ' : ':');
        Serial.print(scrpd_raw[i], HEX);
    }

    Serial.print("; Th:");
    Serial.print(scrpd.getTh());

    Serial.print("; Tl:");
    Serial.print(scrpd.getTl());

    Serial.print("; Resolution:");
    Serial.print(9 + (int)(scrpd.getResolution() - DSTherm::RES_9_BIT));

    long temp = scrpd.getTemp();
    Serial.print("; Temp:");
    if (temp < 0) {
        temp = -temp;
        Serial.print('-');
    }
    Serial.print(temp / 1000);
    Serial.print('.');
    Serial.print(temp % 1000);
    Serial.print(" C");

    Serial.println();
  }

};

#endif // DS18B20