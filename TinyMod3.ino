#include <Keyboard.h>

// Multiplexed Steno Keyboard
// This example program is in the public domain
// modified August 5, 2018 to make row pins HIGH
// for the diodes.
// modified August 18-19 for NKRO protocol
// modified February 27, 2019, for TinyMod3

boolean pressed;
boolean first;

// temp storage for key data
#define NO_BYTES 5
byte data[NO_BYTES];

#define NO_BOLTS 4
byte bolt[NO_BOLTS];

// matrix wiring
#define NO_ROWS 5
const byte row[]={9, 10, 11, 12, 13};
#define NO_COLS 5
const byte column[]={A1, A2, A3, A4, A5};

// protocol choice on this  pin
#define PROTOCOL 7  // TinyMod3

// NKRO protocol
void spit(char a){
  Keyboard.press(a); delay(2);
  if(first == true){
    first = false;
  } else{
    Keyboard.release(a);
  }
}

void sendNKRO(){
  first = true;
  
  if (data[4] & 0x01)spit('q');
  if (data[4] & 0x02)spit('w');
  if (data[4] & 0x04)spit('e');
  if (data[4] & 0x08)spit('r');
  if (data[4] & 0x10)spit('t');

  if (data[3] & 0x01)spit('u');
  if (data[3] & 0x02)spit('i');
  if (data[3] & 0x04)spit('o');  
  if (data[3] & 0x08)spit('p');  
  if (data[3] & 0x10)spit('[');

  if (data[2] & 0x01)spit('a');
  if (data[2] & 0x02)spit('s');
  if (data[2] & 0x04)spit('d');
  if (data[2] & 0x08)spit('f');  
  if (data[2] & 0x10)spit('g');

  if (data[1] & 0x01)spit('j');
  if (data[1] & 0x02)spit('k');
  if (data[1] & 0x04)spit('l');
  if (data[1] & 0x08)spit(';');
  if (data[1] & 0x10)spit('\'');

  if (data[0] & 0x01)spit('c');
  if (data[0] & 0x02)spit('v');  
  if (data[0] & 0x04)spit('3');
  if (data[0] & 0x08)spit('n');
  if (data[0] & 0x10)spit('m');

  Keyboard.releaseAll();
}

void NKRO_keyboard(){
  Keyboard.begin();
  delay(3000);
  while(true){
    scan();
    sendNKRO();
  }
}

// TX Bolt protocol
void arrange() {
  for (byte i = 0; i < NO_BOLTS; i++) {
    bolt[i] = (i * 0x40);
  }
  if (data[4] & 0x01) bolt[0] |= 0x01; // S
  if (data[2] & 0x01) bolt[0] |= 0x01; // S
  if (data[4] & 0x02) bolt[0] |= 0x02; // T
  if (data[2] & 0x02) bolt[0] |= 0x04; // K
  if (data[4] & 0x04) bolt[0] |= 0x08; // P
  if (data[2] & 0x04) bolt[0] |= 0x10; // W 
  if (data[4] & 0x08) bolt[0] |= 0x20; // H

  if (data[2] & 0x08) bolt[1] |= 0x01; // R
  if (data[0] & 0x01) bolt[1] |= 0x02; // A
  if (data[0] & 0x02) bolt[1] |= 0x04; // O
  if (data[4] & 0x10) bolt[1] |= 0x08; // *
  if (data[2] & 0x10) bolt[1] |= 0x08; // *
  if (data[0] & 0x08) bolt[1] |= 0x10; // E
  if (data[0] & 0x10) bolt[1] |= 0x20; // U 

  if (data[3] & 0x01) bolt[2] |= 0x01; // F 
  if (data[1] & 0x01) bolt[2] |= 0x02; // R
  if (data[3] & 0x02) bolt[2] |= 0x04; // P
  if (data[1] & 0x02) bolt[2] |= 0x08; // B
  if (data[3] & 0x04) bolt[2] |= 0x10; // L 
  if (data[1] & 0x04) bolt[2] |= 0x20; // G

  if (data[3] & 0x08) bolt[3] |= 0x01; // T
  if (data[1] & 0x08) bolt[3] |= 0x02; // S
  if (data[3] & 0x10) bolt[3] |= 0x04; // D
  if (data[1] & 0x10) bolt[3] |= 0x08; // Z
  if (data[0] & 0x04) bolt[3] |= 0x10; // # 

}

void sendTX(){
  for(int i=0; i<NO_BOLTS; i++) Serial.write(bolt[i]);
  delay(20);  // wait a bit before scanning again    
}

void TX_Bolt(){
  Serial.begin(9600);
  delay(3000); // Apparently Arduino Micro needs this
  while(true){
    scan();
    arrange();
    sendTX();
  }
}

// scan for keypresses
byte pinState(int pin) {
  byte state = !digitalRead(pin);
  if(state == 1)  pressed = true;
  return state;
}

// Activate and read one row  
byte getRow(int pin) {
  digitalWrite(pin, LOW);
  byte row = 0;
  for(int i=0; i<NO_COLS; i++)  row |= pinState(column[i]) << i;
  digitalWrite(pin, HIGH);
  return row;
}

// look at each key once
void look() {
  pressed = false;
  for(int i=0; i<NO_BYTES; i++)  data[i] |= getRow(row[i]);
}  

// wait for keypress and scan until all released
void scan(){
  do {
    for(int i=0; i<NO_BYTES; i++) data[i] = 0; // zero data
    do {look();} while(!pressed); delay(20);
  } while(!pressed);
  do {look();} while(pressed);
}

void setup() {
  for(int i=0; i<NO_COLS; i++)  pinMode(column[i], INPUT_PULLUP);
  for(int i=0; i<NO_ROWS; i++){
    pinMode(row[i], OUTPUT); digitalWrite(row[i], HIGH);
  }
  pinMode(PROTOCOL, INPUT_PULLUP);
  if(digitalRead(PROTOCOL)){
    TX_Bolt(); // no shunt installed
  }else{
    NKRO_keyboard(); // shunt is installed
  }
}

// not using "loop" but Arduino IDE wants it to exist
void loop(){
  while(true);
}

