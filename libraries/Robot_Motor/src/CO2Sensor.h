/*
 * CO2Sensor: The aim of this object is to get the quantity of CO2 in ppm in the air
 * 
 * Version : 0.1  February-2014
 * Copyright 2014 AWBB team
 * Based on sample code of DFRobot http://www.dfrobot.com/wiki/index.php/CO2_Sensor_SKU:SEN0159
 *
 */

//------------------------------------------------------------------------------------------------
// Calibrating CO2 Sensor using outside air
//------------------------------------------------------------------------------------------------
// For most applications, outside air is an accurate and effective calibration source. Outdoor
// carbon dioxide is generally a well-mixed atmospheric gas. That is, it typically remains at a
// relatively constant slowly-changing level. Currently, this level is near 390 ppm.
// Although carbon dioxide is generally well-mixed in the atmosphere, in locations near carbon
// dioxide sources (such as running vehicles, factories, humans, etc.) carbon dioxide can vary.
// For rural areas and small towns, this variability it rarely more than 20 ppm above the well-
// mixed level of 390 ppm. For metropolitan areas, the daily maximum can be 80 ppm or more
// above this level. Also, because of weather and climate effects, carbon dioxide is generally
// better mixed in mid-afternoon than in the morning. For these reasons, as a general rule,
// calibrate your sensor in the mid-afternoon.
// To calibrate using outside air, take your sensor to an outside area away from running vehicles,
// people, animals, and other carbon dioxide sources. Let the sensor carbon dioxide value settle
// for about 20 minutes (TBC). (Since human breathing creates a significant amount of carbon dioxide,
// for best results leave the sensor during this time.)


#ifndef CO2Sensor_h
#define CO2Sensor_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/************************Hardware Related Constants************************************/
#define   DC_GAIN              (8.5)   //define the DC gain of amplifier


/***********************Software Related Constants************************************/
#define   READ_SAMPLE_INTERVAL (50)    //define how many samples you are going to take in normal operation
#define   READ_SAMPLE_TIMES    (5)     //define the time interval(in milisecond) between each samples in 
//normal operation

/**********************Application Related Constants**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define   ZERO_POINT_VOLTAGE   (0.324) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define   REACTION_VOLTAGE     (0.020) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

//two points are taken from the curve. 
//with these two points, a line is formed which is
//"approximately equivalent" to the original curve.
//data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280) 
//slope = ( reaction voltage ) / (log400 –log1000) 
float           CO2Curve[3]  =  {
  2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTAGE/(2.602-3))};   

//----------------------------------------------------------------------------
// Class definition
//----------------------------------------------------------------------------
class CO2Sensor {
protected:
  uint8_t _pin;
  float   _volts;

public:

  CO2Sensor(uint8_t pin) {
    _pin = pin;
    _volts = 0;
  };

  //----------------------------------------------------------------------------
  // Description : read the output value of DfRobot SEN0159 (CO2 Sensor module)
  //               in the delay of READ_SAMPLE_TIMES x READ_SAMPLE_INTERVAL
  //               An average value is processed during this period
  // Output      : output voltage
  //----------------------------------------------------------------------------    
  float read(){
    uint8_t i;
    _volts = 0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
      _volts += analogRead(_pin);
      delay(READ_SAMPLE_INTERVAL);
    }

    _volts = (_volts/READ_SAMPLE_TIMES) *5/1024 ;
//    Serial.print("CO2v:");
//    Serial.println(_volts);
    return _volts;
  }


  //----------------------------------------------------------------------------
  // Description : voltage getter 'read' method must be called before
  // Output      : output voltage
  //----------------------------------------------------------------------------    
  float getVolts(){ 
    return _volts;
  }

  //----------------------------------------------------------------------------
  // Description : get the CO2 quantity in ppm
  //               By using the slope and a point of the line. The x(logarithmic value of ppm) 
  //               of the line could be derived if y(MG-811 output) is provided. As it is a 
  //               logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic 
  //               value.
  //              'read' method must called before
  // Output      : ppm of the target gas
  //----------------------------------------------------------------------------    
  int getPPM(){
    if ((_volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
    } 
    else { 
//      Serial.print("PPM:");
//      Serial.println(int(pow(10, ((_volts/DC_GAIN)-CO2Curve[1])/CO2Curve[2]+CO2Curve[0])));
      return pow(10, ((_volts/DC_GAIN)-CO2Curve[1])/CO2Curve[2]+CO2Curve[0]);
    }
  }
};

#endif


