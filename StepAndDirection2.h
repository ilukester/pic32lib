#ifndef STEPANDDIRECTION2_H
#define STEPANDDIRECTION2_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"

typedef bool us1;	// for some reason typedef for bool or boolean do not work

const uint32_t delay_5us   = CORE_TICK_RATE/1000*5;
const uint32_t delay_25us  = CORE_TICK_RATE/1000*25;
const uint32_t delay_50us  = CORE_TICK_RATE/1000*50;
const uint32_t delay_125us = CORE_TICK_RATE/1000*125;
const uint32_t delay_250us = CORE_TICK_RATE/1000*250;
const uint32_t delay_1ms   = CORE_TICK_RATE/1000*1000;
const uint32_t delay_250ms = CORE_TICK_RATE/1000*250000;

class StepAndDirection2 {
public:
    typedef struct {
        us1 direction;
        int steps;
        int skip;
        int currentSkip;
    } Vector;

    StepAndDirection2(us8 pin_step, us8 pin_direction, us8 pin_enable, us8 pin_sleep) {
		StepAndDirection2::pin_step = pin_step;
		StepAndDirection2::pin_direction = pin_direction;
		StepAndDirection2::pin_enable = pin_enable;
		StepAndDirection2::pin_sleep = pin_sleep;
		
		pinMode(pin_step,OUTPUT);
		pinMode(pin_direction,OUTPUT);
		pinMode(pin_enable,OUTPUT);
		pinMode(pin_sleep,OUTPUT);
		digitalWrite(pin_sleep,HIGH);
		
        interruptPeriod = delay_250ms;
    }

    uint32_t interrupt(uint32_t currentTime) {
        if(vector.steps > 0) {
            if(vector.currentSkip > 0) {
                vector.currentSkip--;
            }
            else {
                vector.currentSkip = vector.skip;

                // step
                us1 temp = digitalRead(pin_step);
                if(temp) {
                    vector.steps--;
                }
                digitalWrite(pin_step, !temp);
            }
        }

        return (currentTime + interruptPeriod);
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("stp")) {
            parser.nextToken();

            if(parser.compare("vector")) {
                vector.direction = 0;
                vector.currentSkip = 0;

                parser.nextToken();
                vector.steps = parser.toVariant().toInt();

                parser.nextToken();
                vector.skip = parser.toVariant().toInt();

                Serial.println("OK");
            }
        }
    }

private:
	us8 pin_step;
	us8 pin_direction;
	us8 pin_enable;
	us8 pin_sleep;

    Variant timeBase;
    uint32_t interruptPeriod;
    Vector vector;
};

#endif // STEPANDDIRECTION2_H