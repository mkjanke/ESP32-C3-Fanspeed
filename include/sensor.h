#ifndef SENSOR_H
#define SENSOR_H

#include "settings.h"

#ifdef DHT11SENSOR
#include "dht11.h"
#endif

#ifdef DS18B20SENSOR
#include "ds18b20.h"
#endif

#endif //SENSOR_H