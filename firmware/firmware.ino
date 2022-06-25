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
bool up = false;


// the loop function runs over and over again forever
void loop() {
  while (!Serial){
    delay(1);  // Wait until the serial port is ready
  }
  if (!up){
    Serial.flush();
    // Notify the host that we are ready
    Serial.println("Arduino is ready!");
    up = true;
  }

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
      }
    }
  }
}
