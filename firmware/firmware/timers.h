#pragma once

#include "sys_globals.h"

// 16bit timer prescaler configuration. See AtMega2560 data sheet table 17-6
#define PRESC1024

#ifdef PRESC8  // max TC1 period is 32ms
const uint8_t presc_shift = 1;
const uint8_t TCCR1B_prescaler_bits = bit(CS11);
#endif // PRESC64

#ifdef PRESC64  // max TC1 period is 262ms
const uint8_t presc_shift = 2;
const uint8_t TCCR1B_prescaler_bits = bit(CS10) | bit(CS11);
#endif // PRESC64

#ifdef PRESC256  // max TC1 period is 1048ms
const uint8_t presc_shift = 4;
const uint8_t TCCR1B_prescaler_bits = bit(CS12);
#endif // PRESC256

#ifdef PRESC1024  // max TC1 period is 4194ms
const uint8_t presc_shift = 6;
const uint8_t TCCR1B_prescaler_bits = bit(CS10) | bit(CS12);
#endif // PRESC1024

#ifdef PRESC8
#define cts2us(cts) ((uint32_t)cts >> presc_shift) // counts to microseconds
#define us2cts(us) (us << presc_shift)             // microseconds to counts
#else
#define cts2us(cts) ((uint32_t)cts << presc_shift) // counts to microseconds
#define us2cts(us) (us >> presc_shift)             // microseconds to counts
#endif // PRESC8


void start_stroboscopic_acq(uint32_t n_frames = 0);
void start_continuous_acq(uint32_t n_frames = 0);

void stop_acq();
