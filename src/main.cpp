
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "bleInterface.h"
#include "esp_system.h"
#include "fan.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor.h"
#include "settings.h"
#include "log.h"


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

char uptimeBuffer[20];  // scratch space for storing formatted 'uptime' string
PWMFan fan;
bleInterface bleIF;
bool bleInterface::deviceConnected = false;

sensorPair sensors(SENSORPIN_A, SENSORPIN_B, SENSORTYPE);

void dumpESPStatus();     // Dump ESP status to terminal window
void dumpSensorStatus();  // Dump Sensor status to terminal window
void uptime();
void checkWiFi(void*);
void readSensors(void*);
void outputWebPage(WiFiClient);

WiFiServer server(80);

TaskHandle_t xcheckWiFiHandle = NULL;
TaskHandle_t xreadSensorsHandle = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("This device is Woke!");

  // Setup status LED and Relay
  pinMode(LED_OUT, OUTPUT);
  digitalWrite(LED_OUT, LOW);

  // WiFi
  WiFi.begin(ssid, password);
  WiFi.setHostname(DEVICE_NAME);

  sLog.init();

  // Initialize BlueTooth
  bleIF.begin();

  // Start web server
  server.begin();
  sLog.send((String)"Web Server started at http://" + WiFi.localIP().toString(), true);

  // Initialize fan and temperature sensors
  fan.begin();
  sensors.begin();

  digitalWrite(RELAY_OUT, HIGH);

  uptime();

  bleIF.updateUptime(uptimeBuffer);
  bleIF.updateFan();

  dumpESPStatus();

  // Background tasks, WiFi and Temperature Read
  xTaskCreate(checkWiFi, "Check WiFi", 3000, NULL, 1, &xcheckWiFiHandle);
  xTaskCreate(readSensors, "Read Sensors", 2500, NULL, 6, &xreadSensorsHandle);

  // Initialize OTA Update libraries
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.onStart([]() { bleIF.stopAdvertising(); });
  ArduinoOTA.begin();
  sLog.send((String)DEVICE_NAME + " is Woke", true);
}

void loop() {
  static int loopcnt = 0;
  ArduinoOTA.handle();

  WiFiClient client = server.available();  // Listen for incoming clients
  if (client) {
    digitalWrite(LED_OUT, HIGH);
    Serial.print("Connection from: ");
    Serial.println(client.remoteIP());
    outputWebPage(client);  // If a new client connects,
    client.flush();
    client.stop();
    digitalWrite(LED_OUT, LOW);
  }

  digitalWrite(RELAY_OUT, fan.relayOut);

  delay(HEARTBEAT / 10);  // Non-blocking on ESP32 <?>

  if (loopcnt++ > 60) {
    dumpSensorStatus();
    dumpESPStatus();
    loopcnt = 0;
  }
}

void uptime() {
  // Constants for uptime calculations
  static const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  static const uint32_t millis_in_hour = 1000 * 60 * 60;
  static const uint32_t millis_in_minute = 1000 * 60;

  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes =
      (millis() - (days * millis_in_day) - (hours * millis_in_hour)) /
      millis_in_minute;
  snprintf(uptimeBuffer, sizeof(uptimeBuffer), "Uptime: %2dd%2dh%2dm", days,
           hours, minutes);
}

void checkWiFi(void* parameter) {
  String status;
  status.reserve(40);
  for (;;) {  // infinite loop
    vTaskDelay(HEARTBEAT * 4 / portTICK_PERIOD_MS);
    if (WiFi.status() == WL_CONNECTED) {
      status=(String)"IP:" + WiFi.localIP().toString() + " DNS:" + WiFi.dnsIP().toString();
      bleIF.updateStatus(status.c_str());
    } else {
      vTaskDelay(HEARTBEAT * 4 / portTICK_PERIOD_MS);
      bleIF.updateStatus("WiFi not connected");
      WiFi.reconnect();  // Try to reconnect to the server
    }
  }
}

// Read both temp sensors
// Update BlueTooth characteristics
// Set fan speed
void readSensors(void* parameter) {
  String status = "";
  status.reserve(28);

  for (;;) {  // infinite loop

    vTaskDelay(HEARTBEAT * 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_OUT, HIGH);

    if (sensors.readFahrenheit()) {
      status = "Read A: " + String(sensors.temperatureA, 1) + ", B: " + String(sensors.temperatureB, 1);
      bleIF.updateStatus(status.c_str());
      sLog.send(status.c_str());
      fan.setFanSpeed((int)sensors.temperatureA, (int)sensors.temperatureB);
      bleIF.updateTemperature(sensors.temperatureA, sensors.temperatureB);
      bleIF.updateFan();
    }
    else {
      bleIF.updateStatus("read failed");
    }

    digitalWrite(LED_OUT, LOW);

    vTaskDelay(HEARTBEAT / portTICK_PERIOD_MS);
    digitalWrite(LED_OUT, HIGH);

    // Update BlueTooth characteristics
    uptime();
    bleIF.updateUptime(uptimeBuffer);
    digitalWrite(LED_OUT, LOW);
  }
}

// Output sensor status to terminal
void dumpSensorStatus() {
  Serial.printf("TempA: %.1f\n", sensors.temperatureA);
  Serial.printf("TempB: %.1f\n", sensors.temperatureB);
  Serial.printf("Fan Speed: %d\n", fan.fanSpeed);
  Serial.printf("Read Count: %d\n", sensors.readCount);
  Serial.printf("Error Count: %d\n", sensors.errorCount);
  Serial.println();
}

// Output device status to terminal
void dumpESPStatus() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  Serial.println("Hardware info:");
  Serial.printf("%d cores Wifi %s%s\n", chip_info.cores,
                (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
  Serial.println();
  Serial.printf("Free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
  Serial.println();
}

void outputWebPage(WiFiClient client) {
  if (client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");  //  Important.
    client.println("<!DOCTYPE HTML><html><head><meta charset=utf-8></head>");
    client.println("<body><font face='Arial'>");
    client.print("<h1>");
    client.print(DEVICE_NAME);
    client.print("</h1>");
    client.println("<br><br>");
    client.println(uptimeBuffer);
    client.print("<br>Temp A: ");
    client.print(sensors.temperatureA);
    client.print("<br>Temp B: ");
    client.println(sensors.temperatureB);
    client.println("<br>");
    client.print("<br>Error Count: ");
    client.println(sensors.errorCount);
    client.print("<br>Read Count: ");
    client.println(sensors.readCount);
    client.println("<br>");
    client.print("<br>Duty Cycle: ");
    client.println(fan.dutyCycle);
    client.print("<br>Fan Speed: ");
    client.println(fan.fanSpeed);
    client.print("<br>Start Temp: ");
    client.print(fan.startTemp);
    client.print("<br>Max Temp: ");
    client.println(fan.maxTemp);
    client.print("<br>Low Speed: ");
    client.println(fan.lowSpeed);
    client.print("<br>Override: ");
    client.println(fan.override);
    client.print("<br>Relay: ");
    client.println(fan.relayOut);

    client.print("<br>HW Wifi Task: ");
    client.println(uxTaskGetStackHighWaterMark(xcheckWiFiHandle));
    client.print("<br>HW Sensor: ");
    client.println(uxTaskGetStackHighWaterMark(xreadSensorsHandle));
    client.print("<br>Heap: ");
    client.println(esp_get_minimum_free_heap_size());

    client.println("");
    client.println("<br><br>");
    client.println("</font></center></body></html>");
  }
}