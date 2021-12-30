
#include "settings.h"
#include "fan.h"
#include "bleInterface.h"
#include "dht11.h"

#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <stdio.h>

// #include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

const char* ssid = WIFI_SSID;
const char* password =  WIFI_PASSWORD;

char uptimeBuffer[20];                  //scratch space for storing formatted 'uptime' string
PWMFan fan;
bleInterface bleIF;

void dumpESPStatus();                   // Dump ESP status to terminal window
void dumpSensorStatus();                // Dump Sensor status to terminal window
void uptime();
void checkWiFi(void * );
void readDHT(void * );
void outputWebPage( WiFiClient );

WiFiServer server(80);

void setup(){
  Serial.begin(115200);
  Serial.println("Woke");

  // Setup status LED and Relay
  pinMode(LED_OUT,OUTPUT);
  digitalWrite(LED_OUT,LOW);

  // WiFi
  WiFi.begin(ssid, password);
  WiFi.setHostname(DEVICE_NAME);

  // Start web server
  server.begin();
  Serial.println("Web Server started.");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  // Initialize fan and temperature sensors  
  fan.begin();
  dhtA.begin();
  dhtB.begin();

  digitalWrite(RELAY_OUT,HIGH);

  uptime();

  //Initialize BlueTooth
  bleIF.begin();
  bleIF.update();

  dumpESPStatus();

    // Background tasks, WiFi and Temperature Read
  xTaskCreate(
      checkWiFi,        // Function that should be called
      "Check WiFi",     // Name of the task (for debugging)
      1000,             // Stack size (bytes)
      NULL,             // Parameter to pass
      1,                // Task priority
      NULL              // Task handle
  );

    xTaskCreate(
      readDHT,          // Function that should be called
      "Read DHT",       // Name of the task (for debugging)
      2000,             // Stack size (bytes)
      NULL,             // Parameter to pass
      6,                // Task priority
      NULL             // Task handle
  ); 

  // Initialize OTA Update libraries
  ArduinoOTA.setHostname(DEVICE_NAME);
    ArduinoOTA.onStart([]() {
        bleIF.stopAdvertising();
    });
  ArduinoOTA.begin();
}

void loop(){
  static int loopcnt = 0;
  ArduinoOTA.handle();
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {
      digitalWrite(LED_OUT,HIGH);
      Serial.print("Connection from: ");
      Serial.println(client.remoteIP());
      outputWebPage(client);                     // If a new client connects,
      client.flush();
      client.stop();
      digitalWrite(LED_OUT,LOW);
  }
  
  digitalWrite(RELAY_OUT,fan.relayOut);

  delay(HEARTBEAT/10);                      // Non-blocking on ESP32 <?>
  
  if ( loopcnt++ > 60 ){
      dumpSensorStatus();
      dumpESPStatus();
      loopcnt = 0;
  }
    
}


void uptime(){
  // Constants for uptime calculations
  static const uint32_t millis_in_day = 1000 * 60 * 60 * 24;
  static const uint32_t millis_in_hour = 1000 * 60 * 60;
  static const uint32_t millis_in_minute = 1000 * 60;

  uint8_t days = millis() / (millis_in_day);
  uint8_t hours = (millis() - (days * millis_in_day)) / millis_in_hour;
  uint8_t minutes = (millis() - (days * millis_in_day) - (hours * millis_in_hour)) / millis_in_minute;
  snprintf(uptimeBuffer, sizeof(uptimeBuffer), "Uptime: %2dd%2dh%2dm", days, hours, minutes);
}

void checkWiFi(void * parameter){
  for(;;){ // infinite loop
    vTaskDelay(HEARTBEAT * 4 / portTICK_PERIOD_MS);
     if (WiFi.status() == WL_CONNECTED){
      Serial.println(WiFi.localIP());
    } else {
      vTaskDelay(HEARTBEAT * 4 / portTICK_PERIOD_MS);
      WiFi.reconnect();  // Try to reconnect to the server
    }
  }
}

// Read both DHT11's
// Update BlueTooth characteristics
// Set fan speed
void readDHT(void * parameter){
  for(;;){ // infinite loop

    vTaskDelay(HEARTBEAT * 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_OUT, HIGH);

    float tempA = dhtA.readTemperature(DHT_TEMP_F);

    if (!isnan(tempA)){
      dhtTempA = tempA;
    }
    digitalWrite(LED_OUT, LOW);

    vTaskDelay(HEARTBEAT * 2 / portTICK_PERIOD_MS);
    digitalWrite(LED_OUT, HIGH);

    float tempB = dhtB.readTemperature(DHT_TEMP_F);  
    if (!isnan(tempB)) {
      dhtTempB = tempB;
    }
    
    if ( isnan(tempA) || isnan(tempB)) {
      dhtReadErrorCount++;
    } else {
      dhtReadCount++;
      fan.setFanSpeed((int)dhtTempA, (int)dhtTempB);
    }

    // Update BlueTooth characteristics
    uptime();
    bleIF.update();
    digitalWrite(LED_OUT, LOW);
  }
}

// Output sensor status to terminal
void dumpSensorStatus()
{
  Serial.println(uptimeBuffer);
  Serial.print("TempA: ");
  Serial.print(dhtTempA);
  Serial.print("\tTempB: ");
  Serial.println(dhtTempB);
  Serial.print("Fan Speed: ");
  Serial.println(fan.fanSpeed);
  Serial.print("Read Count: ");
  Serial.println(dhtReadCount);
  Serial.print("Error Count: ");
  Serial.println(dhtReadErrorCount);
  Serial.println();
}

// Output device status to terminal
void dumpESPStatus()
{
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  
  Serial.println("Hardware info:");
  Serial.printf("%d cores Wifi %s%s\n", chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
  (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
  Serial.println();  
  Serial.printf("Free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
  Serial.println();
}

void outputWebPage(WiFiClient client){
   if (client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  Important.
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><meta charset=utf-8></head>");
    client.println("<body><font face='Arial'>");
    client.print("<h1>");
    client.print(DEVICE_NAME);
    client.print("</h1>");
    client.println("<br><br>");
    client.println(uptimeBuffer);
    client.print("<br>Temp A: ");
    client.print(dhtTempA);
    client.print("<br>Temp B:: ");
    client.println(dhtTempB);
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
    client.print("<br>Error Count: ");
    client.println(dhtReadErrorCount);
    client.print("<br>Read Count: ");
    client.println(dhtReadCount);
    client.println("");
    client.println("<br><br>");
    client.println("</font></center></body></html>");
   }
}