#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

void buzzer_init()
{
    /*
      Speaker is connected to P2.6
      TimerA output TA0.1 drives the buzzer.
    */

    timerAUpmode();

    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL  &= ~BIT7;
    P2SEL  |= BIT6;

    P2DIR |= BIT6;
}

void buzzer_set_period(short cycles)
{
    CCR0 = cycles;
    CCR1 = cycles >> 1;      /* 50% duty cycle */
}
