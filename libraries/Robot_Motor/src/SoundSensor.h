/* 

  soundSensor

  Calculate the sound voltage  

 Circuit:
 * Adafruit Electret Microphone Amplifier - MAX4466 with Adjustable Gain
    https://www.adafruit.com/products/1063
 * The micro output is at the max

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

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

#define FHT1
#ifdef FHT
#define LOG_OUT 1 // use the log output function
#define FHT_N 32 // set to 256 point fht
#include "FHT/FHT.h"
 
float readSoundSensor(byte pin){
  float val = 0;
  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
    fht_input[i] = analogRead(pin); // put real data into bins
  }
 
  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_log(); // take the output of the fht
  int maxv=0;
  Serial.print("vec;");
  for (int i = 5 ; i < FHT_N/2 ; i++) { // save 256 samples
	if (fht_log_out[i]  > maxv ) {
          maxv = fht_log_out[i] ;
        }
        Serial.print(fht_log_out[i]);
        Serial.print(";");
  }
  val = maxv;
  Serial.println(val);
  return val;
};
#else 

int readSoundSensorraw(byte pin){ 
 
  unsigned long startMillis= millis(); // Start of sample window
  unsigned int peakToPeak = 0; // peak-to-peak level
   
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  signalMax = 0;
  signalMin = 1024;
  Serial.print("Raw sound:");
  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow){
    sample = analogRead(pin);
	// The value of sound is a value arround 3.3/2
	if (sample < 1024 && sample > 20 ) // toss out spurious readings
	  {
		if (sample > signalMax){
		  signalMax = sample; // save just the max levels
		}
		else if (sample < signalMin){
		  signalMin = sample; // save just the min levels
		}
	  }
  }
  peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
  Serial.println(signalMin);
  Serial.println(peakToPeak);
  Serial.println(signalMax);
  return peakToPeak;
};

void tri_bulles(int *tab, int taillep)
{
  bool tab_en_ordre = false;
  int taille = taillep;
  int rel;
  while(!tab_en_ordre)
	{
	  tab_en_ordre = true;
	  for(int i=0 ; i < taille-1 ; i++)
		{
		  if(tab[i] > tab[i+1])
			{
			  rel = tab[i+1];
			  tab[i+1] = tab[i];
			  tab[i] =rel;
			  tab_en_ordre = false;
			}
		}
	  taille--;
	}
}

float readSoundSensor(byte pin){ 
  int tabval[10];
  byte i;
  float volts;
  for (i = 0 ; i < 10 ; i++ ) {
	tabval[i] = readSoundSensorraw(pin);
  }
  tri_bulles(tabval, 10);
  Serial.println(tabval[5]);
  volts = ( tabval[5] * 5.0) / 1024.0; // convert to volts
  
  Serial.println(volts);
  return volts;
};
#endif
#endif
