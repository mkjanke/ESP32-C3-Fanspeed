#ifndef DS18B20_H
#define DS18B20_H

#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"

#define PARASITE_POWER false
static Placeholder<OneWireNg_CurrentPlatform> _ow;

// Class to encapsulate a DS18B20 sensor pair.
// Temperature A is will always be the higher temperature sensor,
// so that the fans will spin in an attempt to lower that temp.
// Temperature B is the base (ambient) temperature sensor, presumed to be lower
// temperature
//
// Will find the first two DS sensor on a single OneWire bus.
// More than two sensor == untested.

class sensorPair {
 private:
  uint8_t _pin;

 public:
  sensorPair(uint8_t pin, uint8_t pinB, uint8_t type) {
    // 'pinB' is a dummy paraameter, to be consistent with DHT11 version
    _pin = pin;
  };

  // Temperatures will be in either fahrenheit or celsius, depending on last
  // read.
  float temperatureA = 0.0;
  float temperatureB = 0.0;

  // Flags will be set to whichever scale was read last
  bool isCelsius;
  bool isFahrenheit;

  int readCount = 0;
  int errorCount = 0;

  void begin() {
    new (&_ow) OneWireNg_CurrentPlatform(_pin, false);
    DSTherm drv(_ow);
    drv.filterSupportedSlaves();
  }

  bool readFahrenheit() {
    isFahrenheit = true;
    isCelsius = false;
    return readTemperature(TEMP_FAHRENHEIT);
  }
  bool readCelsius() {
    isFahrenheit = false;
    isCelsius = true;
    return readTemperature(false);
  }

 private:
  bool readTemperature(bool fahrenheit) {
    DSTherm drv(_ow);
    Placeholder<DSTherm::Scratchpad> _scrpd;

    // Only handle two sensors. Behaivior iwth > 2 undifined.
    // temperatures returned from driver are in degrees C * 1000,
    // stored in long integer
    long tempT[2];
    uint8_t i = 0;

    /* convert temperature on all sensors connected... */
    drv.convertTempAll(DSTherm::SCAN_BUS, PARASITE_POWER);

    /* ...and read them one-by-one */
    for (const auto& id : (OneWireNg&)_ow) {
      if (printId(id)) {
        if (drv.readScratchpad(id, &_scrpd) == OneWireNg::EC_SUCCESS) {
          printScratchpad(_scrpd);
          tempT[i] = _scrpd.operator&()->getTemp();
          i++;
        } else {
          Serial.println("  Invalid CRC!");
          errorCount++;
          return false;
        }
      }
    }
    Serial.println("----------");

    if (!isnan(tempT[0]) && !isnan(tempT[1])) {
      readCount++;
      if (fahrenheit) {
        tempT[0] = (9.0 * tempT[0] / 5.0) + 32000;
        tempT[1] = (9.0 * tempT[1] / 5.0) + 32000;
      }
      if (tempT[0] > tempT[1]) {
        temperatureA = (float)((tempT[0] + 50) / 100) / 10;
        temperatureB = (float)((tempT[1] + 50) / 100) / 10;
      } else {
        temperatureA = (float)((tempT[1] + 50) / 100) / 10;
        temperatureB = (float)((tempT[0] + 50) / 100) / 10;
      }

      return true;
    } else
      return false;
  }

  static bool printId(const OneWireNg::Id& id) {
    const char* name = DSTherm::getFamilyName(id);

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

  static void printScratchpad(const DSTherm::Scratchpad& scrpd) {
    const uint8_t* scrpd_raw = scrpd.getRaw();

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

#endif  // DS18B20