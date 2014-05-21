#ifndef Compass_h
#define Compass_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Class dedicating to manage Dicital Compass HMC6352
// Datasheet: http://aerospace.honeywell.com/~/media/UWSAero/common/documents/myaerospacecatalog-documents/Missiles-Munitions/HMC6352.pdf

#define HMC6352SlaveAddress 0x21
//0x21==0x42>>1, from bildr's code
#define HMC6352ReadAddress 0x41
#define HMC6352EnterCalibrationAddress 0x43
#define HMC6352ExitCalibrationAddress 0x45

class Compass{
	public:
		void begin();                    
		float getReading();            // Read angle
		void enterCalibrationMode();   // Enter Calibration mode (need to turn minimum twice in 20 sec)
		void exitCalibrationMode();    // Exit Calibration mode
	private:
		void _beginTransmission();
		void _endTransmission();

};

#endif