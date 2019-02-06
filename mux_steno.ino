#include <Keyboard.h>

// Multiplexed Steno Keyboard
// This example program is in the public domain
// modified August 5, 2018 to make row pins HIGH
// for the diodes.
// modified August 18-19 for NKRO protocol

boolean pressed;
boolean first;

// four bytes for the TX Bolt protocol
#define NO_BYTES 4
byte data[NO_BYTES];

// matrix wiring
#define NO_ROWS 4
const byte row[]={9, 10, 11, 12};
#define NO_COLS 6
const byte column[]={A0, A1, A2, A3, A4, A5};

// protocol choice on this  pin
#define PROTOCOL 7  // TinyMod 2
// #define PROTOCOL 8  // TinyMod 1

void setup() {
  for(int i=0; i<NO_COLS; i++)  pinMode(column[i], INPUT_PULLUP);
  for(int i=0; i<NO_ROWS; i++){
    pinMode(row[i], OUTPUT); digitalWrite(row[i], HIGH);
  }
  pinMode(PROTOCOL, INPUT_PULLUP);
  pinMode(13, OUTPUT); digitalWrite(13, HIGH);
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
  
  if (data[0] & 0x01)spit('q');
  if (data[0] & 0x02)spit('w');
  if (data[0] & 0x04)spit('s');
  if (data[0] & 0x08)spit('e');
  if (data[0] & 0x10)spit('d');
  if (data[0] & 0x20)spit('r');
   
  if (data[1] & 0x01)spit('f');  
  if (data[1] & 0x02)spit('c');  
  if (data[1] & 0x04)spit('v');  
  if (data[1] & 0x08)spit('t');
  if (data[1] & 0x10)spit('n');
  if (data[1] & 0x20)spit('m');

  if (data[2] & 0x01)spit('u');  
  if (data[2] & 0x02)spit('j');  
  if (data[2] & 0x04)spit('i');  
  if (data[2] & 0x08)spit('k');
  if (data[2] & 0x10)spit('o');
  if (data[2] & 0x20)spit('l');

  if (data[3] & 0x01)spit('p');  
  if (data[3] & 0x02)spit(';');  
  if (data[3] & 0x04)spit('[');  
  if (data[3] & 0x08)spit('\'');
  if (data[3] & 0x10)spit('3');

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

void sendTX(){
  for(int i=0; i<NO_BYTES; i++) Serial.write(data[i]);
  delay(20);  // wait a bit before scanning again    
}

void TX_Bolt(){
  Serial.begin(9600);
  delay(3000); // Apparently Arduino Micro needs this
  while(true){
    scan();
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
    for(int i=0; i<NO_BYTES; i++) data[i] = i * 0x40; // zero data
    do {look();} while(!pressed); delay(20);
  } while(!pressed);
  digitalWrite(13, HIGH);
  do {look();} while(pressed);
  digitalWrite(13, LOW);
}

