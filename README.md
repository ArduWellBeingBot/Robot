ArduWellBeingBot
================

More details on the dedicated blog: http://arduwellbeingbot.blogspot.fr


Download the sources and copy them into your Sketchbook directory.

A patch needs to be realized onto your Arduino hardware directories:

You have to add these 4 definition (digitalPinToPCICR, digitalPinToPCICRbit, digitalPinToPCMSK, digitalPinToPCMSKbit(p) ) into the pin_arduino.h file in these following directories:

...\Arduino\hardware\arduino\avr\variants\robot_control 

...\Arduino\hardware\arduino\avr\variants\robot_motor


    #define digitalPinToPCICR(p) ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCICR) : ((uint8_t *)0))
    #define digitalPinToPCICRbit(p) 0
    #define digitalPinToPCMSK(p) ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17) || ((p) >= A8 && (p) <= A10)) ? (&PCMSK0) : ((uint8_t *)0))
    #define digitalPinToPCMSKbit(p) ( ((p) >= 8 && (p) <= 11) ? (p) - 4 : ((p) == 14 ? 3 : ((p) == 15 ? 1 : ((p) == 16 ? 2 : ((p) == 17 ? 0 : (p - A8 + 4))))))
     
    // __AVR_ATmega32U4__ has an unusual mapping of pins to channels
    extern const uint8_t PROGMEM analog_pin_to_channel_PGM[];
    #define analogPinToChannel(P) ( pgm_read_byte( analog_pin_to_channel_PGM + (P) ) )
