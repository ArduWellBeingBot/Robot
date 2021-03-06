/* awbb control based on runaway_robot

Moving the arduino robot (based on code of sample runaway_robot.)
When stop or each MAX_RUNNING_TIME ms take measure of :
gps data, temperature, hydrometrie, co2, sound, ligth
  
 Circuit:
 * Arduino Robot
 * US range finder Maxbotix EZ10, with analog output on TK2/M2
 * Servo on TKD4 (the only pwm)
 * LDR (with pull-up resistor) on TK4/M4
 
 Modified code of robot_motor to support : CO2, gps, temperature and hudrometrie captor, sound level, bluetooth module
  
 created 11 February 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain
 */

// include a modified version of the robot library
// Disable melodie.c squaw*. to disable timer usage for servo and to have more space.
// We cannot use melodie anymore.
#include "ArduinoRobot.h"
// Integration of modified version of servo module
#include "ServoRobot.h"

// time in ms between each measure. 
// Robot will stop, take data and select a new direction
#define MAX_RUNNING_TIME 20000

// PIN
uint8_t sensorPin      = TK2;  // M2 : pin of the range sensor
uint8_t servoPin       = TKD4;  // pin of the servo
// Note the servo is front of the robot when angle is 90 Degres.
uint8_t lightSensorPin  = TK4;    // M4 : pin of the LDR

// Servo object
Servo servo;

// To manage data into the µSDCard
//#define CSV_SEPARATOR F(";")


// To know if the robot has to move (external control)
bool MOVE;

// Flag to know if we change the direction
uint8_t flag=0;
// Distance on front
int distfront = 0;
// Others distance
int dist;
// Distance max found
int distmax;
// Indice of angle found
int imax = 0;
// Table of angle
int disttab[4];
// Degres for each position
unsigned char deg[4]  = {
  170,
  135,
  45,
  10
  };
// angle to turn by indice
int turn[4]  = {
  -90,
  -45,
  45,
  90
  };
// Counter of distance
int countdist;

byte mode ;
#define MODE_STANDBY 1
#define MODE_ALIVE 2
#define MODE_GETDATA 3
#define MODE_MEASURE 4
#define MODE_ONLYMEASURE 5

  uint8_t pinleft = TK3;
  uint8_t pinrigth = TK1;


int cptleft;
int cptrigth;
int mill;
int lastmill;
uint8_t moteurleft = 255;
uint8_t moteurrigth = 255;


// print step
//#define DEBUGSTEP 1
//#define DEBUGNOMOTOR 1
//#define DEBUGNOSERVO 1
// Setup function
void setup() {
  // initialize the Robot, SD card, and display
  Serial.begin(19200);
  Robot.begin();
  Robot.beginTFT();
  // Attach the servo
  servo.attach(servoPin);
  // Put in in front
  servo.write(90);
  // some delay
  delay(150);
  
  // Replace Robot.beginSD() to avoid to call 'melody'
  // SD reader init on pin CARD_CS (D8) of the 32u4
  Robot.card.init(0,CARD_CS);
  // return false if initialization failed
  Robot.file.init(&Robot.card);
  // Activate moving
  MOVE = true;  
  // GPS data struct initilization
  memset ((void*)&Robot.awbbSensorDataBuf,0,sizeof(Robot.awbbSensorDataBuf));
  mode = MODE_ALIVE;
}// End Setup

// Collect and Store data
void CollectAndStoreData() {
    //----------------------------------------------------------
    // Collect all the data from sensors
    //----------------------------------------------------------
    // - light : light is sent to the motor to process rating
    int vLight = Robot.analogRead(lightSensorPin);

    Robot.readSensorsData(vLight,Robot.awbbSensorDataBuf);

    //----------------------------------------------------------
    // Save the collected data and the environment rating
    //----------------------------------------------------------

    // Save the data into µSD
    if (Robot.file.open(LOG_FILENAME, O_RDWR | O_CREAT | O_APPEND)) {
      // Write a line
      //« YYYY-MM-DDTHH :MM :SS.mmm ;FIX;SAT;LAT ;LONG ;ALT ;LIGHT ;TEMP ;HYGRO ;CO2 ;MICRO ;NOTE »
      Robot.file.print(F("20"));
      Robot.file.print(Robot.awbbSensorDataBuf.year);
      Robot.file.print(F("-"));
      Robot.file.print(Robot.awbbSensorDataBuf.month);
      Robot.file.print(F("-"));
      Robot.file.print(Robot.awbbSensorDataBuf.day);
      Robot.file.print(F(" "));
      Robot.file.print(Robot.awbbSensorDataBuf.hour);
      Robot.file.print(F(":"));
      Robot.file.print(Robot.awbbSensorDataBuf.minute);
      Robot.file.print(F(":"));
      Robot.file.print(Robot.awbbSensorDataBuf.seconds);
      Robot.file.print(F("."));
      Robot.file.print(Robot.awbbSensorDataBuf.milliseconds);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.fix);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.satellites);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.latitude);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.longitude);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.altitude);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.vLight);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.thermo);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.hygro);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.CO2Density);
      Robot.file.print(F(";"));
      Robot.file.print(Robot.awbbSensorDataBuf.vSound);
      Robot.file.print(F(";"));
      Robot.file.println(Robot.awbbSensorDataBuf.rating);
      
      Robot.file.close();
      
    }

    //----------------------------------------------------------
    // Display some information onto the LCD screen to debug
    //----------------------------------------------------------

#define  SCREEN_PRINT_DATE_TIME 1
#undef SCREEN_PRINT_DATE_TIME
#define SCREEN_PRINT_LAT_LON 1
#undef SCREEN_PRINT_LAT_LON
#define SCREEN_PRINT_MEASURE 1
	//#undef SCREEN_PRINT_MEASURE
#define SCREEN_PRINT_MOVE 1
#undef SCREEN_PRINT_MOVE
    Robot.fillScreen(ILI9163C_BLACK);
//    Robot.setTextSize(1);
    Robot.setCursor(0,0);
    Robot.setTextColor(ILI9163C_BLACK,ILI9163C_RED);
    Robot.println(F("ArduWellBeingBot v2.1"));

    Robot.setTextColor(ILI9163C_WHITE,ILI9163C_BLACK);
#ifdef SCREEN_PRINT_DATE_TIME
   Robot.print(F("Date:"));
   Robot.print(Robot.awbbSensorDataBuf.day);
   Robot.print(F("/"));
   Robot.print(Robot.awbbSensorDataBuf.month);
   Robot.print(F("/"));   
   Robot.println(Robot.awbbSensorDataBuf.year);
   Robot.print(F("Time:"));
   Robot.print(Robot.awbbSensorDataBuf.hour);
   Robot.print(F(":"));
   Robot.print(Robot.awbbSensorDataBuf.minute);
   Robot.print(F(":"));
   Robot.println(Robot.awbbSensorDataBuf.seconds);
#endif
    Robot.print(F("Fix:"));
    Robot.println(Robot.awbbSensorDataBuf.fix);
    Robot.print(F("Sat:"));
    Robot.println(Robot.awbbSensorDataBuf.satellites);
#ifdef SCREEN_PRINT_LAT_LON
    Robot.print(F("Lat:"));
	Robot.println(Robot.awbbSensorDataBuf.latitude);
	Robot.print(F("Lon:"));
	Robot.println(Robot.awbbSensorDataBuf.longitude);
	Robot.print(F("Alt:"));
	Robot.println(Robot.awbbSensorDataBuf.altitude);
#endif
#ifdef SCREEN_PRINT_MEASURE
    Robot.print(F("T/H:"));
    Robot.print(Robot.awbbSensorDataBuf.thermo);
    Robot.print(F("C/"));
    Robot.print(Robot.awbbSensorDataBuf.hygro);
    Robot.println(F("%"));
    Robot.print(F("CO2:"));
    Robot.println(Robot.awbbSensorDataBuf.CO2Density);
    Robot.print(F("Light:"));
    Robot.println(Robot.awbbSensorDataBuf.vLight);
    Robot.print(F("Sound:"));
    Robot.println(Robot.awbbSensorDataBuf.vSound);
#endif
#ifdef SCREEN_PRINT_MOVE
    Robot.print(F("Turn:"));
    Robot.println(deg[imax]);
#endif
    Robot.setTextColor(ILI9163C_GREEN,ILI9163C_BLACK);
//    Robot.setTextSize(3);
    Robot.println();
    Robot.print(F("Rating:"));
    Robot.println(Robot.awbbSensorDataBuf.rating);
    Robot.println();
    Robot.println(Robot.cmd);
#ifdef SCREEN_PRINT_MOTOR
    Robot.print(cptleft);
    Robot.print(" ");
    Robot.println(cptrigth);
    Robot.print(moteurleft);
    Robot.print(" ");
    Robot.println(moteurrigth);
#endif
//    Serial.println("<BC");

}

// Turn with delay instead of using compass
void readandinterpir(int &cptleft, int &cptrigth, int &mill) {
   
  bool valleft = 0;
  bool valrigth = 0;
  static bool lastvalleft = 0;
  static bool lastvalrigth = 0;
  valleft = Robot.digitalRead(pinleft);
  valrigth = Robot.digitalRead(pinrigth);
  if ( valleft == true && lastvalleft == false ) {
       cptleft++;
  }
  if ( valrigth == true && lastvalrigth == false ) {
       cptrigth++;
  }
  lastvalleft = valleft;
  lastvalrigth = valrigth;
}

// Main loop
void loop() {
  // we dont move
  flag = 0;
  // for indice
  uint8_t i ;
#ifdef DEBUGSTEP
  Serial.println("A");
#endif

// Test of front distance
  if ( mode == MODE_ALIVE || mode == MODE_ONLYMEASURE) {
  while( ((distfront = get3Distance()) < 30) // If an obstacle is less than 30cm away 
         || (MOVE == false)        
       )
  {  
#ifdef DEBUGSTEP
  Serial.println("B");
#endif
    if (mode != MODE_ONLYMEASURE ) {
      mode = MODE_MEASURE;
    }
    // stop the motors
#ifndef DEBUGNOMOTOR
    Robot.motorsStop();
#endif
    
    // Collect and Store Data
    CollectAndStoreData();
    
    //----------------------------------------------------------
    // Find the best direction
    //----------------------------------------------------------
    if (mode != MODE_ONLYMEASURE ) {
    distmax = 0;
    imax=-1;
    // Robot.text(distfront,100,0);
    // Try to found longest distance
#ifndef DEBUGNOSERVO
    for (i=0; i< 4 ; i++ ) {
      // Give deg to servo
      servo.write(deg[i]);
      // Wait the servo to go to position
      delay(1000);
      // Get the distance
      dist = getDistance();
      // Found the max
      if ( dist > distmax ) {
        imax = i;
        distmax = dist;
      }
      // Memory of all distance ( not used for the moment)
      disttab[i] = dist;
      // 
 //     Robot.text(dist,5,i*160/4);
#ifdef DEBUGSTEP
        Serial.println("C");
#endif

    }
#endif
#ifdef DEBUGSTEP
  Serial.println("D");
#endif
  //----------------------------------------------------------
  // Turn in the best direction
  //----------------------------------------------------------

#ifndef DEBUGNOMOTOR
    // Turn to the longest distance
    turnwithoutcompass(imax); 
#endif
    // Go to front for the servo
    servo.write(90);
    flag = 1;
    countdist = 0;
    mode = MODE_ALIVE;
    }  
    MOVE = true;
  }// End Select best direction
}
#ifdef DEBUGSTEP
Serial.println("E");
#endif

  //----------------------------------------------------------
  // Let's go in the choosen direction
  //----------------------------------------------------------

  // if we just turn, start progresively
  if (flag == 1 && mode == MODE_ALIVE) {
	for(int i = 5 ; i< 10 ; i+=2 ) {
#ifndef DEBUGNOMOTOR
	  Robot.motorsWrite(i*20, i*20); 
#endif
	  delay(200);
	}
	flag = 0;
#ifndef DEBUGNOMOTOR
	Robot.motorsWrite(moteurleft,moteurrigth);
#endif
  }    
#ifdef DEBUGSTEP
    Serial.println("F");
#endif
/*
  lastmill = mill;
  readandinterpir(cptleft,cptrigth, mill);
  
  if ( abs(cptleft - cptrigth) > 3 ) {
     if (cptleft > cptrigth ) {
       moteurleft--;
       moteurrigth = 255;
     } else {
       moteurleft=255;
       moteurrigth--;
     }
  }
  */
  // Run
  // Delay of 100
  delay(100);

#ifdef DEBUGSTEP
        Serial.println("G");
#endif
#if 1
      if ( Robot.readCmd(Robot.cmd) != 0 ) {
        switch (Robot.cmd[0]) {
          case COMMAND_EXT_ALIVE  : 
            mode = MODE_ALIVE;
            flag = 1; //restart motor
            break;
          case COMMAND_EXT_STANDBY :
            mode = MODE_STANDBY;
	    Robot.motorsWrite(0, 0);
            break;
          case COMMAND_EXT_ONLYMEASURE :
            mode = MODE_ONLYMEASURE;
            MOVE = false;
	    Robot.motorsWrite(0, 0);
            break;
          case COMMAND_EXT_GETDATA :
            Robot.println("GET DATA");
	    Robot.motorsWrite(0, 0);
            Robot.sendCmdData();
            break;
          case COMMAND_EXT_COUNTFILE :
            Robot.processFileSize();
            break;
          case COMMAND_EXT_INIT :
            // Do nothing this command is proceed by motor;
            break;
          case COMMAND_EXT_REINITFILE :
            Robot.reInitFile();
            break;
          default : 
            break;
        }
      }
#endif
#ifdef DEBUGSTEP
     Serial.println("H");
#endif

  countdist++;
  if ( countdist*100 % MAX_RUNNING_TIME == 0 ) {
	MOVE = false;
  }
} //End loop()

// Try to get best distance measure getting 3 dist and take the best.
float get3Distance() {
  int tab[3];
  // read the value from the sensor
  for(int i = 0 ; i< 3 ; i++ ) { 
    tab[i] = getDistance();
  }
  float val ;
  // Choose the minimum diff
  float diff1 = fabs(tab[0]-tab[1]);
  float diff2 = fabs(tab[1]-tab[2]);
  float diff3 = fabs(tab[0]-tab[2]);
  if (diff1 < diff2 ) {
     if ( diff1 < diff3 ) {
       val = tab[0];
     } else {
       val = tab[2];
     }
   } else {
     if ( diff1 < diff3 ) {
       val = tab[1];
     } else {
       val = tab[2];
     }
   }
  return val;
}

// return the distance in cm
float getDistance() {
  // read the value from the sensor
  int sensorValue = Robot.analogRead(sensorPin);
  //Convert the sensor input to cm.
  float distance_cm = sensorValue*1.27;
//  Serial.println(distance_cm);
  return distance_cm;  
}

// Turn with delay instead of using compass
void turnwithoutcompass(int ind) {
  uint8_t speed=87;
  int val = turn[ind]*10;
  if (  turn[ind] < 0 ) {
    Robot.motorsWrite(speed,-speed);//right
  } else {
    Robot.motorsWrite(-speed,speed);//left
  }
  val = abs(val);
  delay(val);
  Robot.motorsWrite(0,0);//right
}


