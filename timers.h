/*
 * timers.h
 *
 * Some useful definitions for working with timers on the ATMega-162 (and probably others)
 */

#ifndef TIMERS_H_
#define TIMERS_H_

// Definitions for Timer/Counter0 clock settings
#define TIMER0_CLK_MASK = ~((1 << CS00) | (1 << CS01) | (1 << CS02))
#define TIMER0_CLK_STOPPED 0
#define TIMER0_CLK_SCALE_1 (1 << CS00)
#define TIMER0_CLK_SCALE_8 (1 << CS01)
#define TIMER0_CLK_SCALE_64 ((1 << CS01) | (1 << CS00))
#define TIMER0_CLK_SCALE_256 (1 << CS02)
#define TIMER0_CLK_SCALE_1024 ((1 << CS02) | (1 << CS00))
#define TIMERO_CLK_EXT_FALLING ((1 << CS02) | (1 << CS01))
#define TIMER0_CLK_EXT_RISING ((1 << CS02) | (1 << CS01) | (1 << CS00))

// Definitions for Timer/Counter1 clock settings
#define TIMER1_CLK_MASK = ~((1 << CS10) | (1 << CS11) | (1 << CS12))
#define TIMER1_CLK_STOPPED 0
#define TIMER1_CLK_SCALE_1 (1 << CS10)
#define TIMER1_CLK_SCALE_8 (1 << CS11)
#define TIMER1_CLK_SCALE_64 ((1 << CS11) | (1 << CS10))
#define TIMER1_CLK_SCALE_256 (1 << CS12)
#define TIMER1_CLK_SCALE_1024 ((1 << CS12) | (1 << CS10))
#define TIMER1_CLK_EXT_FALLING ((1 << CS12) | (1 << CS11))
#define TIMER1_CLK_EXT_RISING ((1 << CS12) | (1 << CS11) | (1 << CS10))

// Definitions for Timer/Counter2 clock settings
#define TIMER2_CLK_MASK = ~((1 << CS22) | (1 << CS21) | (1 << CS20))
#define TIMER2_CLK_STOPPED 0
#define TIMER2_CLK_SCALE_1 (1 << CS20)
#define TIMER2_CLK_SCALE_8 (1 << CS21)
#define TIMER2_CLK_SCALE_32 ((1 << CS21) | (1 << CS20))
#define TIMER2_CLK_SCALE_64 (1 << CS22)
#define TIMER2_CLK_SCALE_128 ((1 << CS22) | (1 << CS20))
#define TIMER2_CLK_SCALE_256 ((1 << CS22) | (1 << CS21))
#define TIMER2_CLK_SCALE_1024 ((1 << CS22) | (1 << CS21) | (1 << CS20))

// Definitions for Timer/Counter3 clock settings
#define TIMER3_CLK_MASK = ~((1 << CS32) | (1 << CS31) | (1 << CS30))
#define TIMER3_CLK_STOPPED 0
#define TIMER3_CLK_SCALE_1 (1 << CS30)
#define TIMER3_CLK_SCALE_8 (1 << CS31)
#define TIMER3_CLK_SCALE_64 ((1 << CS31) | (1 << CS30))
#define TIMER3_CLK_SCALE_256 (1 << CS32)
#define TIMER3_CLK_SCALE_1024 ((1 << CS32) | (1 << CS30))
#define TIMER3_CLK_SCALE_16 ((1 << CS32) | (1 << CS31))
#define TIMER3_CLK_SCALE_32 ((1 << CS32) | (1 << CS31) | (1 << CS30))


#endif /* TIMERS_H_ */
