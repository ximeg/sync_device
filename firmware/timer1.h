#include "sys_globals.h"

// Timer 1 prescaler configuration. See AtMega328 datasheet table 15-6
#define PRESC64

#ifdef PRESC64
const uint8_t presc_shift = 2;
const uint8_t TCCR1B_prescaler_bits = bit(CS10) | bit(CS11) | bit(CS12); // CS12 means EXTERNAL CLOCK!
#endif                                                                   // PRESC64

#ifdef PRESC256
const uint8_t presc_shift = 4;
const uint8_t TCCR1B_prescaler_bits = bit(CS12);
#endif // PRESC256

#ifdef PRESC1024
const uint8_t presc_shift = 6;
const uint8_t TCCR1B_prescaler_bits = bit(CS10) | bit(CS12);
#endif // PRESC1024

#define cts2us(cts) ((uint32_t)cts << presc_shift) // counts to microseconds
#define us2cts(us) (us >> presc_shift)             // microseconds to counts

void setup_timer1();
void reset_timer1();
void set_timer1_values();

/// Code below should be split between .h and .cpp

typedef struct T1
{
    uint16_t cycle; // current cycle
    uint16_t N_OVF_cycles;
    uint16_t N_matchA_cycles;
    uint16_t N_matchB_cycles;
} T1;

extern T1 t1;

void set_interframe_duration_us(uint32_t us); // delete sys.interframe_time_us??

void set_matchA_us(uint32_t us);

void set_matchB_us(uint32_t us);
