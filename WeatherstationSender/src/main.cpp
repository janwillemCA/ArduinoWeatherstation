
#include <VirtualWire.h>
#include "LowPower.h"
#include "DHT.h"

#define DHTPIN 8     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22

const unsigned long BAUD_RATE = 115200; // serial connection speed
boolean debug = true;

DHT dht(DHTPIN, DHTTYPE);
int ledPin = 13;
char Msg[30];// The string that we are going to send trought rf transmitter

void setup(){
  dht.begin();  // initialing the DHT sensor
  pinMode(ledPin,OUTPUT);
  // VirtualWire setup
  vw_setup(500); // Bits per sec
  vw_set_tx_pin(12);// Set the Tx pin. Default is 12
  Serial.begin(BAUD_RATE);

  if ( debug ) {
      Serial.println("Serial started");
    }
}

void loop(){

  // Read and store Sensor Data
  int humidity = dht.readHumidity();
  int temp = dht.readTemperature();
  int f = dht.readTemperature(true);
  int hi_f = dht.computeHeatIndex(f,humidity); //heat index in fahrenheit
  int heat_index  = (hi_f-32)*5/9; // convert heat index to celcius
  sprintf(Msg, "%d,%d,%d", humidity,temp,heat_index);

  if ( debug ) {
      Serial.println(Msg);
    }

  if ( debug ) {
      // Turn on a light to show transmitting
      digitalWrite(ledPin, HIGH);
      delay(100);
    } else {
      LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    }

  vw_send((uint8_t *)Msg, strlen(Msg));
  vw_wait_tx(); // Wait until the whole message is gone

  if (debug ) {
      // Turn off a light after transmission
      digitalWrite(ledPin, LOW);
    }

  if (debug ) {
      delay(4000);
    } else {
      // put 5 mins sleep mode
      // As lowpower library support maximam 8s ,we use for loop to take longer (5mins) sleep
      // 5x60=300
      // 300/4=75
      for (int i = 0; i<75; i++) {
          LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); // Instead of delay(4000);
        }
    }




}
