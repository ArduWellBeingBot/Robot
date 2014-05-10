/* 

 envRating

 Evaluate a global note based on treshold and weight for each value  

 created 19 February 2014
 by F. Brodziak & P. Locquet
 
 This code is in the public domain
 
 */

#ifndef envRating_h
#define envRating_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Temp treshold
#define T_MAXMAX   32.0
#define T_MAX      26.0
#define T_MIN      19.0
#define T_MINMIN   14.0
#define T_WEIGHT   1
// Hygro treshold
#define H_MAXMAX   90.0
#define H_MAX      70.0
#define H_MIN      40.0
#define H_MINMIN   20.0
#define H_WEIGHT   1
// CO2 treshold
#define C_MAXMAX   1000
#define C_MAX      400
#define C_WEIGHT   2
// Light treshold
#define L_MAX      1000.0
#define L_MIN      600.0
#define L_MINMIN   250.0
#define L_WEIGHT   1
// Sound treshold
#define S_MAXMAX   3.8
#define S_MAX      3.0
#define S_WEIGHT   2

class EnvRating {

public:
  static const uint8_t RATE_GOOD = 2;
  static const uint8_t RATE_MID  = 1;
  static const uint8_t RATE_BAD  = 0;

  EnvRating(){
    reset();
  };
  
  void reset(){
    _rating = 0.0;
    _weight = 0.0;
  };
  
  void addTemp(float t){
    addValue(t, T_MINMIN, T_MIN, T_MAX, T_MAXMAX, T_WEIGHT);
  };
  
  void addHygro(float h){
    addValue(h, H_MINMIN, H_MIN, H_MAX, H_MAXMAX, H_WEIGHT);
  };
  
  void addCO2(float c){
    addValue(c, C_MAX, C_MAXMAX, C_WEIGHT);
  };

  void addSound(float s){
    addValue(s, S_MAX, S_MAXMAX, S_WEIGHT);
  };
  
  void addLight(float l){
    if (l<=L_MINMIN) {
    _rating += RATE_BAD*L_WEIGHT;
    } else if (((l<=L_MIN) && (l>L_MINMIN)) || (l>=L_MAX) ){
    _rating += RATE_MID*L_WEIGHT;
    } else if ((l<L_MAX) || (l>L_MIN)){
    _rating += RATE_GOOD*L_WEIGHT;
    }
    _weight += L_WEIGHT;  
  };
  
  uint8_t getRating(){
    if (_weight == 0)
      return 0;  
    else
      return (_rating/_weight)*10;
    };
    
protected:
  float   _weight;
  float   _rating;
  
    void addValue(float v, float t_max, float t_maxmax, float v_weight){
      if (v<=t_max){
      _rating += RATE_GOOD*v_weight;
      } else if ((v<=t_maxmax) && (v>t_max)){
      _rating += RATE_MID*v_weight;
      } else if (v>t_maxmax){
      _rating += RATE_BAD*v_weight;
      }
      _weight += v_weight;
  };
  
    void addValue(float v, float t_minmin, float t_min, float t_max, float t_maxmax, float v_weight){
      if ((v<=t_minmin) || (v>=t_maxmax)){
      _rating += RATE_BAD*v_weight;
      } else if (((v<=t_min) && (v>t_minmin)) || ((v<t_maxmax) && (v>=t_max)) ){
      _rating += RATE_MID*v_weight;
      } else if ((v<t_max) || (v>t_min)){
      _rating += RATE_GOOD*v_weight;
      }
      _weight += v_weight;
  };

  
  };




#endif

