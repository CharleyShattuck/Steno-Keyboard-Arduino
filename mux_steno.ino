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
byte b0 = 0;
byte b1 = 0x40;
byte b2 = 0x80;
byte b3 = 0xc0;
boolean pressed = false;

void setup() {
  for (int i=2; i<8; i++) pinMode(i, INPUT_PULLUP);
  for (int i=8; i<12; i++) pinMode(i, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
  delay(3000); // Apparently Arduino Micro needs this
}

void loop() {
// Used to probe and verify the wiring
//  pinMode(O4, OUTPUT);
//  digitalWrite(O4, LOW);
//  int x = digitalRead(6);
//  if (x == HIGH) {
//    digitalWrite(13, HIGH);
//  } else {
//    digitalWrite(13, LOW);
//  }
  do {scan();} while(!pressed);
  delay(5); // Debounce the first key pressed for 5 ms
  while(pressed) {
    do {scan();} while(pressed);
    delay(5);
  }  // Debounce the last release
  TXBolt(); // Serial could take 4 ms minimum, add 1 more delay to debounce
//  scan(); // debug scan without pressed
//  show();
}

void scan() {
  digitalWrite(13, HIGH); // toggle LED to time the scan
  pressed = false;
  b0 = getRow(r0, b0);
  b1 = getRow(r1, b1);
  b2 = getRow(r2, b2);
  b3 = getRow(r3, b3);
  digitalWrite(13, LOW);
}  

// Activate and read one row  
byte getRow(int r, byte row) {
  pinMode(r, OUTPUT);
  digitalWrite(r, LOW);
  row = getKey(c0, 0x01, row);
  row = getKey(c1, 0x02, row);
  row = getKey(c2, 0x04, row);
  row = getKey(c3, 0x08, row);
  row = getKey(c4, 0x10, row);
  row = getKey(c5, 0x20, row);
  pinMode(r, INPUT);
  return row;
}

// Read and record one key into its byte
byte getKey(int k, byte b, byte v) {
  if (digitalRead(k) == 0) {
    v |= b;
    pressed = true;
  }
  return v;
}

// TX Bolt protocol, simplified, unconditional
void TXBolt() {
  Serial.write(b0);
  Serial.write(b1);
  Serial.write(b2);
  Serial.write(b3);
  b0 = 0;
  b1 = 0x40;
  b2 = 0x80;
  b3 = 0xc0;
} 

// Debug display, no TX protocol
void show() {
  Serial.print(b0, HEX); Serial.print(" ");
  Serial.print(b1, HEX); Serial.print(" ");
  Serial.print(b2, HEX); Serial.print(" ");
  Serial.print(b3, HEX); Serial.print("\n");
  b0 = 0;
  b1 = 0x40;
  b2 = 0x80;
  b3 = 0xc0;
}
