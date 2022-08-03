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

typedef struct
{
    int16_t cycle;
    int16_t n_cycles;
} ISRcounter;

inline volatile ISRcounter t1ovflow = {0, 0};
inline volatile ISRcounter t1matchA = {0, 0};
inline volatile ISRcounter t1matchB = {0, 0};

/**
 * @brief High precision timer/counter with extended dynamic range
 *
 */
class Timer1
{
public:
    Timer1(uint32_t ICR1_us, uint32_t OCR1A_us, uint32_t OCR1B_us);
    uint32_t ICR1_32;
    uint32_t OCR1A_32;
    uint32_t OCR1B_32;
    void set_ICR1(uint32_t us);
    void set_OCR1A(uint32_t us);
    void set_OCR1B(uint32_t us);
    void reset();
    void rewind_time(int32_t us);
    uint32_t get_time();
    void pause();
    void run();

    void (*overfl_handler)() = noop;
    void (*matchA_handler)() = noop;
    void (*matchB_handler)() = noop;
};

inline Timer1 timer1 = Timer1(100000, 1000, 2000);
