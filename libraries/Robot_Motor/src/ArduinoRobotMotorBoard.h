#ifndef ArduinoRobot_h
#define ArduinoRobot_h

#include "EasyTransfer2.h"
#include "Multiplexer.h"
#include "LineFollow.h"
//#include "IRremote.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//Command code
#define COMMAND_SWITCH_MODE 0
#define COMMAND_RUN	10
#define COMMAND_MOTORS_STOP	11
#define COMMAND_ANALOG_WRITE	20
#define COMMAND_DIGITAL_WRITE	30
#define COMMAND_ANALOG_READ	40
#define COMMAND_ANALOG_READ_RE	41
#define COMMAND_DIGITAL_READ	50
#define COMMAND_DIGITAL_READ_RE	51
#define COMMAND_READ_IR	60
#define COMMAND_READ_IR_RE	61
#define COMMAND_ACTION_DONE 70
#define COMMAND_READ_TRIM 80
#define COMMAND_READ_TRIM_RE 81
#define COMMAND_PAUSE_MODE 90
#define COMMAND_LINE_FOLLOW_CONFIG 100
/*
#define COMMAND_READ_TH 110
#define COMMAND_READ_TH_RE 111
#define COMMAND_READ_GPS_TIMEDATE 120
#define COMMAND_READ_GPS_TIMEDATE_RE 121
#define COMMAND_READ_GPS_COORD 122
#define COMMAND_READ_GPS_COORD_RE 123
#define COMMAND_READ_CO2 130
#define COMMAND_READ_CO2_RE 132
#define COMMAND_READ_SOUND 140
#define COMMAND_READ_SOUND_RE 141
*/
#define COMMAND_READ_CMD 150
#define COMMAND_READ_CMD_RE 151
#define COMMAND_SEND_CMD_DATA 160
#define COMMAND_SEND_CMD_DATA_RE 161
#define COMMAND_READ_SENSORS 170
#define COMMAND_READ_SENSORS_RE 171
#define COMMAND_COUNTFILE_CMD_DATA 180
#define COMMAND_COUNTFILE_CMD_DATA_RE 181

//External command
#define COMMAND_EXT_NULL     0
#define COMMAND_EXT_ALIVE   'A'
#define COMMAND_EXT_STANDBY 'S'
#define COMMAND_EXT_GETDATA 'G'
#define COMMAND_EXT_INIT 'I'
#define COMMAND_EXT_REINITFILE 'R'
#define COMMAND_EXT_COUNTFILE 'C'
#define COMMAND_EXT_REP_OK 0
#define COMMAND_EXT_REP_KO 1
#define COMMAND_EXT_REP_OK_DATA 2 // + len

//component codename
#define CN_LEFT_MOTOR	0
#define CN_RIGHT_MOTOR	1
#define CN_IR 2

//motor board modes
#define MODE_SIMPLE 0
#define MODE_LINE_FOLLOW 1
#define MODE_ADJUST_MOTOR 2
#define MODE_IR_CONTROL 3

//bottom TKs, just for communication purpose
#define B_TK1 201
#define B_TK2 202
#define B_TK3 203
#define B_TK4 204

/*
A message structure will be:
switch mode (2):
	byte COMMAND_SWITCH_MODE, byte mode
run (5):
	byte COMMAND_RUN, int speedL, int speedR
analogWrite (3):
	byte COMMAND_ANALOG_WRITE, byte codename, byte value;
digitalWrite (3):
	byte COMMAND_DIGITAL_WRITE, byte codename, byte value;
analogRead (2):
	byte COMMAND_ANALOG_READ, byte codename;
analogRead _return_ (4):
	byte COMMAND_ANALOG_READ_RE, byte codename, int value;
digitalRead (2):
	byte COMMAND_DIGITAL_READ, byte codename;
digitalRead _return_ (4):
	byte COMMAND_DIGITAL_READ_RE, byte codename, int value;
read IR (1):
	byte COMMAND_READ_IR;
read IR _return_ (9):
	byte COMMAND_READ_IR_RE, int valueA, int valueB, int valueC, int valueD;
read TH (1):
	byte COMMAND_READ_TH;
read TH _return_ (4):
	byte COMMAND_READ_TH_RE, int valueA, int valueB;
read GPS_TIMEDATE (1):
	byte COMMAND_READ_GPS_TIMEDATE;
read GPS_TIMEDATE _return_ (8):
	byte COMMAND_READ_GPS_TIMEDATE_RE, char hour, char minute, char seconds, int milli, char day, char month , char year without century
read GPS_COORD (1):
	byte COMMAND_READ_GPS_COORD;
read GPS_COORD _return_ (14):
	byte COMMAND_READ_GPS_COORD_RE, char fix, char satnb, float latitude, float longitude, float altitude
read SOUND (1):
	byte COMMAND_READ_SOUND;
read SOUND _return_ (4):
	byte COMMAND_READ_SOUND_RE, float sound
read CO2 (1):
	byte COMMAND_READ_CO2;
read CO2 _return_ (4):
	byte COMMAND_READ_CO2_RE, float co2density
read CMD _return_ (var):
	byte COMMAND_READ_CMD_RE, command


*/

class RobotMotorBoard:public LineFollow{
	public:
		RobotMotorBoard();
		void begin();
		void beginGps();
		
		void process();
		void processGps();
		
		void parseCommand();
		
		int IRread(uint8_t num);
		
		void setMode(uint8_t mode);
		void pauseMode(bool onOff);
		
		void motorsWrite(int speedL, int speedR);
		void motorsWritePct(int speedLpct, int speedRpct);//write motor values in percentage
		void motorsStop();		
		void correctDateHour();
	private:
		float motorAdjustment;//-1.0 ~ 1.0, whether left is lowered or right is lowered
		
		//convert codename to actual pins
		uint8_t parseCodename(uint8_t codename);
		uint8_t codenameToAPin(uint8_t codename);
		
		void stopCurrentActions();
		//void sendCommand(byte command,byte codename,int value);
		
		void _analogWrite(uint8_t codename, int value);
		void _digitalWrite(uint8_t codename, bool value);
		void _analogRead(uint8_t codename);
		void _digitalRead(uint8_t codename);
		int _IRread(uint8_t num);
		void _readIR();
		void _readTrim();
		void _readSensors();
	    void _collectSensors();
		/*
		void _readTH();
		void _readGPSTimeDate();
		void _readGPSCoord();
		void _readCO2();
		void _readSoundLevel();
		*/		
		void _refreshMotorAdjustment();
     	void _readCmd();
     	void _sendCmdData();
		void _sendCountfileData(uint32_t filesize,uint32_t lastpos);

		Multiplexer IRs;
		uint8_t mode;
		uint8_t isPaused;
		EasyTransfer2 messageIn;
		EasyTransfer2 messageOut;
		
		//Line Following 
		void reportActionDone();
};

extern RobotMotorBoard RobotMotor;

#endif
