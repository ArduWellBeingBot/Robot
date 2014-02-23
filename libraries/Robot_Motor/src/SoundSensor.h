/* 

  soundSensor

  Calculate the sound voltage  

 Circuit:
 * Adafruit Electret Microphone Amplifier - MAX4466 with Adjustable Gain
    https://www.adafruit.com/products/1063
 
 created 17 February 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain
 */

#ifndef soundSensor_h
#define soundSensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define MIC_VOLTAGE  5.0   // Mic Voltage = 5 volts
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
 
float readSoundSensor(byte pin){ 
 
  unsigned long startMillis= millis(); // Start of sample window
  unsigned int peakToPeak = 0; // peak-to-peak level
   
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
   
  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow){
    sample = analogRead(pin);
    if (sample < 1024) // toss out spurious readings
    {
      if (sample > signalMax){
        signalMax = sample; // save just the max levels
      }
      else if (sample < signalMin){
        signalMin = sample; // save just the min levels
      }
    }
  }
    Serial.print("Raw sound:");
peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
  double volts = ( peakToPeak * 5.0) / 1024.0; // convert to volts
  Serial.print(peakToPeak);
  Serial.println(volts);
  return volts;
};

#endif
