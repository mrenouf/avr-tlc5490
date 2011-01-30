#include <stdint.h>
#include <ctype.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdarg.h>
#include "port_macros.h"
#include "timers.h"
#include "lfsr.h"
#include "gamma.h"


#define SPI_SS     B,0
#define SPI_SCK    B,1
#define SPI_MOSI   B,2
#define SPI_MISO   B,3

#define GSCLK      B,7		// OC0A: Pin for Timer0 Output Compare unit A
#define BLANK      C,0		// General purpose output
#define XLAT       C,1		// General purpose output

// Row address output
#define ROW_MASK   ~(0x07 << ROW_BIT)
#define ROW_PORT   PORTD
#define ROW_BIT    2		// PORTD2

#define nop() asm volatile("nop"::)

typedef volatile uint8_t (*matrix_buf)[8][16];

void spi_xmit_pwm(void);
void spi_send_next(void);
static volatile uint32_t speed = F_CPU;

// Reserve storage for two framebuffers
static volatile uint8_t buffer1[8][16];
static volatile uint8_t buffer2[8][16];

// Pointers to the visible and hidden buffers
// These are swapped to perform double buffering
static volatile matrix_buf visible = &buffer1;
static volatile matrix_buf hidden = &buffer2;

static volatile uint8_t row = 0;		// indicates the row being transmitted
static volatile int8_t channel = 15;	// the current channel value being transmitted
static volatile int8_t bitnum = 11;     // the next bit to be sent of the current channel
static volatile uint8_t vsync = 0; 		// flag indicating it's safe to page flip


#define GSCLK_SCALE 		8	// GSCLK = F_CPU / GSCLK_SCALE
#define CYCLE_LENGTH 		4096
#define CYCLE_COUNT			((uint16_t) CYCLE_LENGTH * GSCLK_SCALE)

/**
 * A single 8 bit timer is used to generate a clock signal. The timer is
 * configured in CTC (Clear Timer on Compare) mode. The output compare pin
 * is also configured to toggle it's value on compare match. This creates
 * a clock output on OCR0 with a period of F_CPU / (OCR0A + 1)
 */
void init_gsclk() {
	OUTPUT(GSCLK);
	OCR0A = (GSCLK_SCALE / 2) - 1;
	TCCR0A &= ~(_BV(COM0A1) | _BV(COM0A0) | _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01) | _BV(WGM00));
	TCCR0A |= _BV(WGM01) | _BV(COM0A0);
	TCCR0B &= ~(_BV(FOC0A) | _BV(FOC0B) | _BV(WGM02) | _BV(CS02) | _BV(CS01) | _BV(CS00));
	TCCR0B |= _BV(CS00);
}

/**
 * PWM cycle timer
 */
void init_cycle() {
	OCR1A = CYCLE_COUNT - 1;
	TIMSK1 |= _BV(OCIE1A);
	TCCR1A &= ~(_BV(COM1A1) | _BV(COM1A0) | _BV(WGM11) | _BV(WGM10));
	TCCR1A |= _BV(COM1A0);
	TCCR1B &= ~(_BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS11) | _BV(CS10));
	TCCR1B |= _BV(WGM12) | _BV(CS10);
}

void init_spi() {
	OUTPUT(SPI_SS);
	OUTPUT(SPI_MOSI);
	OUTPUT(SPI_SCK);
	INPUT(SPI_MISO);
	SET(SPI_SS);

	// Enable, Interrupt on complete, MSB first, master mode,
	// positive clock polarity, sample on leading edge,
	// SCK frequency F_CPU/4
	SPCR = _BV(SPIE) | _BV(SPE) | _BV(MSTR);
	SPSR &= ~_BV(SPI2X);
	// F_CPU/2
	////SPSR |= _BV(SPI2X);
}

void init_row_addr() {
	DDRD |= (0x07 << 2);				// PORTC[2:4] is a 3-bit row address
}

static inline void gsclk_stop() {
	TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
}

static inline void gsclk_start() {
	TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
	TCNT0 = 0;
	TCCR0B |= _BV(CS00);
}

static inline void row_activate() {
	PORTD = ((ROW_PORT & ROW_MASK) | (row << ROW_BIT));
}

static inline void row_step() {
	row = (row + 1) % 8;
}

static inline void cycle_stop() {
	TCCR1B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
}

static inline void cycle_start() {
	TCCR1B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
	TCNT1 = 0xFFFF;
	TCCR1B |= _BV(CS00);
}

// This interrupt marks the end of each PWM cycle
ISR (TIMER1_COMPA_vect) {
	cycle_stop();
	SET(BLANK);
	row_activate(row); 		// bring the active row high

	SET(XLAT);				// pulse XLAT to latch the new data
	_delay_us(10);
	CLEAR(XLAT);
	cycle_start();			// start timimg the pwm cycle (up to 4095)
	CLEAR(BLANK);

	//row_step();	 			// point to the next row to be sent
	channel = 15;			// reset state for next row
	spi_xmit_pwm(); 		// trigger xmit of next cycle's data
}


// Xmit 192 bits (16 ch x 12 bits/ch) out to the 5490's SIN port
// The data is MSB first, starting with the last channel
// Only the lower 12 bits of each pwm value are sent (0-4095)
void spi_xmit_pwm() {
	vsync = 0;                  // clear sync flag so protect visible buffer
	CLEAR(SPI_SS);				// bring SS low to signal the next packet
	spi_send_next();
}

ISR (SPI_STC_vect) {
	spi_send_next();
}

// This method is repeatedly called by the SPI_STC interrupt
// handler until all the data for the next PWM cycle has been
// sent.
void spi_send_next() {
	static uint16_t pwm;
	uint8_t value = 0;
	uint8_t a, b;
	if (channel >= 0) {
		switch (bitnum) {
		case 3:
			a = buffer1[row][channel--];
			b = buffer1[row][channel];

			// use the last 4 bits of previous, first 4 of next
			value = (uint8_t) ((gamma_lut[a] << 4) & 0xF0);
			value |= (uint8_t) ((gamma_lut[b] >> 8) & 0x0F);
			break;
		case 11:
			pwm = gamma_lut[buffer1[row][channel]];
		case 7:
			// byte consist of upper or lower 8 bits of channel
			// bitpos is either 11 or 7 here
			value = (uint8_t) ((pwm >> (bitnum - 7)) & 0xFF);
			break;
		}

		// sequence: 11, 3, 7
		bitnum = (bitnum + 4) % 12;

		if (bitnum == 11)
			--channel;

		// write the next 8 bits to SPI
		SPDR = value;
	}
	else {
		SET(SPI_SS);			// bring SS high, marking the the end of the packet

		if (row == 7) {			// check if all rows have been sent
			//swap_buffer();	// flip buffer for next frame
//			uint8_t j;
//			for (j=0; j < 16; j++) {
//				buffer1[0][j] = lfsr_16(rand);
//			}
		}
		uint8_t i;
		for (i=0; i<16; i++) {
			buffer1[0][i] += (1+i);
		}
	}
}

// waits for the current pwm cycle to complete which is triggered
// by the vsync flag being set after all of the data from the
// visible buffer as been transmitted for the next frame
// the two buffer pointers are swapped so the main loop
// can continue writing to the hidden buffer while the visible buffer
// is transmitted for following cycle
void wait_frame() {
    cli();
    while (!vsync) {					// has the current frame xmit completed?
        sei();
	    sleep_cpu();					// wait for next interrupt
	}
	cli();
//	matrix_buf tmp = visible; 			// pointer swap
//	visible = hidden;
//	hidden = tmp;
	vsync = 0;
}

#define STATUS A

int main(void) {
	uint8_t i;
	for (i=0; i<16; i++) {
		buffer1[0][i] = 0x10 + (i*0xF);
	}
	OUTPUT(BLANK);
	OUTPUT(XLAT);


//	uint8_t i = 0;
//	uint8_t j = 0;
//	for (i=0; i < 8; i++) {
//		for (j=0; j < 16; j++) {
//			buffer1[i][j] = (i*8*4) + (j*2);
//		}
//	}

	init_spi();
	init_row_addr();
	init_gsclk();
	init_cycle();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
	sei();
	for (;;) {
		sleep_cpu();
	}
}
