/**
 * Macros for clear and concise single pin port manipulation.
 * Credit to: http://www.starlino.com/port_macro.html
 *
 * Usage examples:
 *
 * #define LED_PID    A,1
 * #define SWITCH     C,4
 *
 * OUTPUT(LED_PIN);
 * SET(LED_PIN);
 * CLEAR(LED_PIN);
 * TOGGLE(LED_PID);
 *
 * INPUT(SWITCH);
 * uint8_t sw = READ(SWITCH);
 *
 */
#ifndef _PORT_MACROS_H_
#define _PORT_MACROS_H_ 1

#include <stdint.h>

//these macros are used indirectly by other macros, mainly for string concatination
#define _SET(type,name,bit)           type##name  |= _BV(bit)
#define _CLEAR(type,name,bit)         type##name  &= ~ _BV(bit)
#define _TOGGLE(type,name,bit)        type##name  ^= _BV(bit)
#define _READ(type,name,bit)          ((type##name >> bit) &  1)
#define _WRITE(type,name,bit,value)   type##name = (type##name & ~_BV(bit)) | ((1 & (uint8_t)value) << bit)

//these macros are used by end user
#define OUTPUT(pin)                   _SET(DDR,pin)
#define INPUT(pin)                    _CLEAR(DDR,pin)
#define SET(pin)                      _SET(PORT,pin)
#define CLEAR(pin)                    _CLEAR(PORT,pin)
#define TOGGLE(pin)                   _TOGGLE(PORT,pin)
#define READ(pin)                     _READ(PIN,pin)
#define WRITE(pin,value)              _WRITE(PORT,pin,value)

// (mrenouf): I added these for more efficient whole-port operations
#define _READ_PORT(type,name)         type##name
#define _WRITE_PORT(type,name,value)  type##name = value
#define _CLEAR_PORT(type,name)        type##name = 0x00
#define _INC_PORT(type,name)          ++type##name
#define _DEC_PORT(type,name)          --type##name

#define WRITE_PORT(port,value)       _WRITE_PORT(PORT,port,value)
#define CLEAR_PORT(port)             _CLEAR_PORT(PORT,port)
#define OUTPUT_PORT(port)            _WRITE_PORT(DDR,port,0xFF)
#define INPUT_PORT(port)             _WRITE_PORT(DDR,port,0x00)
#define READ_PORT(port)              _READ_PORT(PORT,port)
#define INC_PORT(port)               _INC_PORT(PORT,port)
#define DEC_PORT(port)               _DEC_PORT(PORT,port)

#endif // _PORT_MACROS_H_
