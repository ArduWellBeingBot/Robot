/* 

  ligthSensor

  Calculate the ligth 

 Circuit:

 created 1 may 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain
 */

#ifndef ligthSensor_h
#define ligthSensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

int BH1750_Read(int address) //
{
  int i=0;
  int val;
  byte buff[2];
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
	}
  Wire.endTransmission();  
  val=((buff[0]<<8)|buff[1])/1.2;
  return val;
}

void BH1750_Init(int address) 
{
  Wire.begin();
  Wire.beginTransmission(address);
  Wire.write(0x10);//1lx resolution 120ms
  Wire.endTransmission();
}

#define BH1750address 0x23 //setting i2c address

int readLigthSensor(){
  static bool Init = false; 
  if (!Init) {
	BH1750_Init(BH1750address);
	Init = true;
  }
  int val =  BH1750_Read(BH1750address);
  Serial.print("Lux:");
  Serial.println(val);
  return val;
};
#endif
