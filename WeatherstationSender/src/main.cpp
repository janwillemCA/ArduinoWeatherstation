#include <RCSwitch.h>
#include <stdint.h>
#include "LowPower.h"
#include "DHT.h"

#include "SparkFunBME280.h"
//Library allows either I2C or SPI, so include both.
#include "Wire.h"
#include "SPI.h"

BME280 bme;

#define DHTPIN 8     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22

const unsigned long BAUD_RATE = 115200; // serial connection speed
boolean debug = false;

DHT dht(DHTPIN, DHTTYPE);
int ledPin = 13;
char Msg[30];// The string that we are going to send trought rf transmitter

RCSwitch mySwitch = RCSwitch();

void setup(){

  // set led pin as output
  pinMode(ledPin,OUTPUT);

  // start serial
  Serial.begin(BAUD_RATE);

  if ( debug ) {
      Serial.println("Serial started");
    }

  // Transmitter is connected to Arduino Pin #12
  mySwitch.enableTransmit(12);

  // initialise the DHT sensor
  dht.begin();

  // initialise and set up the BME280

  //***Driver settings********************************//
  //commInterface can be I2C_MODE or SPI_MODE
  //specify chipSelectPin using arduino pin names
  //specify I2C address.  Can be 0x77(default) or 0x76

  //For I2C, enable the following and disable the SPI section
  bme.settings.commInterface = I2C_MODE;
  bme.settings.I2CAddress = 0x76;

  //For SPI enable the following and dissable the I2C section
  //bme.settings.commInterface = SPI_MODE;
  //bme.settings.chipSelectPin = 10;


  //***Operation settings*****************************//

  //renMode can be:
  //  0, Sleep mode
  //  1 or 2, Forced mode
  //  3, Normal mode
  bme.settings.runMode = 3;   //Normal mode

  //tStandby can be:
  //  0, 0.5ms
  //  1, 62.5ms
  //  2, 125ms
  //  3, 250ms
  //  4, 500ms
  //  5, 1000ms
  //  6, 10ms
  //  7, 20ms
  bme.settings.tStandby = 0;

  //filter can be off or number of FIR coefficients to use:
  //  0, filter off
  //  1, coefficients = 2
  //  2, coefficients = 4
  //  3, coefficients = 8
  //  4, coefficients = 16
  bme.settings.filter = 0;

  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.tempOverSample = 1;

  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.pressOverSample = 1;

  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.humidOverSample = 1;


  //Calling .begin() causes the settings to be loaded
  delay(10);        //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  bme.begin();

}

void loop(){

  // Read and store Sensor Data
  double humidity = dht.readHumidity();
  double temp = bme.readTempC(); //dht.readTemperature();
  double f = dht.readTemperature(true);
  double hi_f = dht.computeHeatIndex(f,humidity); //heat index in fahrenheit
  double heat_index  = (hi_f-32)*5/9; // convert heat index to celcius
  long pressure = bme.readFloatPressure();

  if ( debug ) {
      Serial.println(humidity);
      Serial.println(temp);
      Serial.println(pressure);
    }

  if ( debug ) {
      // Turn on a light to show transmitting
      digitalWrite(ledPin, HIGH);
      delay(100);
    } else {
      LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    }

  mySwitch.send(humidity*100, 24);
  mySwitch.send(temp*100, 24);
  mySwitch.send(pressure, 24);
  mySwitch.send(999, 24);

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
      for (int i = 0; i<30; i++) {
          LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); // Instead of delay(4000);
        }
    }
}
