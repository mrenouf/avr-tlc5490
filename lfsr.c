#include "lfsr.h"


uint16_t lfsr_16(uint16_t in) {
	return (in >> 1) ^ (-(in & 1u) & 0xB400u);
}

uint32_t lfsr_32(uint32_t in) {
	return (in >> 1) ^ (-(in & 1u) & 0x80200003u);
}
