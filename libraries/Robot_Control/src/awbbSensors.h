#ifndef awbbSensors_h
#define awbbSensors_h

//A data structure for storing all sensor information
typedef struct SENSOR_DATA{
	uint8_t fix;
	uint8_t satellites;
	uint8_t hour;
	uint8_t minute;
	uint8_t seconds;
	uint16_t milliseconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
    float   latitude;
    float   longitude;
    float   altitude;
    float   speed;
    float   angle;
    int vLight;
    float vSound;
    int CO2Density;
    float thermo;
    float hygro;
	uint8_t rating;
    
} awbbSensorData ;

#endif
