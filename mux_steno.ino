// Multiplexed Steno Keyboard
// This example program is in the public domain

#define r0 9
#define r1 10
#define r2 11
#define r3 8
#define c0 2
#define c1 3
#define c2 4
#define c3 5
#define c4 6
#define c5 7
byte b0, b1, b2, b3;
boolean pressed;

void setup() {
  for (int i=2; i<8; i++) pinMode(i, INPUT_PULLUP);
  for (int i=8; i<12; i++) pinMode(i, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
  delay(3000); // Apparently Arduino Micro needs this
}

void loop() {
  b0 = 0;
  b1 = 0x40;
  b2 = 0x80;
  b3 = 0xc0;
  do {scan();} while(!pressed);
  delay(5); // Debounce the first key pressed for 5 ms
  do {
    do {scan();} while(pressed);
    delay(10);
  } while(pressed); // Debounce the last release
  Serial.write(b0);
  Serial.write(b1);
  Serial.write(b2);
  Serial.write(b3);
}

void scan() {
  digitalWrite(13, HIGH); // toggle LED to time the scan
  pressed = false;
  b0 |= getRow(r0);
  b1 |= getRow(r1);
  b2 |= getRow(r2);
  b3 |= getRow(r3);
  digitalWrite(13, LOW);
}  

// Activate and read one row  
byte getRow(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  byte row = pinState(c0);
  row |= pinState(c1) << 1;
  row |= pinState(c2) << 2;
  row |= pinState(c3) << 3;
  row |= pinState(c4) << 4;
  row |= pinState(c5) << 5;
  pinMode(pin, INPUT);
  return row;
}

byte pinState(int pin) {
  byte state = !digitalRead(pin);
  if(state == 1) pressed = true;
  return state;
}
