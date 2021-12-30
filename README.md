# ESP32-C3-Fanspeed

ESP32-C3 Fan speed controler

Reads two DHT11 temperature probes and sets computer fan speed using PWM motor controller, based on temperature differential

Publishes temperature and fan data to BlueTooth LE and HTML page.

Allows limited control of fan parameters via BLE

Inspiration and sample code from Everlanders:

      https://youtu.be/aS3BiYaEfiw
      https://youtu.be/KX67lBrizPg

## Libraries:
    Arduino OTA
    Adafruit DHT Sensor Library

DHT library must be modified to include mutex during DHT sensor read, or read errors will occur. In DHT.h, modify INterruptLock class:

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
    Edit WiFi, Pin and Fan parameters in setings.h


Uses the special espressif platform compiled by tasmota.

## Platform.ini

    [env:esp32c3-tasmota]
    platform  = https://github.com/tasmota/platform-espressif32/releases/download/v2.0.2/platform-tasmota-espressif32-2.0.2.zip
    framework = arduino
    board = esp32-c3-devkitm-1
    monitor_speed = 115200
    lib_deps = 
	    adafruit/DHT sensor library@^1.4.3
	    h2zero/NimBLE-Arduino@^1.3.1

