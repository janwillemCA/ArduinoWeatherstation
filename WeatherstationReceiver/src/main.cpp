#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

// temp global variables voor data processing
int prevReceivedData = NULL;
int counter = 0;

// Global variables
WiFiClient client;
double humidity = 0;
double temp = 0;
long pressure = 0;

// WiFi information
const char WIFI_SSID[] = "muitg";
const char WIFI_PSK[] = "gehaktbal";

// Remote site information
const char http_site[] = "weatherstation.wunderground.com";
const int http_port = 80;

// Pin definitions
const int LED_PIN = 5;

void connectWiFi();
bool sendData(double tempf, int pressure, double humidity);

void setup() {
  Serial.begin(115200);

  // Set to receive data on pin 2
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2

  // Connect to WiFi
  connectWiFi();

}

void loop() {
  if (mySwitch.available()) {
      int receivedData = mySwitch.getReceivedValue();
      if (receivedData != prevReceivedData) {
          prevReceivedData = receivedData;
          if (receivedData == 999) {
              counter = 0;
              Serial.print("Humidity: "); Serial.println(humidity/100);
              Serial.print("Temperature: "); Serial.println(temp/100);
              Serial.print("Pressure: "); Serial.println(pressure);
              if ( !sendData(((temp/100) * 1.8 + 32), pressure, (humidity/100)) ) {
                  Serial.println("GET request failed");
                }
            } else {
              if (counter == 0) {
                  humidity = receivedData;
                }
              if (counter == 1) {
                  temp = receivedData;
                }
              if (counter == 2 ) {
                  pressure = receivedData;
                }
              counter++;
            }
        }
      mySwitch.resetAvailable();
    }
}

// Attempt to connect to WiFi
void connectWiFi() {

  byte led_status = 0;

  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);

  // Initiate connection with SSID and PSK
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  // Blink LED while we wait for WiFi connection
  while ( WiFi.status() != WL_CONNECTED ) {
      digitalWrite(LED_PIN, led_status);
      led_status ^= 0x01;
      delay(100);
    }

  // Turn LED on when we are connected
  digitalWrite(LED_PIN, HIGH);
}

// Perform an HTTP GET request to a remote page
bool sendData(double tempf, int pressure, double humidity) {

  // Attempt to make a connection to the remote server
  if ( !client.connect(http_site, http_port) ) {
      return false;
    }

  // Make an HTTP GET request
  char data[600];
  char str_temp_tempf[6];
  char str_temp_humidity[6];
  dtostrf(tempf, 4, 2, str_temp_tempf);
  dtostrf(humidity, 4, 2, str_temp_humidity);
  sprintf(data, "GET /weatherstation/updateweatherstation.php?ID=IGARDERE6&PASSWORD=og0fq7lz&dateutc=now&tempf=%s&baromin=%d&humidity=%s&softwaretype=DIY_Weather_Station&action=updateraw HTTP/1.1",str_temp_tempf, pressure, str_temp_humidity);
  Serial.println(data);
  client.println(data);
  client.print("Host: ");
  client.println(http_site);
  client.println("Connection: close");
  client.println();

  return true;
}
