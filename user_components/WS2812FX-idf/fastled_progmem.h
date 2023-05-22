#ifndef __INC_FL_PROGMEM_H
#define __INC_FL_PROGMEM_H

//#include "FastLED.h"

///@file fastled_progmem.h
/// wrapper definitions to allow seamless use of PROGMEM in environmens that have it

//FASTLED_NAMESPACE_BEGIN

// Compatibility layer for devices that do or don't
// have "PROGMEM" and the associated pgm_ accessors.
//
// If a platform supports PROGMEM, it should define
// "FASTLED_USE_PROGMEM" as 1, otherwise FastLED will
// fall back to NOT using PROGMEM.
//
// Whether or not pgmspace.h is #included is separately
// controllable by FASTLED_INCLUDE_PGMSPACE, if needed.


// If FASTLED_USE_PROGMEM is 1, we'll map FL_PROGMEM
// and the FL_PGM_* accessors to the Arduino equivalents.

// If FASTLED_USE_PROGMEM is 0 or undefined,
// we'll use regular memory (RAM) access.

//empty PROGMEM simulation
#define FL_PROGMEM
#define FL_PGM_READ_BYTE_NEAR(x)  (*((const  uint8_t*)(x)))
#define FL_PGM_READ_WORD_NEAR(x)  (*((const uint16_t*)(x)))
#define FL_PGM_READ_DWORD_NEAR(x) (*((const uint32_t*)(x)))


// On some platforms, most notably ARM M0, unaligned access
// to 'PROGMEM' for multibyte values (eg read dword) is
// not allowed and causes a crash.  This macro can help
// force 4-byte alignment as needed.  The FastLED gradient
// palette code uses 'read dword', and now uses this macro
// to make sure that gradient palettes are 4-byte aligned.
#define FL_ALIGN_PROGMEM  __attribute__ ((aligned (4)))

//FASTLED_NAMESPACE_END

#endif
