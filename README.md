# ESP32-C3-Fanspeed

ESP32-C3 Fan speed controller

Reads two temperature probes (DHT11 or DS18B20) and sets computer fan speed using PWM motor controller, based on temperature differential

Publishes temperature and fan data to Bluetooth LE and HTML page.

Allows limited control of fan parameters via BLE.

Inspiration and sample code from Everlanders:

      https://youtu.be/aS3BiYaEfiw
      https://youtu.be/KX67lBrizPg

### A stripped-down verion using ESP-NOW instead of BLE is here: https://github.com/mkjanke/ESP32-Fanspeed-NOW

## Libraries:
    Arduino OTA
    Adafruit DHT Sensor Library (DHT11 version)
    OneWireNG (DS18B20 version)

Adafruit DHT library must be modified to include mutex during DHT sensor read, or read errors will occur. In DHT.h, modify InterruptLock class:

    class InterruptLock {
    #if defined(ESP32)
      portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;
    #endif

    public:
      InterruptLock() {
    #if !defined(ARDUINO_ARCH_NRF52)
        noInterrupts();
    #endif
    #if defined(ESP32)
        portENTER_CRITICAL(&mutex);
    #endif
      }
      ~InterruptLock() {
    #if !defined(ARDUINO_ARCH_NRF52)
        interrupts();
    #endif
    #if defined(ESP32)
        portEXIT_CRITICAL(&mutex);
    #endif

      }
    };

Note: this is a crude - better ways exist.

## To get started:
    Rename setup-dist.h to setup.h 
    Edit WiFi, Sensor, Pin and Fan parameters in settings.h


Uses the special espressif platform compiled by tasmota.

## Platform.ini (DHT Version)

    [env:esp32c3-tasmota]
    platform  = https://github.com/tasmota/platform-espressif32/releases/download/v2.0.2/platform-tasmota-espressif32-2.0.2.zip
    framework = arduino
    board = esp32-c3-devkitm-1
    monitor_speed = 115200
    lib_deps = 
	    adafruit/DHT sensor library@^1.4.3
	    h2zero/NimBLE-Arduino@^1.3.1

## Platform.ini (DS18B20 Version)

    [env:esp32c3-tasmota]
    platform  = https://github.com/tasmota/platform-espressif32/releases/download/v2.0.2/platform-tasmota-espressif32-2.0.2.zip
    framework = arduino
    board = esp32-c3-devkitm-1
    monitor_speed = 115200
    lib_deps = 
	    pstolarz/OneWireNg@^0.10.1
	    h2zero/NimBLE-Arduino@^1.3.1

