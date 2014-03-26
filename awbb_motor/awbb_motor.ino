/* Motor Core

  This code is based on the main arduino control board with reading gps param.
  
*/

#include <ArduinoRobotMotorBoard.h>

void setup(){
  Serial.begin(19200);
  delay(1000);
  RobotMotor.begin();
  RobotMotor.beginGps();
}
void loop(){
  RobotMotor.parseCommand();
  RobotMotor.process();
}
