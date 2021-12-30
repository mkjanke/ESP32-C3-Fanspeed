#ifndef DHT11_H
#define DHT11_H

// DHT Sensor includes and defines
#include <Adafruit_Sensor.h>
#include <DHT.h>

// DHT11 temp sensor config
#define DHTPIN_A  18                // Digital pin connected to the DHT sensor
#define DHTPIN_B  19                // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11            // Sensor type DHT 11
#define DHT_TEMP_F true          // Set to True for Fahrenheit, false for Celsius

// DHT Temperature Probe setup
DHT dhtA(DHTPIN_A, DHTTYPE);
DHT dhtB(DHTPIN_B, DHTTYPE);

// Global vars for current temperature & humidity, updated by timers
float dhtTempA = 0.0;
float dhtTempB = 0.0;
int dhtReadCount = 0;
int dhtReadErrorCount = 0;

#endif //DHT11_H