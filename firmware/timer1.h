#include "sys_globals.h"

// Timer 1 prescaler configuration. See AtMega328 datasheet table 15-6
#define PRESC64

#ifdef PRESC64
const uint8_t presc_shift = 2;
const uint8_t TCCR1B_bits = bit(CS10) | bit(CS11);
#endif // PRESC64

#ifdef PRESC256
const uint8_t presc_shift = 4;
const uint8_t TCCR1B_bits = bit(CS12);
#endif // PRESC256

#ifdef PRESC1024
const uint8_t presc_shift = 6;
const uint8_t TCCR1B_bits = bit(CS10) | bit(CS12);
#endif // PRESC1024

#define cts2us(cts) (cts << presc_shift) // counts to microseconds
#define us2cts(us) (us >> presc_shift)   // microseconds to counts

void setup_timer1();
void reset_timer1();