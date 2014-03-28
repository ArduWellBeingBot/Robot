/* 

 awbbSensor

 Contains all the methods to access sensor located on the 
 motor board 

 created 19 February 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain.
 
 */
#include "ArduinoRobot.h"

// Read All sensors data fill the record
void RobotControl::readSensorsData(int ligth, awbbSensorData &awbbSensorDataBuf){
  messageOut.writeByte(COMMAND_READ_SENSORS);
  messageOut.writeInt(ligth);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
  uint8_t mess = messageIn.readByte();
  if(mess==COMMAND_READ_SENSORS_RE){
	messageIn.readBuffer(sizeof(awbbSensorDataBuf),(uint8_t *)&awbbSensorDataBuf);
  }
}

// Read cmd
byte RobotControl::readCmd(char *cmd){
  char c;
  messageOut.writeByte(COMMAND_READ_CMD);
  messageOut.sendData();
  delay(10);
  while(!messageIn.receiveData());
  uint8_t len=0;
  uint8_t mess = messageIn.readByte();
  if(mess==COMMAND_READ_CMD_RE){
    len = messageIn.readByte();
	if ( len > 0 ) {
    uint8_t i;
    for ( i=0; i < len; i++ )
      cmd[i] = messageIn.readByte();
    cmd[len] = 0;
	Serial.println("cmd");
    Serial.println(cmd);
    } 
  }
  return len; 
}

// send cmd data
void RobotControl::sendCmdData() {
  initDataSDReading(true);
  bool cont = true;
  char c;
  // Send the file from sd : return -1 when EOF
  while ( cont ) {
	messageOut.writeByte(COMMAND_SEND_CMD_DATA);
	c = 1;
	while ( c > 0 && c != '\n' ) { 
	  c = SDReadByteSinceFromPos();
	  messageOut.writeByte(c);
	}
	messageOut.sendData();
	while(!messageIn.receiveData());
	uint8_t len;
	uint8_t mess = messageIn.readByte();
	if(mess==COMMAND_SEND_CMD_DATA_RE){
	   mess = messageIn.readByte();
	}
	if ( c == 0 ||c == -1) {
	  cont = false;
	} else {
	  cont = true;
	}
  }
  // This is EOF Write it
  messageOut.resetData();
  messageOut.writeByte(COMMAND_SEND_CMD_DATA);
  messageOut.writeByte('E');
  messageOut.writeByte('O');
  messageOut.writeByte('F');
  messageOut.writeByte('\r');
  messageOut.writeByte('\n');
  messageOut.sendData();

  while(!messageIn.receiveData());
  uint8_t len;
  uint8_t mess = messageIn.readByte();
  if(mess==COMMAND_SEND_CMD_DATA_RE){
	// Close the file and register the last pos
	afterDataSDReading();
  }
}

// process count command
void RobotControl::processFileSize() {
  uint32_t filesize = 0;
  // Read SDLastPos
  initDataSDReading(false);
  // Proceed filesize
  if (file.open(LOG_FILENAME, O_RDWR)) {
	filesize = file.fileSize();
	file.close();
  }
  // Send it to motor
  messageOut.writeByte(COMMAND_COUNTFILE_CMD_DATA);
  messageOut.writeInt32(filesize);
  messageOut.writeInt32(SDLastPos);
  messageOut.sendData();
  while(!messageIn.receiveData());
  uint8_t len;
  uint8_t mess = messageIn.readByte();
  if(mess==COMMAND_COUNTFILE_CMD_DATA_RE){
  }
}

// File to store the last posread
#define LAST_POS_FILE "LastPos.sd"

// To call before to read the SD card
// Initialize the reading to the last pos read
void RobotControl::initDataSDReading(bool openfile) {
  if (file.open(LAST_POS_FILE, O_RDWR )) {
    char result[20]; 
    uint8_t i = 0; // a counter
    char c =  1;
    while (c > 0 && i < (sizeof(result) - 1)) // while character isn't the string terminator
    {
      c = file.read(); // store character in result
      result[i++] = c; // increase counter
    }
    result[i] = '\0';

    SDLastPos = atol(result); // convert result string to numeric value
    file.close();    
  }
  else {
    SDLastPos = 0;
  } 
  if (openfile) {
	if (file.open(LOG_FILENAME, O_RDWR)) {
	  file.seekSet(SDLastPos);
	}
  }
}

// Read onebyte from last pos

// If 'initDataSDReading' has not been called, read from begining of file
char RobotControl::SDReadByteSinceFromPos() {
  if (!file.isOpen()) {
  file.open (LOG_FILENAME, O_RDWR);
    file.seekSet(SDLastPos);
  }
  char byte_read = file.read();
  SDLastPos++;
  return byte_read;  
}

// Save the last pos read into a file  
void RobotControl::afterDataSDReading() {
  // if log file is open , close it 
  if (file.isOpen()){
	SDLastPos = file.curPosition();
    file.close();
  }
  // Save the last position read 
  if (file.open(LAST_POS_FILE, O_RDWR | O_CREAT)) {
    file.println(SDLastPos);
    file.close();
  }  
}
// reset last pos read  
void RobotControl::reInitFile() {
  SDLastPos = 0;
  afterDataSDReading();
}

 



