#include "ArduinoRobotMotorBoard.h"
#include "EasyTransfer2.h"
#include "Multiplexer.h"
#include "LineFollow.h"
// Read hygro and temp
#include "DHT.h"
// Decoding gps frame
#include "Adafruit_GPS.h"
// Communicate witrh gps
#include "SoftwareSerial.h"
// Decoding CO2 sensor
#include "CO2Sensor.h"
// Decoding Sound sensor
#include "SoundSensor.h"
/*
 Circuit:
 * Arduino Robot : motor board
 * dht thermo + hygro on pin TK4 http://snootlab.com/adafruit/252-capteur-de-temperature-et-humidite-am2302.html
 * CO2 sensor on pin TK2 http://www.dfrobot.com/index.php?route=product/product&product_id=1023#.UwnntPl5N8E
 * Sound sensor on pin TK1 https://www.adafruit.com/products/1063
 
*/

//----------------------------------------------------------
// PIN and object for sensor
//----------------------------------------------------------
// dht thermo + hydro
#define DHTPIN TK4 
// Type of sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// Object
DHT dht(DHTPIN, DHTTYPE);

// Object for CO2Sensor on Analog pin 
CO2Sensor gasSensor(TK2);

// Pin for sound sensor
#define SOUND_PIN TK1

// Software serial for gps
SoftwareSerial gpsSerial(MOSI,TK3);
Adafruit_GPS GPS(&gpsSerial);

// Software serial for bluetooth
//SoftwareSerial btSerial(MISO,TK4);

//----------------------------------------------------------
// Interupt to receive data on software serial
//----------------------------------------------------------
// Interrupt is called once a millisecond, looks for any new GPS data, and stores it Note : to be confirmed for use with btSerial
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
    if (c) Serial.print(c);   
}

// To set interupt for reading gps : Note : to be confirmed for use with btSerial
bool usingInterrupt;
void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

RobotMotorBoard::RobotMotorBoard(){
	// Do nothing
}

void RobotMotorBoard::beginGps(){
    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);

	// Output max info
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// Refresh each 5 seconds
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5SEC);
	 
}


void RobotMotorBoard::begin(){
	//initialze communication between boards
	Serial1.begin(9600);
	messageIn.begin(&Serial1);
	messageOut.begin(&Serial1);

	//init MUX
	/*
	uint8_t MuxPins[]={MUXA,MUXB,MUXC};
	this->IRs.begin(MuxPins,MUX_IN,3);
	pinMode(MUXI,INPUT);
	digitalWrite(MUXI,LOW);
	*/
	dht.begin();
	isPaused=false;
}

// Memory of last timer
uint32_t timer = millis();
void RobotMotorBoard::processGps(){
  // reading a char from gps serial line
  char c = GPS.read();

// According to the GPS, my location is 4042.6142,N (Latitude 40 degrees, 42.6142 decimal minutes North) & 07400.4168,W. (Longitude 74 degrees, 0.4168 decimal minutes West) To look at this location in Google maps, type +40° 42.6142', -74° 00.4168' into the google maps search box . Unfortunately gmaps requires you to use +/- instead of NSWE notation. N and E are positive, S and W are negative.
//People often get confused because the GPS is working but is "5 miles off" - this is because they are not parsing the lat/long data correctly. Despite appearances, the geolocation data is NOT in decimal degrees. It is in degrees and minutes in the following format: Latitude: DDMM.MMMM (The first two characters are the degrees.) Longitude: DDDMM.MMMM (The first three characters are the degrees.)
  // We met the next timer 
  if (millis() > timer)
	{
	  timer = millis() + 5000;
	  if (GPS.newNMEAreceived()) {
		// a tricky thing here is if we print the NMEA sentence, or data
		// we end up not listening and catching other sentences! 
		// so be very wary if using OUTPUT_ALLDATA and trytng to print out data
		Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
		if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
		  return;  // we can fail to parse a sentence in which case we should just wait for another
		Serial.print("\nTime: ");
		Serial.print(GPS.hour, DEC); Serial.print(':');
		Serial.print(GPS.minute, DEC); Serial.print(':');
		Serial.print(GPS.seconds, DEC); Serial.print('.');
		Serial.println(GPS.milliseconds);
		Serial.print("Date: ");
		Serial.print(GPS.day, DEC); Serial.print('/');
		Serial.print(GPS.month, DEC); Serial.print("/20");
		Serial.println(GPS.year, DEC);
		Serial.print("Fix: "); Serial.print((int)GPS.fix);
		Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
		if (GPS.fix) {
		  Serial.print("Location: ");
		  if (GPS.lat == 'S') { GPS.latitude *= -1; }
		  Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
		  Serial.print(", "); 
		  if (GPS.lon == 'W') { GPS.longitude *= -1; }
		  Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
		  
		  Serial.print("Speed (knots): "); Serial.println(GPS.speed);
		  Serial.print("Angle: "); Serial.println(GPS.angle);
		  Serial.print("Altitude: "); Serial.println(GPS.altitude);
		  Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
		}
	  }
	  
	}//if (millis() > updateTime)
  // if a sentence is received, we can check the checksum, parse it...
}

void RobotMotorBoard::process(){
	if(isPaused)return;//skip process if the mode is paused
	if(mode==MODE_SIMPLE){
		//Serial.println("s");
		//do nothing? Simple mode is just about getting commands

	}else if(mode==MODE_LINE_FOLLOW){
		//do line following stuff here.
		LineFollow::runLineFollow();
	}else if(mode==MODE_ADJUST_MOTOR){
		Serial.println('a');
		//motorAdjustment=analogRead(POT);
		//setSpeed(255,255);
		//delay(100);
	}
}
void RobotMotorBoard::pauseMode(bool onOff){
	if(onOff){
		isPaused=true;
	}else{
		isPaused=false;
	}
	stopCurrentActions();

}
void RobotMotorBoard::parseCommand(){
	uint8_t modeName;
	uint8_t codename;
	int value;
	int speedL;
	int speedR;
	if(this->messageIn.receiveData()){
		//Serial.println("data received");
		uint8_t command=messageIn.readByte();
		//Serial.println(command);
		switch(command){
			case COMMAND_SWITCH_MODE:
				modeName=messageIn.readByte();
				setMode(modeName);
				break;
			case COMMAND_RUN:
				if(mode==MODE_LINE_FOLLOW)break;//in follow line mode, the motor does not follow commands
				speedL=messageIn.readInt();
				speedR=messageIn.readInt();
				motorsWrite(speedL,speedR);
				break;
			case COMMAND_MOTORS_STOP:
				motorsStop();
				break;
			case COMMAND_ANALOG_WRITE:
				codename=messageIn.readByte();
				value=messageIn.readInt();
				_analogWrite(codename,value);
				break;
			case COMMAND_DIGITAL_WRITE:
				codename=messageIn.readByte();
				value=messageIn.readByte();
				_digitalWrite(codename,value);
				break;
			case COMMAND_ANALOG_READ:
				codename=messageIn.readByte();
				_analogRead(codename);
				break;
			case COMMAND_DIGITAL_READ:
				codename=messageIn.readByte();
				_digitalRead(codename);
				break;
			case COMMAND_READ_IR:
				_readIR();
				break;
			case COMMAND_READ_TRIM:
				_readTrim();
				break;
			case COMMAND_PAUSE_MODE:
				pauseMode(messageIn.readByte());//onOff state
				break;
			case COMMAND_LINE_FOLLOW_CONFIG:
				LineFollow::config(
					messageIn.readByte(),	//KP
					messageIn.readByte(),	//KD
					messageIn.readByte(),	//robotSpeed
					messageIn.readByte()	//IntegrationTime
				);
				break;
			case COMMAND_READ_TH:
				_readTH();
				break;
			case COMMAND_READ_GPS_TIMEDATE:
				_readGPSTimeDate();
				break;
			case COMMAND_READ_GPS_COORD:
				_readGPSCoord();
				break;
			case COMMAND_READ_CO2:
				_readCO2();
				break;
			case COMMAND_READ_SOUND:
				_readSoundLevel();
				break;
		}
	}
	//delay(5);
}
uint8_t RobotMotorBoard::parseCodename(uint8_t codename){
	switch(codename){
		case B_TK1:
			return TK1;
		case B_TK2:
			return TK2;
		case B_TK3:
			return TK3;
		case B_TK4:
			return TK4;
	}
}
uint8_t RobotMotorBoard::codenameToAPin(uint8_t codename){
	switch(codename){
		case B_TK1:
			return A0;
		case B_TK2:
			return A1;
		case B_TK3:
			return A6;
		case B_TK4:
			return A11;
	}
}

void RobotMotorBoard::setMode(uint8_t mode){
	if(mode==MODE_LINE_FOLLOW){
		LineFollow::calibIRs();
	}
	/*if(mode==SET_MOTOR_ADJUSTMENT){
			save_motor_adjustment_to_EEPROM();
		}
	*/
	/*if(mode==MODE_IR_CONTROL){
		beginIRReceiver();
	}*/
	this->mode=mode;
	//stopCurrentActions();//If line following, this should stop the motors
}

void RobotMotorBoard::stopCurrentActions(){
	motorsStop();
	//motorsWrite(0,0);
}

void RobotMotorBoard::motorsWrite(int speedL, int speedR){
	/*Serial.print(speedL);
	Serial.print(" ");
	Serial.println(speedR);*/
	//motor adjustment, using percentage
	_refreshMotorAdjustment();
	
	if(motorAdjustment<0){
		speedR*=(1+motorAdjustment);
	}else{
		speedL*=(1-motorAdjustment);
	}
	
	if(speedL>0){
		analogWrite(IN_A1,speedL);
		analogWrite(IN_A2,0);
	}else{
		analogWrite(IN_A1,0);
		analogWrite(IN_A2,-speedL);
	}
	
	if(speedR>0){
		analogWrite(IN_B1,speedR);
		analogWrite(IN_B2,0);
	}else{
		analogWrite(IN_B1,0);
		analogWrite(IN_B2,-speedR);
	}
}
void RobotMotorBoard::motorsWritePct(int speedLpct, int speedRpct){
	//speedLpct, speedRpct ranges from -100 to 100
	motorsWrite(speedLpct*2.55,speedRpct*2.55);
}
void RobotMotorBoard::motorsStop(){
	analogWrite(IN_A1,255);
	analogWrite(IN_A2,255);

	analogWrite(IN_B1,255);
	analogWrite(IN_B2,255);
}


/*
*
*
*	Input and Output ports
*
*
*/
void RobotMotorBoard::_digitalWrite(uint8_t codename,bool value){
	uint8_t pin=parseCodename(codename);
	digitalWrite(pin,value);
}
void RobotMotorBoard::_analogWrite(uint8_t codename,int value){
	//There's no PWM available on motor board
}
void RobotMotorBoard::_digitalRead(uint8_t codename){
	uint8_t pin=parseCodename(codename);
	bool value=digitalRead(pin);
	messageOut.writeByte(COMMAND_DIGITAL_READ_RE);
	messageOut.writeByte(codename);
	messageOut.writeByte(value);
	messageOut.sendData();
}
void RobotMotorBoard::_analogRead(uint8_t codename){
	uint8_t pin=codenameToAPin(codename);
	int value=analogRead(pin);
	messageOut.writeByte(COMMAND_ANALOG_READ_RE);
	messageOut.writeByte(codename);
	messageOut.writeInt(value);
	messageOut.sendData();
}
int RobotMotorBoard::IRread(uint8_t num){
	return _IRread(num-1); //To make consistant with the pins labeled on the board
}

int RobotMotorBoard::_IRread(uint8_t num){
	IRs.selectPin(num); 
	return IRs.getAnalogValue();
}


void RobotMotorBoard::_readIR(){
	int value;
	messageOut.writeByte(COMMAND_READ_IR_RE);
	for(int i=0;i<5;i++){
		value=_IRread(i);
		messageOut.writeInt(value);
	}
	messageOut.sendData();
}

void RobotMotorBoard::_readTrim(){
	int value=analogRead(TRIM);
	messageOut.writeByte(COMMAND_READ_TRIM_RE);
	messageOut.writeInt(value);
	messageOut.sendData();	
}

void RobotMotorBoard::_refreshMotorAdjustment(){
	motorAdjustment=map(analogRead(TRIM),0,1023,-30,30)/100.0;
}

void RobotMotorBoard::reportActionDone(){
	setMode(MODE_SIMPLE);
	messageOut.writeByte(COMMAND_ACTION_DONE);
	messageOut.sendData();
}

void RobotMotorBoard::_readTH(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
	Serial.print(h);
	Serial.print(" ");
	Serial.println(t);
  
	messageOut.writeByte(COMMAND_READ_TH_RE);
	messageOut.writeInt(t*10);
	messageOut.writeInt(h*10);
	messageOut.sendData();	
}

void RobotMotorBoard::_readGPSTimeDate(){
	
	messageOut.writeByte(COMMAND_READ_GPS_TIMEDATE_RE);
	messageOut.writeByte(GPS.hour);
	messageOut.writeByte(GPS.minute);
	messageOut.writeByte(GPS.seconds);
	messageOut.writeInt(GPS.milliseconds);
	messageOut.writeByte(GPS.day);
	messageOut.writeByte(GPS.month);
	messageOut.writeByte(GPS.year);
    messageOut.sendData();	
}

// Used to convert Bytes to float
union float2bytes { float f; char b[sizeof(float)]; };

void RobotMotorBoard::_readGPSCoord(){
	
	messageOut.writeByte(COMMAND_READ_GPS_COORD_RE);
	messageOut.writeByte(GPS.fix);
	messageOut.writeByte(GPS.satellites);

	float2bytes f2b;
	uint8_t i;
	f2b.f = GPS.latitude;
	for ( i=0; i < sizeof(float); i++ )
		messageOut.writeByte(f2b.b[i]);
	f2b.f = GPS.longitude;
	for ( i=0; i < sizeof(float); i++ )
		messageOut.writeByte(f2b.b[i]);
	f2b.f = GPS.altitude;
	for ( i=0; i < sizeof(float); i++ )
		messageOut.writeByte(f2b.b[i]);
	messageOut.sendData();	
}

void RobotMotorBoard::_readSoundLevel(){
	messageOut.writeByte(COMMAND_READ_SOUND_RE);
	float2bytes f2b;
	uint8_t i;
	f2b.f = readSoundSensor(SOUND_PIN);
	Serial.println("readSoundSensor");
	Serial.println(f2b.f);
	for ( i=0; i < sizeof(float); i++ )
		messageOut.writeByte(f2b.b[i]);
	messageOut.sendData();	
}


void RobotMotorBoard::_readCO2(){
	gasSensor.read();
    int CO2Density = gasSensor.getPPM();
	messageOut.writeByte(COMMAND_READ_CO2_RE);
	messageOut.writeInt(CO2Density);
	messageOut.sendData();
}


RobotMotorBoard RobotMotor=RobotMotorBoard();