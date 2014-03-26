#include "EasyTransfer2.h"




//Captures address and size of struct
void EasyTransfer2::begin(HardwareSerial *theSerial){
	_serial = theSerial;
	
	//dynamic creation of rx parsing buffer in RAM
	//rx_buffer = (uint8_t*) malloc(size);
	
	resetData();
}

void EasyTransfer2::writeFloat(float dat){
    writeBuffer(sizeof(dat), (uint8_t *)&dat);
}

float EasyTransfer2::readFloat(){
    float dat=0;
    readBuffer(sizeof(dat), (uint8_t *)&dat);
	return dat;
}

void EasyTransfer2::writeBuffer(uint8_t bufsize, uint8_t *bufdata){
	uint8_t i;
	for ( i=0; i < bufsize; i++ )
	  writeByte(bufdata[i]);
}

void EasyTransfer2::readBuffer(uint8_t bufsize, uint8_t *bufdata){
	uint8_t i;
	for ( i=0; i < bufsize; i++ )
	  bufdata[i] = readByte();
}

void EasyTransfer2::writeByte(uint8_t dat){
	if(position<MAXSIZE)
		data[position++]=dat;
		size++;
}

uint8_t EasyTransfer2::readByte(){
	if(position>=size)return 0;
	return data[position++];
}

void EasyTransfer2::writeInt(int dat){
    writeBuffer(sizeof(dat), (uint8_t *)&dat);
}

int EasyTransfer2::readInt(){
    int dat=0;
    readBuffer(sizeof(dat), (uint8_t *)&dat);
	return dat;
}

void EasyTransfer2::writeInt32(uint32_t  dat){
    writeBuffer(sizeof(dat), (uint8_t *)&dat);
}

uint32_t EasyTransfer2::readInt32(){
    uint32_t dat=0;
    readBuffer(sizeof(dat), (uint8_t *)&dat);
	return dat;
}

void EasyTransfer2::resetData(){
    memset(data, 0, MAXSIZE);
	size=0;
	position=0;
}

//Sends out struct in binary, with header, length info and checksum
void EasyTransfer2::sendData(){
  uint8_t CS = size;
  _serial->write(0x06);
  _serial->write(0x85);
  _serial->write(size);
  for(int i = 0; i<size; i++){
    CS^=*(data+i);
    _serial->write(*(data+i));
	//Serial.print(*(data+i));
	//Serial.print(",");
  }
  //Serial.println("");
  _serial->write(CS);
  
  resetData();
}

boolean EasyTransfer2::receiveData(){
  
  //start off by looking for the header bytes. If they were already found in a previous call, skip it.
  if(rx_len == 0){
  //this size check may be redundant due to the size check below, but for now I'll leave it the way it is.
    if(_serial->available() >= 3){
	//this will block until a 0x06 is found or buffer size becomes less then 3.
      while(_serial->read() != 0x06) {
		//This will trash any preamble junk in the serial buffer
		//but we need to make sure there is enough in the buffer to process while we trash the rest
		//if the buffer becomes too empty, we will escape and try again on the next call
		if(_serial->available() < 3)
			return false;
		}
		//Serial.println("head");
      if (_serial->read() == 0x85){
        rx_len = _serial->read();
		//Serial.print("rx_len:");
		//Serial.println(rx_len);
		resetData();

		//make sure the binary structs on both Arduinos are the same size.
        /*if(rx_len != size){
          rx_len = 0;
          return false;
        }*/
      }
    }
	//Serial.println("nothing");
  }
  
  //we get here if we already found the header bytes, the struct size matched what we know, and now we are byte aligned.
  if(rx_len != 0){
	
    while(_serial->available() && rx_array_inx <= rx_len){
      data[rx_array_inx++] = _serial->read();
    }
    
    if(rx_len == (rx_array_inx-1)){
      //seem to have got whole message
      //last uint8_t is CS
      calc_CS = rx_len;
	  //Serial.print("len:");
	  //Serial.println(rx_len);
      for (int i = 0; i<rx_len; i++){
        calc_CS^=data[i];
		//Serial.print("m");
		//Serial.print(data[i]);
		//Serial.print(",");
      } 
	  //Serial.println();
      //Serial.print(data[rx_array_inx-1]);
	  //Serial.print(" ");
	  //Serial.println(calc_CS);
	  
      if(calc_CS == data[rx_array_inx-1]){//CS good
		//resetData();
        //memcpy(data,d,rx_len);
		for(int i=0;i<MAXSIZE;i++){
			//Serial.print(data[i]);
			//Serial.print(",");
		}
		//Serial.println("");
		size=rx_len;
		rx_len = 0;
		rx_array_inx = 0;
		return true;
		}
		
	  else{
  		//Serial.println("CS");
		resetData();
	  //failed checksum, need to clear this out anyway
		rx_len = 0;
		rx_array_inx = 0;
		return false;
	  }
        
    }
  }
  //Serial.print(rx_len);
  //Serial.print(" ");
  //Serial.print(rx_array_inx);
  //Serial.print(" ");
  //Serial.println("Short");
  return false;
}
