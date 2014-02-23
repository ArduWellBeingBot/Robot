/* Motor Core

  This code is based on the main arduino control board with reading gps param.
  
*/

#include <ArduinoRobotMotorBoard.h>

void setup(){
  Serial.begin(9600);
  delay(2000);
  RobotMotor.begin();
  RobotMotor.beginGps();

}
void loop(){
  RobotMotor.parseCommand();
  RobotMotor.process();
  RobotMotor.processGps();
}
