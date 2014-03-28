#include "ArduinoRobotMotorBoard.h"
#include "EasyTransfer2.h"
#include "Multiplexer.h"
#include "LineFollow.h"
// Read hygro and temp
#include "DHT.h"
// Decoding gps frame
#include "Adafruit_GPS.h"
// Communicate witrh gps
#include "SoftwareSerialRobot.h"
// Decoding CO2 sensor
#include "CO2Sensor.h"
// Decoding Sound sensor
#include "SoundSensor.h"
// To proceed env rating
#include "EnvRating.h"
// To manage time
#include "Time.h"

// Set to debug to serial
//#define DEBUG_MODE 1

/*
 Circuit:
 * Arduino Robot : motor board
 * dht thermo + hygro on pin TK4 http://snootlab.com/adafruit/252-capteur-de-temperature-et-humidite-am2302.html
 * CO2 sensor on pin TK2 http://www.dfrobot.com/index.php?route=product/product&product_id=1023#.UwnntPl5N8E
 * Sound sensor on pin TK1 https://www.adafruit.com/products/1063 (http://snootlab.com/lang-en/adafruit/388-electret-microphone-amplifier-max4466-with-adjustable-gain.html)
 * gps serial RX = MOSI = P4 TX = TK3 (rx gps --> tx motor) http://snootlab.com/lang-en/adafruit/382-adafruit-ultimate-gps-breakout-en.html
 * ble link serial RX = MISO = P1 TX = SCK = P3 (rx ble --> tx motor) http://www.dfrobot.com/index.php?route=product/product&product_id=1073#.Uxpwufl5N8E
black connector usage : 
-----     -----
|P5 P3 P1 |
|P6 P4 P2 |
--------------
 
P1 = MISO
P2 = VTG
P3 = SCK
P4 = MOSI
P5 = RST
P6 = GND
            up side
------------     -------------
|  RST      SCK         MISO |
|  GND      MOSI         VTG |
------------------------------
*/

//----------------------------------------------------------
// PIN and object for sensor
//----------------------------------------------------------
// dht thermo + hydro
#define DHTPIN TK4 
// Type of sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
// Object to thermo + hygro
DHT dht(DHTPIN, DHTTYPE);

// Object for CO2Sensor on Analog pin 
CO2Sensor gasSensor(TK2);

// Pin for sound sensor
#define SOUND_PIN TK1

// Storing all information
awbbSensorData awbbSensorDataBuf;

// Software serial for gps
SoftwareSerial gpsSerial(MOSI,TK3);
// Give to gps record the point of awbb record to store all information in only one place.
Adafruit_GPS GPS(&gpsSerial,&awbbSensorDataBuf);

// Software serial for bluetooth
SoftwareSerial btSerial(MISO,SCK);

// Contructor
RobotMotorBoard::RobotMotorBoard(){
	// Do nothing
}

// Initialize Gps
void RobotMotorBoard::beginGps(){
    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
    // Output max info
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// Refresh each 1 seconds
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
	// Wait a litle
    delay(100);
	// Close the gps
    gpsSerial.end();
}


// Begin the robot
void RobotMotorBoard::begin(){
	//initialize communication between boards
	Serial1.begin(9600);
	messageIn.begin(&Serial1);
	messageOut.begin(&Serial1);
	// Initialize bluetooth module
	btSerial.begin(19200);
	// Initialize thermo + hygro
	dht.begin();
	
	isPaused=false;
}

// Memory of last timer
uint32_t timer = millis();

// Process gps :
// Cutoff the bluetooth serial
// Wait a command from gps
// Register it in common record
void RobotMotorBoard::processGps(){
  bool notRec = true;
  // Cutoff bluetooth
  btSerial.end();
  // Begin gps
  gpsSerial.begin(9600);
  char c;
  // While we have not a record
  while (notRec ) {
	// reading a char from gps serial line
	c = GPS.read();
	if (c != 0 ) 
	  Serial.print(c);
	// According to the GPS, my location is 4042.6142,N (Latitude 40 degrees, 42.6142 decimal minutes North) & 07400.4168,W. (Longitude 74 degrees, 0.4168 decimal minutes West) To look at this location in Google maps, type +40° 42.6142', -74° 00.4168' into the google maps search box . Unfortunately gmaps requires you to use +/- instead of NSWE notation. N and E are positive, S and W are negative.
	//People often get confused because the GPS is working but is "5 miles off" - this is because they are not parsing the lat/long data correctly. Despite appearances, the geolocation data is NOT in decimal degrees. It is in degrees and minutes in the following format: Latitude: DDMM.MMMM (The first two characters are the degrees.) Longitude: DDDMM.MMMM (The first three characters are the degrees.)
	// We met the next timer 
	if (GPS.newNMEAreceived()) {
	  // a tricky thing here is if we print the NMEA sentence, or data
	  // we end up not listening and catching other sentences! 
	  // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
	  if (GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
		notRec = false; // we can fail to parse a sentence in which case we should just wait for another
	  
	}
  }
  // Correct the gps date and hour with the time of init command
  correctDateHour();
  // End of gps
  gpsSerial.end();
  // Yes, we can receive command
  btSerial.begin(19200);
}

// Correct the gps date and time
void RobotMotorBoard::correctDateHour() {
    bool correct = false;
	// Correct hour if = 0:0:0 : robot not working at 0:0:0 (perhaps yes but we asume) 
	if (awbbSensorDataBuf.hour == 0 && awbbSensorDataBuf.minute == 0 && awbbSensorDataBuf.seconds == 0 ) {
	  // Time status is ok
	  if (  timeStatus() == timeSet ) {
		// Complete all hour information
		awbbSensorDataBuf.hour = hour();
		awbbSensorDataBuf.minute = minute();
		awbbSensorDataBuf.seconds = second();
	  }
	  // We do a correction
	  correct = true;
	}
	// Correct date if year no good
	if ( awbbSensorDataBuf.year < 10 || awbbSensorDataBuf.year > 25  ) {
	  if (  timeStatus() == timeSet ) {
		awbbSensorDataBuf.year = year()-2000;
		awbbSensorDataBuf.month = month();
		awbbSensorDataBuf.day = day();
	  }
	  correct = true;
	}
	// This is a good gps time and date, register it for future in the time module.
	if ( !correct ) {
	  setTime(awbbSensorDataBuf.hour,
			  awbbSensorDataBuf.minute,
			  awbbSensorDataBuf.seconds,
			  awbbSensorDataBuf.day,
			  awbbSensorDataBuf.month,
			  awbbSensorDataBuf.year);
	}
}

// Counter char and other think for cmd
char c;
char cpt=0;
char cmd[20];
bool cmdok = false;

// Process bt command
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
	// Bluetooth command reception
	 if ( btSerial.available()) {
	   c = btSerial.read();
	   if ( c == '$' ) {
		 cpt = 0;
		 // If End of command caractere
	   } else if ( c == '\n' || c == '%') {
		 cmd[cpt++] = 0;
		 cmdok = true;
		 Serial.print("Command : '");
		 Serial.print(cmd);
		 Serial.println("'");
		 // Reply ok directly to acq (not hack ) the command
		 btSerial.print("$");
		 btSerial.print(cmd[0]);
		 btSerial.print("OK\r\n");
		 // Direct implementation of Init command
		 // Register the time in time module.
		 if (cmd[0] == COMMAND_EXT_INIT) {
		     time_t montime = atol(cmd+1);
			 setTime(montime);
			 Serial.print("Setting time ");
			 Serial.println(montime);
			 Serial.println(second());
			 Serial.println(minute());
			 Serial.println(hour());
			 Serial.println(day());
			 Serial.println(month());
			 Serial.println(year());
		 }
	   } else {
		 // REgister the received caractere in command buffer
		 if ( cpt +1 < sizeof(cmd)) {
		   cmd[cpt++] = c;
		 }
	   }
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

// Parse inter Robot control - motor command
void RobotMotorBoard::parseCommand(){
	uint8_t modeName;
	uint8_t codename;
	int value;
	int speedL;
	int speedR;
	if(this->messageIn.receiveData()){
#ifdef DEBUG_MODE
		Serial.println("data received");
#endif
		uint8_t command=messageIn.readByte();
#ifdef DEBUG_MODE
		Serial.println(command);
#endif
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
			case COMMAND_READ_CMD:
				_readCmd();
				break;
			case COMMAND_SEND_CMD_DATA:
				_sendCmdData();
				break;
			case COMMAND_COUNTFILE_CMD_DATA:
			  {
			    uint32_t filesize;
			    uint32_t lastpos;
			    filesize=messageIn.readInt32();
			    lastpos=messageIn.readInt32();
				_sendCountfileData(filesize,lastpos);
			  }
				break;
			case COMMAND_READ_SENSORS:
				_readSensors();
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

// read a cmd from bluetooth
void RobotMotorBoard::_readCmd(){
  byte status;
  uint8_t i;
  messageOut.writeByte(COMMAND_READ_CMD_RE);
  if ( cmdok ) {
	messageOut.writeByte(cpt);
	for ( i=0; i < cpt; i++ )
	  messageOut.writeByte(cmd[i]);
  } else {
	messageOut.writeByte(0);
  }
  messageOut.sendData();
  cmdok = false;
}

// receive data from sdcard
void RobotMotorBoard::_sendCmdData(){
  uint8_t i;
  Serial.print("_sendCmdData ");  
  uint8_t c = -1;
  uint8_t cpt = 0;
  btSerial.write('$');
  btSerial.write('G');
  while (c != 0 && c != '\n') {
	c = messageIn.readByte();
	if (btSerial.overflow()) Serial.println("BLEoverflow");
	btSerial.write(c);
	cpt ++;
  }
  
  Serial.println(cpt);
  messageOut.writeByte(COMMAND_SEND_CMD_DATA_RE);
  //  while(btSerial.available()) ;
  c = btSerial.read();
  messageOut.writeByte(c);
  messageOut.sendData();
}

// process count cmd
void RobotMotorBoard::_sendCountfileData(uint32_t filesize,uint32_t lastpos){
  Serial.print("_sendCountfileData ");  
  btSerial.write('$');
  btSerial.write('C');
  btSerial.print("FILESIZE:");
  btSerial.print(filesize);
  btSerial.print(";");
  btSerial.println(lastpos);
  
  messageOut.writeByte(COMMAND_COUNTFILE_CMD_DATA_RE);
  messageOut.sendData();
}

// collect local sensor
void RobotMotorBoard::_collectSensors(){
	// Temperature/Hygro
	awbbSensorDataBuf.hygro = dht.readHumidity();
	awbbSensorDataBuf.thermo = dht.readTemperature();
	// Sound Level
	awbbSensorDataBuf.vSound = readSoundSensor(SOUND_PIN);
	// CO2 density
	gasSensor.read();
    awbbSensorDataBuf.CO2Density = gasSensor.getPPM();
  
}

// Readsensor and send the record to control
void RobotMotorBoard::_readSensors(){
    Serial.println("_readSensors");
	// Read gps information awbbSensorDataBuf is completed with
	processGps();
	// Collect all motor sensor value
	_collectSensors();
	// Complete with ligth on control board
	awbbSensorDataBuf.vLight = messageIn.readInt();
	EnvRating envRating;
	// Calcul rating
    envRating.addLight(awbbSensorDataBuf.vLight);
    envRating.addTemp(awbbSensorDataBuf.hygro);
    envRating.addHygro(awbbSensorDataBuf.hygro);
    envRating.addCO2(awbbSensorDataBuf.CO2Density);
    envRating.addSound(awbbSensorDataBuf.vSound);
    // Process the rating
    awbbSensorDataBuf.rating = envRating.getRating();
	// Send the all data
	messageOut.writeByte(COMMAND_READ_SENSORS_RE);
	messageOut.writeBuffer(sizeof(awbbSensorDataBuf),(uint8_t *)&awbbSensorDataBuf);
	messageOut.sendData();	
}

RobotMotorBoard RobotMotor=RobotMotorBoard();
