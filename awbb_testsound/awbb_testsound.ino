#include <MedianFilter.h>
#define LOG_OUT 1 // use the log output function
#define FHT_N 32 // set to 256 point fht
#include <FHT.h>
#include "SoundSensor1.h"
// Pin for sound sensor
#define SOUND_PIN TK1

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
 double val = readSoundSensor1(SOUND_PIN);
 delay(100);
}
