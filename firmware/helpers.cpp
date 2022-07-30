#include "helpers.h"

void send_ok() { Serial.print("OK\n"); }
void send_err() { Serial.print("ERR\n"); }
void send_err(const char *msg)
{
    Serial.print(msg);
    Serial.print("\n");
    Serial.write(data.ALEX_cycle_delay_us);
}
