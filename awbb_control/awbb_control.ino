/* awbb control based on runaway_robot

Moving the arduino robot (based on code of sample runaway_robot.)
When stop or each MAX_RUNNING_TIME ms take measure of :
gps data, temperature, hydrometrie, co2, sound, ligth
  
 Circuit:
 * Arduino Robot
 * US range finder Maxbotix EZ10, with analog output on TK2/M2
 * Servo on TKD4 (the only pwm)
 * LDR (with pull-up resistor) on TK4/M4
 
 Modified code of robot_motor to support : CO2, gps, temperature and hudrometrie captor, sound level
  
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

// To manage the sensor
#include "EnvRating.h"

// time in ms between each measure. 
// Robot will stop, take data and select a new direction
#define MAX_RUNNING_TIME 10000

// PIN
uint8_t sensorPin      = TK2;  // M2 : pin of the range sensor
uint8_t servoPin       = TKD4;  // pin of the servo
// Note the servo is front of the robot when angle is 90 Degres.
uint8_t lightSensorPin = TK4;    // M4 : pin of the LDR

// Servo object
Servo servo;

// To manage data into the µSDCard
SdCard card;
Fat16 file;
#define CSV_SEPARATOR F(";")
#define LOG_FILENAME "AWBBlog.txt"

// Contains all GPS information (get from motor_board)
GPS_DATA gpsInfo;

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
unsigned char deg[4] = {
  170,
  135,
  45,
  10
  };
// angle to turn by indice
int turn[4] = {
  -90,
  -45,
  45,
  90
  };
// Counter of distance
int countdist;

// Setup function
void setup() {
  // initialize the Robot, SD card, and display
  Serial.begin(9600);
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
  card.init(0,CARD_CS);
  // return false if initialization failed
  file.init(&card);
  // Activate moving
  MOVE = true;  
  // GPS data struct initilization
  memset ((void*)&gpsInfo,0,sizeof(gpsInfo));
}// End Setup

// Collect and Store data
void CollectAndStoreData() {
    //----------------------------------------------------------
    // Collect all the data from sensors
    //----------------------------------------------------------
    // Get TimeStamp
    Robot.readGPSTimeDate(gpsInfo);
    
    // Get GPS Lat/Long
    Robot.readGPSCoord(gpsInfo);

    //Launch all the analyzes
    EnvRating rating;

    // - temp/hygro
    float t = -9999.9;
    float h = -9999.9;
    Robot.readTH(t,h);
    rating.addTemp(t);
    rating.addHygro(h);

    // - CO2
    int CO2Density = -9999;
    Robot.readCO2Sensor(CO2Density);
    rating.addCO2(CO2Density);
        
    // - light
    float vLight = 0.0;
    vLight = Robot.analogRead(lightSensorPin);
    rating.addLight(vLight);

    // - Sound (take some time to analyze)
    float vSound = -9999.9;
    Robot.readSoundLevel(vSound);
    rating.addSound(vSound);

    //----------------------------------------------------------
    // Save the collected data and the environment rating
    //----------------------------------------------------------

    // Save the data into µSD
    if (file.open(LOG_FILENAME, O_RDWR | O_CREAT | O_APPEND)) {
      Serial.println("Writing...");
      // Write a line
      //« YYYY-MM-DDTHH :MM :SS.mmm ;FIX;SAT;LAT ;LONG ;ALT ;LUMIERE ;TEMP ;HYGRO ;CO2 ;MICRO ;NOTE »
      file.print(F("20"));
      file.print(gpsInfo.year);
      file.print(F("-"));
      file.print(gpsInfo.month);
      file.print(F("-"));
      file.print(gpsInfo.day);
      file.print(F(" "));
      file.print(gpsInfo.hour);
      file.print(F(":"));
      file.print(gpsInfo.minute);
      file.print(F(":"));
      file.print(gpsInfo.seconds);
      file.print(F("."));
      file.print(gpsInfo.milliseconds);
      file.print(F(";"));
      file.print(gpsInfo.lat);
      file.print(F(";"));
      file.print(gpsInfo.lon);
      file.print(F(";"));
      file.print(gpsInfo.alt);
      file.print(F(";"));
      file.print(vLight);
      file.print(F(";"));
      file.print(t);
      file.print(F(";"));
      file.print(h);
      file.print(F(";"));
      file.print(CO2Density);
      file.print(F(";"));
      file.print(vSound);
      file.print(F(";"));
      file.println(rating.getRating());
      file.close();
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
    Robot.println(F("ArduWellBeingBot v1.0"));

    Robot.setTextColor(ILI9163C_WHITE,ILI9163C_BLACK);
#ifdef SCREEN_PRINT_DATE_TIME
   Robot.print(F("Date:"));
   Robot.print(gpsInfo.day);
   Robot.print("/");
   Robot.print(gpsInfo.month);
   Robot.print("/");   
   Robot.println(gpsInfo.year);
   Robot.print(F("Time:"));
   Robot.print(gpsInfo.hour);
   Robot.print(":");
   Robot.print(gpsInfo.minute);
   Robot.print(":");
   Robot.println(gpsInfo.seconds);
#endif
    Robot.print(F("Fix:"));
    Robot.println(gpsInfo.fix);
    Robot.print(F("Sat:"));
    Robot.println(gpsInfo.sat);
#ifdef SCREEN_PRINT_LAT_LON
    Robot.print(F("Lat:"));
	Robot.println(gpsInfo.lat);
	Robot.print(F("Lon:"));
	Robot.println(gpsInfo.lon);
	Robot.print(F("Alt:"));
	Robot.println(gpsInfo.alt);
#endif
#ifdef SCREEN_PRINT_MEASURE
    Robot.print(F("T/H:"));
    Robot.print(t);
    Robot.print("C/");
    Robot.print(h);
    Robot.println("%");
    Robot.print(F("CO2:"));
    Robot.println(CO2Density);
    Robot.print(F("Light:"));
    Robot.println(vLight);
    Robot.print(F("Sound:"));
    Robot.println(vSound);
#endif
#ifdef SCREEN_PRINT_MOVE
    Robot.print(F("Turn:"));
    Robot.println(deg[imax]);
#endif
    Robot.setTextColor(ILI9163C_GREEN,ILI9163C_BLACK);
//    Robot.setTextSize(3);
    Robot.println();
    Robot.print(F("Rating:"));
    Robot.println(rating.getRating());
}

// Main loop
void loop() {
  // we dont move
  flag = 0;
  // for indice
  int i ;
  
  // Test of front distance
  while( ((distfront = get3Distance()) < 30) // If an obstacle is less than 30cm away 
         || (MOVE == false)                  // Or if we are connected to Serial
       )
  {  
    // stop the motors
    Robot.motorsStop();
    // Collect and Store Data
	CollectAndStoreData();

    //----------------------------------------------------------
    // Find the best direction
    //----------------------------------------------------------
    distmax = 0;
    imax=-1;
    // Robot.text(distfront,100,0);
    // Try to found longest distance
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
      Robot.text(dist,5,i*160/4);
    }


  //----------------------------------------------------------
  // Turn in the best direction
  //----------------------------------------------------------

    // Turn to the longest distance
    turnwithoutcompass(imax); 
    // Go to front for the servo
    servo.write(90);
    delay(1000); 
    flag = 1;
    countdist = 0;
	MOVE = true;
  }// End Select best direction

  //----------------------------------------------------------
  // Let's go in the choosen direction
  //----------------------------------------------------------

  // if we just turn, start progresively
  if (flag == 1) {
	for(int i = 5 ; i< 10 ; i+=2 ) {
	  Robot.motorsWrite(i*20, i*20); 
	  delay(200);
	}
	flag = 0;
	Robot.motorsWrite(255, 255);
  }    
  // Run
  // Delay of 100 
  delay(100);
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
