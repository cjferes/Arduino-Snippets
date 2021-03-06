// Arduino Sketch for the SensorMote from Kippkitts
// Original Sketch and Board by kippkitts LLC, 2013
// - Arduino Sketch available in: https://github.com/kippkitts/DataSensingLab/tree/master/DSL_Sensor_Mote
// - SensorMote Eagle files (schematic and board) available in https://github.com/kippkitts/DataSensingLab/tree/master/SensorMote_Code_Framework/Arduino
// Last version by Kippkitts: 15-April-2013 04:00 EST

// Currently being modified by cjferes
// Converted raw data into real data:
// - Sensor BMP085: pressure in atm and temperature in °C or °F
// - Sensor TEMT6000: ambient light intensity in lux
// - Sensor HIH-4030: relative Humidity (RH) in %
// - Sensor Electret Microphone: sound wave, currently in dB
// ON WORK:
// - Sensor Gas Sensor: Air quality 
// - Sensor LT5534: RF Detector
// - XBee: getting them to talk to each other

// LIBRARIES:
// You must download and install:
// 1) Adafruit BMP085 library. Available in https://github.com/adafruit/Adafruit-BMP085-Library
// 2) XBee:

#include <math.h> //math library for log10()
#include <Wire.h>
#include "Adafruit_BMP085.h"


// INITIAL SETTINGS
Adafruit_BMP085 bmp;

String tempUnits="C"; //C for Celcius, F for Farenheit

#define BMP085_ADDRESS 0x77  // I2C address of BMP085
#define CODE_VERSION 0.1
#define SILENT_VALUE 380  // Starting neutral microphone value (self-correcting)

// ********** START DIGITAL PINS *************
//
// 0         RX (from XBee)
// 1         TX (to XBee)
// 2
// 3
// 4 RF-Detect-Shutdown
const int RF_shdn = 4;
// 5  BMP085 XCLR
const int BMP085_XCLR = 5;
// 6         Button (Pressure Mat)
const int butPin = 6;
// 7 BMP085 EOC
const int BMP085_EOC = 7;
// 8         LED (Power)
int powr_led = 8;
// 9         LED (Loop Activity)
int loop_led = 9;
// 10        LED (Motion Detected - aka Button Pushed)
int motn_led = 10;
// 11       RF Configuration Detector
int rf_config = 11;
// 12       Gas Sensor Configuration Detector
int gas_config = 12;
// ********** END DIGITAL PINS *************


// ********** START ANALOG PINS *************
// A0        Humidity Sensor (HTH-4030)
const int hih4030 = A0;
// A1        Light Sensor (TEMT6000)
const int temt6000 = A1;
// A2        Microphone (Adafruit Board)
const int micPin = A2;
// A3        Gas Sensor
const int gasPin = A3;
// ********** END ANALOG PINS *************

// Node
const int nodeID = 1;

// Sensing variables
int light;
int humid;
int gasValue;
int button;
int temp;
int micVal = 0;
double temperature;
double pressure;
const int samplingWindow=50;

// SETUP ------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  // to XBee
  
  // while the serial stream is not open, do nothing:
//  while (!Serial) ;
  Serial.println("Initializing...");
 
  pinMode(RF_shdn, INPUT); // TESTING ONLY. SHOULD BE OUTPUT
  pinMode(BMP085_XCLR, OUTPUT); //
  pinMode(BMP085_EOC, INPUT); // End of conversion signal
  pinMode(butPin, INPUT);  // SET PULL-UP FOR CONFIG TESTING
  pinMode(rf_config, INPUT);  // SET PULL-UP FOR CONFIG TESTING
  pinMode(gas_config, INPUT);  // SET PULL-UP FOR CONFIG TESTING
 
  pinMode( powr_led, OUTPUT);
  pinMode( loop_led, OUTPUT);
  pinMode( motn_led, OUTPUT);

  digitalWrite(powr_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(powr_led, LOW);   // LED CHECK
  digitalWrite(loop_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(loop_led, LOW);   // LED CHECK
  digitalWrite(motn_led, HIGH);  // LED CHECK
  delay(1000);
  digitalWrite(motn_led, LOW);   // LED CHECK

  digitalWrite(powr_led, HIGH);  // LED OPERATIONAL
  digitalWrite(loop_led, HIGH);  // LED OPERATIONAL

  digitalWrite(BMP085_XCLR, HIGH);  // Make sure BMP085 is on
   
  Serial.print("SensorMote (Google I/O) v");
  Serial.println(CODE_VERSION);
  
  bmp.begin(); 
  
  digitalWrite(loop_led, LOW);
  
}

// LOOP ------------------------------------------------------------------------------------------------------
void loop() {
  
  // Button
  // read the state of the switch into a local variable:
  button = digitalRead(butPin);
  digitalWrite(motn_led, button);
  
  // Sensed variables
  light = analogRead(temt6000)*5.0/1024*100*2; //converted raw data into lux
  humid = 0.1628*(analogRead(hih4030)-26.667); //converted raw data into RH %
  temperature=bmp.readTemperature(); //get temperature in Celcius (given directly by sensor)
  if (tempUnits=="F") {
   temperature=temperature*9/5+32; //convert Celcius in Farenheit
  }
  pressure=bmp.readPressure()*9.86923267/1000000; //pressure in atm
  //micVal = getSound();  // get sound level with kippkitt's function
  micVal=getSoundPeakToPeak(); //get sound using my function
  
  //ON WORK
  gasValue =  analogRead(gasPin);
  
 
 
  // Output
  Serial.print("Hello PC! My Xbee is node ");
  Serial.print(nodeID);
  Serial.println(".");
  Serial.print("Button = ");
  Serial.print( button );
  Serial.print( ", Humidity = " );
  Serial.print( humid );
  Serial.print( "%, Light = " );
  Serial.print( light );
  Serial.print( " lx, Mic = " );
  Serial.print( micVal );
  Serial.print( "dB, Gas or RF = " );
  Serial.print( gasValue );

  Serial.print(", Temperature = ");
  Serial.print(temperature);
  Serial.print(" *");
  Serial.print(tempUnits);
  Serial.print(", ");
 
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" atm");


  digitalWrite(loop_led, HIGH);

  Serial1.print("Hello XBee ");
  Serial1.print(nodeID);
  Serial1.println("!!");
  Serial1.print("Button = ");
  Serial1.print( button );
  Serial1.print( ", Humidity = " );
  Serial1.print( humid );
  Serial1.print( "(raw), Light = " );
  Serial1.print( light );
  Serial1.print( ", Mic = " );
  Serial1.print( micVal );
  Serial1.print( ", Gas or RF = " );
  Serial1.print( gasValue );
  Serial1.print(", Temperature = ");
  Serial1.print(temperature);
  Serial1.print(" *");
  Serial1.print(tempUnits);
  Serial1.print(", ");
  Serial1.print("Pressure = ");
  Serial1.print(pressure);
  Serial1.println(" atm");

  digitalWrite(loop_led, LOW);
  delay(500);
  
}

// GET SOUND ------------------------------------------------------------------------------------------------------
//
// Does something sensible(ish) with the microphone input

// Kippkitt's function to get sound level
int getSound() {
  static int average = SILENT_VALUE; // stores the neutral position for the mic
  static int avgEnvelope = 0; // stores the average sound pressure level
  int avgSmoothing = 10; // larger values give more smoothing for the average
  int envSmoothing = 2; // larger values give more smoothing for the envelope
  int numSamples=1000; //how many samples to take
  int envelope=0; //stores the mean sound from many samples
  for (int i=0; i<numSamples; i++) {
    int sound=analogRead(micPin); // look at the voltage coming from the mic
    int sampleEnvelope = abs(sound - average); // the distance from this reading to the average
    envelope = (sampleEnvelope+envelope)/2;
    avgEnvelope = (envSmoothing * avgEnvelope + sampleEnvelope) / (envSmoothing + 1);
    //Serial.println(avgEnvelope);
    average = (avgSmoothing * average + sound) / (avgSmoothing + 1); //create a new average
  }
  return envelope;
}

int getSoundPeakToPeak(){
  // This function gets the sound level in dB.
  // First, it finds the raw peak to peak value of the sound wave within a defined time interval (sampling window)
  // using iterations to define the current maxima and minima of the sound wave.
  // Then it converts the raw peak to peak value into dB.
  // initialization
  unsigned long startTime=millis(); //first instant of the sampling window
  unsigned int signalMax=0; // inital max value
  unsigned int signalMin=1024; // initial min value
  
  while (millis()-startTime<samplingWindow){
    int sample=analogRead(micPin); //read the current sample of the sound wave
    if (sample<1024) {
      if (sample>signalMax){
        signalMax=sample; //update new maxima of the sound wave
      } else if (sample<signalMin){
        signalMin=sample; //update new minima of the sound wave
      }
    }
  }
  unsigned int peakToPeak=signalMax-signalMin; //peak to peak amplitude = maxValue-minValue
  double dB = pow(10,(log10(5.0/1024*peakToPeak)+12.77)/6.7511); // 5.0/1024 because AVCC=5V
  return dB;
}
