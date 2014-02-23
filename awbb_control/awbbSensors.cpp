/* 

 awbbSensor

 Contains all the methods to access sensor located on the 
 motor board 

 created 19 February 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain.
 
 */
#include "ArduinoRobot.h"

// Read temperature and hygro
void RobotControl::readTH(float &t,float &h){
  messageOut.writeByte(COMMAND_READ_TH);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
  uint8_t mess = messageIn.readByte();
    if(mess==COMMAND_READ_TH_RE){
      t=messageIn.readInt()/10.0;
      h=messageIn.readInt()/10.0;
    }
}

// Read Gps time and date
void RobotControl::readGPSTimeDate(GPS_DATA &gpsData){
  messageOut.writeByte(COMMAND_READ_GPS_TIMEDATE);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
    uint8_t mess = messageIn.readByte();
    if(mess==COMMAND_READ_GPS_TIMEDATE_RE){
      gpsData.hour = messageIn.readByte();
      gpsData.minute = messageIn.readByte();
      gpsData.seconds = messageIn.readByte();
      gpsData.milliseconds = messageIn.readInt();
      gpsData.day = messageIn.readByte();
      gpsData.month = messageIn.readByte();
      gpsData.year = messageIn.readByte();
    }
}

// Used to convert Bytes to float
union float2bytes { float f; char b[sizeof(float)]; };

// Read Gps lat long alt
void RobotControl::readGPSCoord(GPS_DATA &gpsData){
  messageOut.writeByte(COMMAND_READ_GPS_COORD);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
  uint8_t mess = messageIn.readByte();
    if(mess==COMMAND_READ_GPS_COORD_RE){
      gpsData.fix = messageIn.readByte();
      gpsData.sat = messageIn.readByte();
      float2bytes f2b;
      uint8_t i;
      for ( i=0; i < 4; i++ )
        f2b.b[i] = messageIn.readByte();
      gpsData.lat = f2b.f;
      for ( i=0; i < 4; i++ )
        f2b.b[i] = messageIn.readByte();
      gpsData.lon = f2b.f;
      for ( i=0; i < 4; i++ )
        f2b.b[i] = messageIn.readByte();
      gpsData.alt = f2b.f;
      }
}

// Read Sound level
void RobotControl::readSoundLevel(float &value){
  messageOut.writeByte(COMMAND_READ_SOUND);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
    uint8_t mess = messageIn.readByte();
    if(mess==COMMAND_READ_SOUND_RE){
      float2bytes f2b;
      uint8_t i;
      for ( i=0; i < 4; i++ )
        f2b.b[i] = messageIn.readByte();
      value = f2b.f;
      
      }
}

// Read CO2 sensor value
void RobotControl::readCO2Sensor(int &c){
  c = -9999;  //default returned value in case of problem of connection
  messageOut.writeByte(COMMAND_READ_CO2);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
  uint8_t mess = messageIn.readByte();
  if(mess==COMMAND_READ_CO2_RE){
    c=messageIn.readInt();
  }
}

