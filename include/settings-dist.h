#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

#define WIFI_SSID "---"
#define WIFI_PASSWORD "---"

#define HEARTBEAT 10000L         // Sensor and WiFi loop delay (ms)
#define DEVICE_NAME "ESP-Fanspeed"


#define SYSLOG_HOST "192.168.1.7"
#define SYSLOG_PORT 514

// Default PWM parameters
// Linear fan speed. I.E 75F to 90F will result in fan speed from 40% to 100% of max RPM
// Overriden by BLE writes
#define FAN_START_TEMP         65   // Temp at which to turn on fan at low speed
#define FAN_MAX_TEMP           90   // Temp above which fan will run at 100%
#define FAN_TEMP_HIGH_LIMIT   120   // Max acceptable value for fan.maxTemp
#define FAN_TEMP_LOW_LIMIT     40   // Min acceptable value for fan.startTemp
#define FAN_MAX_SPEED         100   // Fan max speed (percent, 0-100)
#define FAN_LOW_SPEED          10   // PWM pulse width at minimum fan speed
                                    //   - must be less than PWM_MAX_DUTY_CYCLE)

// Fixed PWM parameters
#define PWM_PIN                 3   // GPIO pin 3
#define PWM_CHANNEL             0   // 
#define PWM_RESOLUTION          8   // 8-bit resolution, 255 possible values
#define PWM_MAX_DUTY_CYCLE    255   // Maximum value when PWM is fully 'ON'. Sets PWM range 8-bits = 255 possible values.
#define PWM_FREQUENCY       20000   // How often PWM cycles on & off 
                                    //   - Set high enough that you don't hear fan pulses

// Fan control pins
#define RELAY_OUT              10
#define LED_OUT                 7

// Sensor type, pins and config
// Uncomment either DHT section or DS18B20 Section, not both

// ---- DHT section
// #define SENSORPIN_A  18
// #define SENSORPIN_B  19

// #define SENSORTYPE DHT11            // Sensor type DHT 11
// #define DHT11SENSOR
// ---- End DHT Section

// ---- DS18B20 Setion
static const uint8_t DS18B20{18};      /** DS18B20 type sensor */
#define SENSORTYPE DS18B20
#define DS18B20SENSOR
#define SENSORPIN_A  5
#define SENSORPIN_B  SENSORPIN_A      // Dummy def for compatibility with DHT version
// ---- End DS Section

#define TEMP_FAHRENHEIT true          // Set to True for Fahrenheit, false for Celsius

// BlueTooth UUID's

#define SERVICE_UUID         "611f9238-3915-11ec-8d3d-0242ac130003"  // 9238
#define TEMPERATURE_UUID     "611f945e-3915-11ec-8d3d-0242ac130003"  // 945e FLoat Little Endian
#define HUMIDITY_UUID        "611f9620-3915-11ec-8d3d-0242ac130003"  // 9620 FLoat Little Endian
#define UPTIME_UUID          "611f96f2-3915-11ec-8d3d-0242ac130003"  // 96f2 String
#define FANSPEED_UUID        "611f9800-3915-11ec-8d3d-0242ac130003"  // 9800 Float Little Endian
#define FANSTART_UUID        "611f997c-3915-11ec-8d3d-0242ac130003"  // Fan Start Temp
#define FANMAX_UUID          "611f9a44-3915-11ec-8d3d-0242ac130003"  // Fan Max Temp
#define FANLOW_UUID          "611f9b02-3915-11ec-8d3d-0242ac130003"  // Fan speed - minimum PWM pulse width
#define FANOVERRIDE_UUID     "611f9bb6-3915-11ec-8d3d-0242ac130003"  // Force fan speed to 100%
#define RELAY_UUID           "611f9d00-3915-11ec-8d3d-0242ac130003"  // Fan Controller bypass relay
#define STATUS_UUID          "6bcbf0ea-7fbb-11ec-a8a3-0242ac120002"  // String - Device Status messages

#endif //SETTINGS_H