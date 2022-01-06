#ifndef FAN_H
#define FAN_H

#include "settings.h"
#include <stdint.h>

#include "esp32-hal-ledc.h"   // lcdSetup()
#include "driver/gpio.h"      // gpio functions

class PWMFan {
  public:
    uint8_t dutyCycle = PWM_MAX_DUTY_CYCLE;   // Max PWM duty cycle (= 255)
    uint8_t fanSpeed = FAN_MAX_SPEED;         // Current fan speed - default = 100(%)
    uint8_t startTemp = FAN_START_TEMP;       // Temp at which fan starts spinning
    uint8_t maxTemp = FAN_MAX_TEMP;           // Temp at which fan is at max speed (100% pulse width)
    uint8_t lowSpeed = FAN_LOW_SPEED;         // Low limit for fan speed (in %)
    bool override = false;                    // Force fan to 100%  
    bool relayOut = 1;                        // Set relay to force controller bypass
 
    void begin(){
      // Create PWM channel, Attach the LED PWM Channel to the GPIO Pin
      ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
      ledcAttachPin(PWM_PIN, PWM_CHANNEL);
      
      // Relay output
      gpio_set_direction((gpio_num_t)RELAY_OUT, GPIO_MODE_OUTPUT);

    }

    // Calc fan speed based on current temp and fan parameters.
    void setFanSpeed(int dhtTempA, int dhtTempB){
      // Assumes that 'A' is the upper sensor, above fridge coils
      //              'B' is lower sensor, below coils, presumed to be ambient temperature
      //
      // Fan speed will be proportional to difference between A and B, bounded by HIGH and LOW limits
      // 

      // Sanity checks
      if (override)
        fanSpeed = FAN_MAX_SPEED; //Full speed
      else {
        maxTemp = (maxTemp > FAN_TEMP_HIGH_LIMIT) ? FAN_TEMP_HIGH_LIMIT : maxTemp;
        startTemp = (startTemp < FAN_TEMP_LOW_LIMIT) ? FAN_TEMP_LOW_LIMIT : startTemp;
        if (startTemp >= maxTemp){
          // Bogus values - Reset to defaults
          startTemp = FAN_START_TEMP;
          maxTemp = FAN_MAX_TEMP;
        }
        
        if ((dhtTempA < startTemp) || (dhtTempA <= dhtTempB))
          fanSpeed = 0;
        else if (dhtTempA > maxTemp)
          fanSpeed = FAN_MAX_SPEED;
        else {
          int a = map(dhtTempA, startTemp, maxTemp, 0, 20);
          int b = (dhtTempA - dhtTempB) >= 5 ? 5 : (dhtTempA - dhtTempB) ;
          fanSpeed = (a * b) < FAN_LOW_SPEED ? 0 : (a * b);
        }
      }
      dutyCycle = map(fanSpeed, 0, FAN_MAX_SPEED, 0, PWM_MAX_DUTY_CYCLE);
      ledcWrite(PWM_CHANNEL, dutyCycle);

      // digitalWrite(RELAY_OUT, relayOut);
      gpio_set_level((gpio_num_t)RELAY_OUT, relayOut);

    }

    // map()function for short int's 
    uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
    const uint8_t dividend = out_max - out_min;
    const uint8_t divisor = in_max - in_min;
    const uint8_t delta = x - in_min;
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}
};

#endif //FAN_H