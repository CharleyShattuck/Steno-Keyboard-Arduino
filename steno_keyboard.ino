// This example program is in the public domain

boolean pressed = false;

byte val0 = 0;
byte val1 = 0;
byte val2 = 0;
byte val3 = 0;

void setup() {
  for (int i=30; i<54; i++) {
    pinMode(i, INPUT_PULLUP);
    }
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);  // Turn off LED
  Serial.begin(9600);
}

void loop() {
  do { scan(); } while (!pressed);
  do { scan(); } while (pressed);
  delay(50);
//  show();  // debug
  give();  // TX Bolt Protocol
  empty();
}

void scan() {
  pressed = false;

  val0 = getKey(36, 1, val0);
  val0 = getKey(31, 2, val0);
  val0 = getKey(30, 4, val0);
  val0 = getKey(33, 8, val0);
  val0 = getKey(32, 16, val0);
  val0 = getKey(35, 32, val0); 

  val1 = getKey(34, 1, val1);
  val1 = getKey(52, 2, val1);
  val1 = getKey(53, 4, val1);
  val1 = getKey(50, 8, val1);
  val1 = getKey(51, 16, val1);
  val1 = getKey(48, 32, val1); 
 
  val2 = getKey(47, 1, val2);
  val2 = getKey(45, 2, val2);
  val2 = getKey(49, 4, val2);
  val2 = getKey(46, 8, val2);
  val2 = getKey(43, 16, val2);
  val2 = getKey(44, 32, val2); 

  val3 = getKey(41, 1, val3);
  val3 = getKey(42, 2, val3);
  val3 = getKey(39, 4, val3);
  val3 = getKey(40, 8, val3);
  val3 = getKey(38, 16, val3);
} 

byte getKey(int k, byte b, byte v) {
  int val = digitalRead(k);
  if (val == 0) {
    v = b | v;
    pressed = true;
  }
  return v;
}

void empty() {
  val0 = 0;
  val1 = 0;
  val2 = 0;
  val3 = 0;
}

void cr() {
  Serial.print("\n");
}

void space() {
  Serial.print(" ");
}

void show() {
  Serial.print(val0, HEX);
  space();
  Serial.print(val1, HEX);
  space();
  Serial.print(val2, HEX);
  space();
  Serial.print(val3, HEX);
  cr();
}

void give() {
  Serial.write(val0);
  val1 = val1 | 64;
  Serial.write(val1);
  val2 = val2 | 128;
  Serial.write(val2);
  val3 = val3 | 192;
  Serial.write(val3);
  empty();
} 
