#define uchar unsigned char

void setup_timer_counter_1(){
  /* TCCR1: Timer Counter Control Register */
  //                 bit 7      6      5      4      3     2    1     0
  uchar _TCCR1A = 0;  // COM1A1 COM1A0 COM1B1 COM1B0 -     -    WGM10 WGM11
  uchar _TCCR1B = 0;  // ICNC1  ICES1  -      WGM13  WGM12 CS12 CS11  CS10
  uchar _TCCR1C = 0;  // FOC1A  FOC1B  -      -      -     -    -     -

  // Set COM (Compare Output Mode) for pins OC1A (#9) and OC1B (#10)
  _TCCR1A |= bit(COM1A0) | bit(COM1B0);     // toggle pins on compare match

  // Set WGM (Waveform Generation Mode)
  _TCCR1B |= bit(WGM12); // Mode 4, CTC with top=OCR1A

  // Select the clock source
  _TCCR1B |= bit(CS12) | bit(CS10);  // 1024x prescaler
  
  // Write the registers 
  TCCR1A = _TCCR1A;
  TCCR1B = _TCCR1B;
  TCCR1C = _TCCR1C;

  /* OCR1: Output Compare Register */
  // note: OCR1A is top; OCR1A must be larger than OCR1B
  // 0x3D09 = 15625 x 64us = 1s (with 1024 prescaler) for pin #9
  OCR1AH = 0x3D;
  OCR1AL = 0x09;
  // 0x0F42 = 3906 x 64us = 250ms (with 1024 prescaler) for pin #10
  OCR1BH = 0x0F;
  OCR1BL = 0x42;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  // All pins on port B are outputs
  DDRB = 0xFF; 

  setup_timer_counter_1();

  Serial.begin(2000000);
  Serial.setTimeout(10);  // ms
}


// Bytes in the data frame
struct Bytes {
  uchar cmd;
  uchar addr;
  uchar value;
};

// Data frame consists of bytes but can
// be represented as text
union DataFrame {
  struct Bytes bytes;
  uchar text[4];
};

union DataFrame df;
int charsRead;


// the loop function runs over and over again forever
void loop() {

  if (Serial.available() > 0){

    charsRead = Serial.readBytes(df.text, 3);
    if (charsRead == 3){
      switch (df.bytes.cmd){
        case 'W':
        case 'w':
          // Write the value to the register with given address
          *( (uchar *) df.bytes.addr) = df.bytes.value;
          break;

        case 'R':
        case 'r':
          // Read the value from the given register
          Serial.write(*((uchar *) df.bytes.addr));
          break;

        case 'D':
        case 'd':
          // debug
          Serial.print("Received "); Serial.write(df.text, 3);
          Serial.print(" addr=0x"); Serial.print(df.bytes.addr, HEX);
          Serial.print(" value=0x"); Serial.println(df.bytes.value, HEX);
          
          break;

      }
    }

  }
}
