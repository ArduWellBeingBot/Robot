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
const int sampleWindowPic = 600; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
MedianFilter Filter;
#define FHT1
#ifdef FHT
 
float readSoundSensor1(byte pin){
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
int tab[500];
int tab1[500];
float readSoundSensor1(byte pin){ 
 
  unsigned long startMillis= millis(); // Start of sample window
  unsigned int peakToPeak = 0; // peak-to-peak level
   
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  bool ko = 0;
  double volts = -1;
  int filtered;
  signalMax = 0;
  signalMin = 1024;
//  // collect data for 50 mS
//  while ((millis() - startMillis < sampleWindowPic) && ((signalMax - signalMin)<610)  && (signalMin < 230)){
//    sample = analogRead(pin);
//      if (sample > signalMax){
//        signalMax = sample; // save just the max levels
//      }
//      else if (sample < signalMin){
//        signalMin = sample; // save just the min levels
//      }
//  }
//  delay(30);
    signalMin = 0;
    signalMax = 1024;
    int cpt=0;
  //while ( signalMin < 230 || signalMax > 800) {
  startMillis= millis(); // Start of sample window
  signalMax = 0;
  signalMin = 1024;
cpt=0;
  int nextread = -1;
  int lastval[10] ;
  lastval[cpt] = tab[cpt] = tab1[cpt]=  analogRead(pin);
  cpt++;
  lastval[cpt]= tab[cpt] = tab1[cpt]= analogRead(pin);
  cpt++;
  lastval[cpt]= tab[cpt] = tab1[cpt]= analogRead(pin);
  cpt++;
  lastval[cpt]= tab[cpt] = tab1[cpt]= analogRead(pin);
  cpt++;
  lastval[cpt]= tab[cpt] = tab1[cpt]= analogRead(pin);
  cpt++;
  while (millis() - startMillis < sampleWindow){
    sample = analogRead(pin);
    tab[cpt] = sample;
    if ( nextread > 0 && cpt < nextread  ) {
      tab1[cpt] = -1;
    } else {
      tab1[cpt] = sample;
      int found=0;
      int i;
      for( i = 0 ; i < 5 ; i++ ) {
	if ((sample - lastval[(cpt-i+1)%5]) < -100) {
	  found ++;
	}
      }
      if ( nextread == -1 && found >= 4 ) {
        nextread = cpt + 80;
      } else {
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
    }
    lastval[cpt%5] = sample;
    cpt++;
  }
  //}
  Serial.print("Raw sound;");
  Serial.print(cpt);
  Serial.print(";");
  peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
  volts = ( peakToPeak * 5.0) / 1024.0; // convert to volts
  Serial.print(signalMin);
  Serial.print(";");
  Serial.print(peakToPeak);
  Serial.print(";");
  Serial.print(signalMax);
  Serial.print(";");
  Serial.println(volts);
  for(int i = 0 ;i < cpt;i++ ) {
    Serial.print(tab[i]);
    Serial.print(";");
  }
  Serial.println("");
  for(int i = 0 ;i < cpt;i++ ) {
    Serial.print(tab1[i]);
    Serial.print(";");
  }
  Serial.println("");
   
  return volts;
};
#endif
#endif
