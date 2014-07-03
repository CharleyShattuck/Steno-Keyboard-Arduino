// Multiplexed Steno Keyboard
// This example program is in the public domain

boolean pressed;

#define NO_BYTES 4
byte data[NO_BYTES];

#define NO_ROWS 4
const byte row[]={9, 10, 11, 8};

#define NO_COLS 6
const byte column[]={2, 3, 4, 5, 6, 7};

void setup() {
  for(int i=0; i<NO_COLS; i++)  pinMode(column[i], INPUT_PULLUP);
  for(int i=0; i<NO_ROWS; i++)  pinMode(row[i], INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.begin(9600);
  delay(3000); // Apparently Arduino Micro needs this
}

void loop() {
  for(int i=0; i<NO_BYTES; i++)  data[i] = i * 0x40;
  do {scan();} while(!pressed);
  delay(5); // Debounce the first key pressed for 5 ms
  do {
    do {scan();} while(pressed);
    delay(10);
  } while(pressed); // Debounce the last release
  for(int i=0; i<NO_BYTES; i++)  Serial.write(data[i]);
}

void scan() {
  digitalWrite(13, HIGH); // toggle LED to time the scan
  pressed = false;
  for(int i=0; i<NO_BYTES; i++)  data[i] |= getRow(row[i]);
  digitalWrite(13, LOW);
}  

// Activate and read one row  
byte getRow(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  byte row = 0;
  for(int i=0; i<NO_COLS; i++)  row |= pinState(column[i]) << i;
  pinMode(pin, INPUT);
  return row;
}

byte pinState(int pin) {
  byte state = !digitalRead(pin);
  if(state == 1)  pressed = true;
  return state;
}
