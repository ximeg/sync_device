#include "utils.h"

uint8_t count_bits(uint8_t v)
{
    unsigned int c; // c accumulates the total bits set in v

    for (c = 0; v; v >>= 1)
    {
        c += v & 1;
    }
    return c;
}

uint8_t decode_shutter_bits(uint8_t rx_bits)
{
    uint8_t cy2_bit = (rx_bits & 1) > 0;
    uint8_t cy3_bit = (rx_bits & 2) > 0;
    uint8_t cy5_bit = (rx_bits & 4) > 0;
    uint8_t cy7_bit = (rx_bits & 8) > 0;
    return (cy2_bit << CY2_PIN) | (cy3_bit << CY3_PIN) | (cy5_bit << CY5_PIN) | (cy7_bit << CY7_PIN);
}