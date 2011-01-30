/*
 * lfsr.c
 *
 * Linear-feedback shift registers for fast pseudo-random number generation.
 *
 * Based on the application note:
 * "Efficient Shift Registers, LFSR Counters, and Long Pseudo-Random Sequence Generators"
 * URL: http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
 */

#ifndef LFSR_H_
#define LFSR_H_

#include <stdint.h>

/**
 * A 16-bit linear feedback shift register with taps at 16, 14, 13, and 11
 * producing a pseudo-random sequence 65,535 integers.
 */
uint16_t lfsr_16(uint16_t in);

/**
 * A 32-bit linear feedback shift register with taps at 32, 22, 2, and 1
 * producing a pseudo-random sequence of 4,294,967,294 integers.
 */
inline uint32_t lfsr_32(uint32_t in);


#endif /* LFSR_H_ */



