#define uchar unsigned char

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  // All pins on port B are outputs
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

/*
        case 'D':
        case 'd':
          // debug
          Serial.print("Received "); Serial.write(df.text, 3);
          Serial.print(" addr=0x"); Serial.print(df.bytes.addr, HEX);
          Serial.print(" value=0x"); Serial.println(df.bytes.value, HEX);
          
          break;*/

      }
    }

  }
}
