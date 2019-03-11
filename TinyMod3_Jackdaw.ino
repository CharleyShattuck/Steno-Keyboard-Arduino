#include <Keyboard.h>

// Multiplexed Steno Keyboard
// This example program is in the public domain
// modified August 5, 2018 to make row pins HIGH
// for the diodes.
// modified August 18-19 for NKRO protocol
// modified February 12, 2019 for Jackdaw
// modified March 6, 2019 for TinyMod3

boolean pressed;
#define PROTOCOL 7

// four bytes for the TX Bolt protocol
#define NO_BOLTS 4
byte bolt[NO_BOLTS];

// five bytes in the keyboard matrix
#define NO_BYTES 5
byte data[NO_BYTES];

// matrix wiring
#define NO_ROWS 5
const byte row[]={9, 10, 11, 12, 13};
#define NO_COLS 5
const byte column[]={A1, A2, A3, A4, A5};

// parts of the keyboard
byte left = 0;
byte center = 0;
byte right = 0;
boolean star = false;
boolean number = false;
boolean caps = false;
boolean vowels = false;
boolean leaving = false;
boolean spacing = false;
boolean ekey = false;
boolean ykey = false;

void setup() {
  for(int i=0; i<NO_COLS; i++)  pinMode(column[i], INPUT_PULLUP);
  for(int i=0; i<NO_ROWS; i++){
    pinMode(row[i], OUTPUT); digitalWrite(row[i], HIGH);
  }
  pinMode(PROTOCOL, INPUT_PULLUP);
  if (digitalRead(PROTOCOL)) TX_BOLT();
  Keyboard.begin();
  delay(3000);
}

void scanTX() {
  do {
    for (int i = 0; i<NO_BYTES; i++) data[i] = 0;
    do {look ();} while (!pressed); delay(20);
  } while (!pressed);
  do {look ();} while (pressed);
}

// TX Bolt protocol
void sendTX(){
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

  for(int i=0; i<NO_BOLTS; i++) Serial.write(bolt[i]);
  delay(20);  // wait a bit before scanning again    
}

void TX_BOLT() {
  Serial.begin(9600);
  delay(3000);
  while(true) {
    scanTX();
    sendTX();
  }
}

void organize(){
  left = 0;
  if (data[4] & 0x01) left |= 0x01; // a
  if (data[4] & 0x02) left |= 0x04; // c
  if (data[4] & 0x04) left |= 0x10; // w
  if (data[4] & 0x08) left |= 0x40; // n
  if (data[2] & 0x01) left |= 0x02; // s
  if (data[2] & 0x02) left |= 0x08; // t
  if (data[2] & 0x04) left |= 0x20; // h
  if (data[2] & 0x08) left |= 0x80; // r
  center = 0;
  if (data[0] & 0x01) center |= 0x01; // a
  if (data[0] & 0x02) center |= 0x02; // o
  if (data[0] & 0x08) center |= 0x04; // e
  if (data[0] & 0x10) center |= 0x08; // u
  right = 0;
  if (data[3] & 0x01) right |= 0x01; // r
  if (data[3] & 0x02) right |= 0x04; // l
  if (data[3] & 0x04) right |= 0x10; // c
  if (data[3] & 0x08) right |= 0x40; // t
  if (data[1] & 0x01) right |= 0x02; // n
  if (data[1] & 0x02) right |= 0x08; // g
  if (data[1] & 0x04) right |= 0x20; // h
  if (data[1] & 0x08) right |= 0x80; // s
  star = false;
  if (data[4] & 0x10) star = true;
  if (data[2] & 0x10) star = true;
  number = false; if (data[0] & 0x04) number = true;
  vowels = false; if (data[0] & 0x1b) vowels = true;
  ekey = false; if (data[3] & 0x10) ekey = true;
  ykey = false; if (data[1] & 0x10) ykey = true;
}

void spit(String a) {
  if(a.length() == 0) return;
  if(caps) {
    String s;
    caps = false;
    s = a.substring(0, 1);
    s.toUpperCase();
    Keyboard.print(s);
    s = a.substring(1);
    Keyboard.print(s);
    return;
  }
  Keyboard.print(a);
}

void maybeSpace() {
  if (star) Keyboard.write(' ');
}

void numbers() { // number key is pressed if we got here
  if (ekey) {
    if (left & 0x03) {spit("11"); return;}
    if (left & 0x04) {spit("22"); return;}
    if (left & 0x10) {spit("33"); return;}
    if (left & 0x40) {spit("44"); return;}
    if (left & 0x80) {spit("55"); return;}
    if (right & 0x02) {spit("00"); return;}
    if (right &  0x01) {spit("66"); return;}
    if (right &  0x04) {spit("77"); return;}
    if (right & 0x10) {spit("88"); return;}
    if (right & 0x40) {spit("99"); return;}
  }
  if (right & 0x80) {
    if (right & 0x40) Keyboard.write('9');
    if (right & 0x10) Keyboard.write('8');
    if (right & 0x04) Keyboard.write('7');
    if (right & 0x01) Keyboard.write('6');
    if (right & 0x02) Keyboard.write('0');
    if (left & 0x80) Keyboard.write('5');
    if (left & 0x40) Keyboard.write('4');
    if (left & 0x10) Keyboard.write('3');
    if (left & 0x04) Keyboard.write('2');
    if (left & 0x03) Keyboard.write('1');
    return;
  }
  if (left & 0x03) Keyboard.write('1');
  if (left & 0x04) Keyboard.write('2');
  if (left & 0x10) Keyboard.write('3');
  if (left & 0x40) Keyboard.write('4');
  if (left & 0x80) Keyboard.write('5');
  if (right & 0x02) Keyboard.write('0');
  if (right & 0x01) Keyboard.write('6');
  if (right & 0x04) Keyboard.write('7');
  if (right & 0x10) Keyboard.write('8');
  if (right & 0x40) Keyboard.write('9');
  return;
}

boolean commands() { // before "maybeSpace()"
//  if(star & !right & !left & !center & !(ekey | ykey)) {
//    Keyboard.write(KEY_BACKSPACE); spacing = false; return true;
//  }
  if (center == 0) {
    if(left == 0x14) {
      if(right == 0x14) {Keyboard.write('.'); caps = true; return true;}
      if(right == 0x28) {Keyboard.write('!'); caps = true; return true;}
    }
    if(left == 0x28) {
      if(right == 0x28) {Keyboard.write(','); return true;}
      if(right == 0x14) {Keyboard.write('?'); caps = true; return true;}
    }
    if(left == 0xf0) { // contractions
      if(right == 0x00) {spit("'"); return true;}
      if(right == 0x0e) {spit("'d"); return true;}
      if(right == 0xb0) {spit("'d"); return true;}
      if(right == 0x40) {spit("'t"); return true;}
      if(right == 0x80) {spit("'s"); return true;}
      if(right == 0x06) {spit("'s"); return true;}
      if(right == 0x2a) {spit("'m"); return true;}
      if(right == 0x42) {spit("n't"); return true;}
      if(right == 0x01) {spit("'re"); return true;}
      if(right == 0x04) {spit("'ll"); return true;}
      if(right == 0x22) {spit("'ve"); return true;}
    }
    if (right == 0x16) {
      if (left == 0x00) {
        Keyboard.write(' '); spacing = false;
        return true;
      }
    }
    if((left == 0xa0) & (right == 0x0a)) {
      Keyboard.write(KEY_RETURN);
      spacing = false; return true;
    }
    if((left == 0x80) & (right == 0x02)) {
      Keyboard.write(KEY_RETURN);
      caps = true; spacing = false; return true;
    }
    if(left == 0xaa) {
      if(right == 0xa8) {caps = true; spacing = false; return true;}
      if(right == 0xaa) {caps = true; spacing = true; return true;}
      if(right == 0x81) {caps = false; return true;}
      if(right == 0x55) {Keyboard.write(KEY_ESC); return true;}
      if(right == 0x15) {Keyboard.write(KEY_HOME); return true;}
      if(right == 0x2a) {Keyboard.write(KEY_END); return true;}
      if(right == 0x26) {Keyboard.write(KEY_PAGE_UP); return true;}
      if(right == 0x19) {Keyboard.write(KEY_PAGE_DOWN); return true;}
// arrow keys go here      
      if(right == 0x0a) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.write(KEY_LEFT_ARROW);
        Keyboard.release(KEY_LEFT_CTRL);
        return true;
      }
      if(right == 0x28) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.write(KEY_RIGHT_ARROW);
        Keyboard.release(KEY_LEFT_CTRL);
        return true;
      }
      if(right == 0x3f) {Keyboard.write(KEY_CAPS_LOCK); return true;}
    }
    if(right == 0xc3) { // left modifiers
      if(left == 0x04) {Keyboard.press(KEY_LEFT_CTRL); return true;}
      if(left == 0x02) {Keyboard.press(KEY_LEFT_SHIFT); return true;}
      if(left == 0xc0) {Keyboard.press(KEY_LEFT_ALT); return true;}
      if(left == 0x0e) {Keyboard.press(KEY_LEFT_GUI); return true;}
      Keyboard.releaseAll(); return true;
    }
    if(right == 0xcc) { // right modifiers
      if(left == 0x04) {Keyboard.press(KEY_RIGHT_CTRL); return true;}
      if(left == 0x02) {Keyboard.press(KEY_RIGHT_SHIFT); return true;}
      if(left == 0xc0) {Keyboard.press(KEY_RIGHT_ALT); return true;}
      if(left == 0x0e) {Keyboard.press(KEY_RIGHT_GUI); return true;}
      Keyboard.releaseAll(); return true;
    }
    if(left == 0x98) { // function keys
      if(right == 0x26) {Keyboard.write(KEY_TAB); return true;}
      if(right == 0x01) {Keyboard.write(KEY_F1); return true;}
      if(right == 0x04) {Keyboard.write(KEY_F2); return true;}
      if(right == 0x10) {Keyboard.write(KEY_F3); return true;}
      if(right == 0x40) {Keyboard.write(KEY_F4); return true;}
      if(right == 0x02) {Keyboard.write(KEY_F5); return true;}
      if(right == 0x08) {Keyboard.write(KEY_F6); return true;}
      if(right == 0x20) {Keyboard.write(KEY_F7); return true;}
      if(right == 0x80) {Keyboard.write(KEY_F8); return true;}
      if(right == 0x03) {Keyboard.write(KEY_F9); return true;}
      if(right == 0x0c) {Keyboard.write(KEY_F10); return true;}
      if(right == 0x30) {Keyboard.write(KEY_F11); return true;}
      if(right == 0xc0) {Keyboard.write(KEY_F12); return true;}
    }
    if(right == 0x56) { // nlct open
      if(left == 0x14) {maybeSpace(); spit("("); spacing = false; return true;}
      if(left == 0x3c) {maybeSpace(); spit("["); spacing = false; return true;}
      if(left == 0x2c) {maybeSpace(); spit("{"); spacing = false; return true;}
      if(left == 0x40) {maybeSpace(); spit("<"); spacing = false; return true;}
      if(left == 0xc2) {maybeSpace(); spit("\""); spacing = false; return true;}
      if(left == 0x08) {maybeSpace(); spit("'"); spacing = false; return true;}
      if(left == 0x02) {maybeSpace(); spit("*"); spacing = false; return true;}
    }
    if(right == 0xa9) { // rghs close
      if(left == 0x14) {spit(")"); return true;} // p paren
      if(left == 0x3c) {spit("]"); return true;} // b bracket
      if(left == 0x2c) {spit("}"); return true;} // f French
      if(left == 0x40) {spit(">"); return true;} // n angle
      if(left == 0xc2) {spit("\""); return true;} // sl backslash
      if(left == 0x08) {spit("'"); return true;} // t tick
      if(left == 0x02) {spit("*"); return true;} // star
    }
  }
  return false;
}

void spitter (String a) {
  spit (a); spacing = true; return;
}

void spitno (String a) {
  spit (a); spacing = false; return;
}

boolean briefs() { // after "maybeSpace()"
  if ((center == 0x0c) & (left == 0xaa) & (right == 0)) {
    spitter ("I"); return true;
  }
  if(center == 0) {
    if (right == 0x55) {  // rlct
      if (left == 0x08) {spit("~"); return true;} // Tilde
      if (left == 0x60) {spit("@"); return true;} // Yat
      if (left == 0x20) {spit("#"); return true;} // Hash
      if (left == 0x0c) {spit("$"); return true;} // Dollar
      if (left == 0x14) {spit("%"); return true;} // Percent
      if (left == 0x84) {spit("^"); return true;} // CaRat
      if (left == 0x50) {spit("&"); return true;} // aMpesand
      if (left == 0x02) {spit("*"); return true;} // Star
      if (left == 0xd4) {spit("+"); return true;} // PLus
      if (left == 0xc8) {spit("="); return true;} // eQual
      if (left == 0xc2) {spit("/"); return true;} // SLash
      if (left == 0x3c) {spit("\\"); return true;} // Backslash
      if (left == 0xc4) {spit(":"); return true;} // CoLon
      if (left == 0x06) {spit(";"); return true;} // SemiColon
      if (left == 0x0e) {spit("`"); return true;} // Grave
      if (left == 0x8c) {spit("_"); return true;} // unDeRscore
      if (left == 0x2c) {spit("-"); return true;} // DasH
      if (left == 0x40) {Keyboard.write(KEY_INSERT); return true;} // iNseRt
      if (left == 0xcc) {Keyboard.write(KEY_DELETE); return true;} // DeLete
    }
    if (right == 0x3f) {
      if (left == 0x01) {spitno("A"); return true;}
      if (left == 0x3c) {spitno("B"); return true;}
      if (left == 0x04) {spitno("C"); return true;}
      if (left == 0x0c) {spitno("D"); return true;}
      if (left == 0x2c) {spitno("F"); return true;}
      if (left == 0x0e) {spitno("G"); return true;}
      if (left == 0x20) {spitno("H"); return true;}
      if (left == 0x58) {spitno("J"); return true;}
      if (left == 0x38) {spitno("K"); return true;}
      if (left == 0xc0) {spitno("L"); return true;}
      if (left == 0x50) {spitno("M"); return true;}
      if (left == 0x40) {spitno("N"); return true;}
      if (left == 0x14) {spitno("P"); return true;}
      if (left == 0xc8) {spitno("Q"); return true;}
      if (left == 0x80) {spitno("R"); return true;}
      if (left == 0x02) {spitno("S"); return true;}
      if (left == 0x08) {spitno("T"); return true;}
      if (left == 0x48) {spitno("V"); return true;}
      if (left == 0x10) {spitno("W"); return true;}
      if (left == 0x1a) {spitno("X"); return true;}
      if (left == 0x60) {spitno("Y"); return true;}
      if (left == 0x44) {spitno("Z"); return true;}
    }
  }
  if ((left == 0x00) & (right == 0x3f)) {
    if (center == 0x01) {spitno("A"); return true;}
    if (center == 0x02) {spitno("O"); return true;}
    if (center == 0x04) {spitno("E"); return true;}
    if (center == 0x08) {spitno("U"); return true;}
    if (center == 0x0c) {spitno("I"); return true;}
  }
  return false;
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

// allow key repeat for some keys
void moving(byte a) {
  Keyboard.press(a);
  do {look();} while(pressed);
  Keyboard.release(a);
}

boolean movement() {
  if((data[0] == 0x00) & (data[4] == 0x00) & (data[2] == 0x0f)) {
    if(data[1] == 0x01) {moving(KEY_LEFT_ARROW); return true;}
    if(data[3] == 0x02) {moving(KEY_UP_ARROW); return true;}
    if(data[1] == 0x02) {moving(KEY_DOWN_ARROW); return true;}
    if(data[1] == 0x04) {moving(KEY_RIGHT_ARROW); return true;}
  }
  if((data[0] == 0x00) & (data[1] == 0x01) &
      (data[2] == 0x06) & (data[3] == 0x06) & (data[4] == 0x06)) {
    moving(KEY_BACKSPACE); spacing = false; return true;
  }
  return false;
}

// wait for keypress and scan until all released
void scan(){
  leaving = false;
  do {
    for(int i=0; i<NO_BYTES; i++) data[i] = 0;
    do {look();} while(!pressed); delay(20);
  } while(!pressed);
  do {
    look();
    if (movement()) {leaving = true; return;}
  } while(pressed);
}

// left hand strings
const char l00[] PROGMEM = "";
const char l01[] PROGMEM = "a";
const char l02[] PROGMEM = "s";
const char l03[] PROGMEM = "as";
const char l04[] PROGMEM = "c";
const char l05[] PROGMEM = "ac";
const char l06[] PROGMEM = "sc"; // sc
const char l07[] PROGMEM = "asc";
const char l08[] PROGMEM = "t";
const char l09[] PROGMEM = "at";
const char l0a[] PROGMEM = "st"; // st
const char l0b[] PROGMEM = "ad";
const char l0c[] PROGMEM = "d"; // ct
const char l0d[] PROGMEM = "ast";
const char l0e[] PROGMEM = "g"; // sct
const char l0f[] PROGMEM = "ag";

const char l10[] PROGMEM = "w"; // w
const char l11[] PROGMEM = "aw"; // aw
const char l12[] PROGMEM = "sw"; // sw
const char l13[] PROGMEM = ""; // asw
const char l14[] PROGMEM = "p"; // cw
const char l15[] PROGMEM = "ap"; // acw
const char l16[] PROGMEM = "sp"; // scw
const char l17[] PROGMEM = "ass"; // ascn
const char l18[] PROGMEM = "tw"; // tw
const char l19[] PROGMEM = "att"; // atw
const char l1a[] PROGMEM = "x"; // stw
const char l1b[] PROGMEM = "ax"; // astw
const char l1c[] PROGMEM = "dw"; // ctw
const char l1d[] PROGMEM = "add"; // actw
const char l1e[] PROGMEM = "gw"; // sctw
const char l1f[] PROGMEM = "agg"; // asctw

const char l20[] PROGMEM = "h";
const char l21[] PROGMEM = "ah"; // ah
const char l22[] PROGMEM = "sh"; // sh
const char l23[] PROGMEM = "ash"; // ash
const char l24[] PROGMEM = "ch"; // ch
const char l25[] PROGMEM = "ach"; // ach
const char l26[] PROGMEM = "sch"; // sch
const char l27[] PROGMEM = ""; //asch
const char l28[] PROGMEM = "th"; // th
const char l29[] PROGMEM = "ath"; // ath
const char l2a[] PROGMEM = ""; // sth
const char l2b[] PROGMEM = "asth"; // asth
const char l2c[] PROGMEM = "f"; // cth
const char l2d[] PROGMEM = "af"; // acth
const char l2e[] PROGMEM = "gh"; // scth
const char l2f[] PROGMEM = "agh"; // ascth

const char l30[] PROGMEM = "wh"; // wh
const char l31[] PROGMEM = "awh"; // awh
const char l32[] PROGMEM = ""; // swh
const char l33[] PROGMEM = ""; // aswh
const char l34[] PROGMEM = "ph"; // cwh
const char l35[] PROGMEM = "aph"; // acwh
const char l36[] PROGMEM = "sph"; // scwh
const char l37[] PROGMEM = "asph"; // ascwh
const char l38[] PROGMEM = "k"; // twh
const char l39[] PROGMEM = "ak"; // atwh
const char l3a[] PROGMEM = "sk"; // stwh
const char l3b[] PROGMEM = "ask"; // astwh
const char l3c[] PROGMEM = "b"; // ctwh
const char l3d[] PROGMEM = "ab"; // actwh
const char l3e[] PROGMEM = ""; // sctwh
const char l3f[] PROGMEM = "abb"; // asctwh

const char l40[] PROGMEM = "n"; 
const char l41[] PROGMEM = "an"; // an
const char l42[] PROGMEM = "sn"; // sn
const char l43[] PROGMEM = "ann"; // asn
const char l44[] PROGMEM = "z"; // cn
const char l45[] PROGMEM = "az"; // acn
const char l46[] PROGMEM = "ss"; // scn
const char l47[] PROGMEM = "ass"; // ascn
const char l48[] PROGMEM = "v"; // tn
const char l49[] PROGMEM = "av"; // atn
const char l4a[] PROGMEM = "sv"; // stn
const char l4b[] PROGMEM = ""; // astn
const char l4c[] PROGMEM = "dev"; // ctn
const char l4d[] PROGMEM = "adv"; // actn
const char l4e[] PROGMEM = "gn"; // sctn
const char l4f[] PROGMEM = "agn"; // asctn

const char l50[] PROGMEM = "m"; // wn
const char l51[] PROGMEM = "am"; // awn
const char l52[] PROGMEM = "sm"; // swn
const char l53[] PROGMEM = "asm"; // aswn
const char l54[] PROGMEM = "pn"; // cwn
const char l55[] PROGMEM = "amm"; // acwn
const char l56[] PROGMEM = ""; // scwn
const char l57[] PROGMEM = "app"; // ascwn
const char l58[] PROGMEM = "j"; // twn
const char l59[] PROGMEM = "aj"; // atwn
const char l5a[] PROGMEM = ""; // stwn
const char l5b[] PROGMEM = ""; // astwn
const char l5c[] PROGMEM = "dem"; // ctwn
const char l5d[] PROGMEM = "adm"; // actwn
const char l5e[] PROGMEM = ""; // sctwn
const char l5f[] PROGMEM = "adj"; // asctwn

const char l60[] PROGMEM = "y"; // hn
const char l61[] PROGMEM = "ay"; // ahn
const char l62[] PROGMEM = "sy"; // shn
const char l63[] PROGMEM = "asy"; // ashn
const char l64[] PROGMEM = "cy"; // chn
const char l65[] PROGMEM = "acc"; // achn
const char l66[] PROGMEM = ""; // schn
const char l67[] PROGMEM = ""; // aschn
const char l68[] PROGMEM =  "ty"; // thn
const char l69[] PROGMEM = ""; // athn
const char l6a[] PROGMEM = "sty"; // sthn
const char l6b[] PROGMEM = ""; // asthn
const char l6c[] PROGMEM = "dy"; // cthn
const char l6d[] PROGMEM = "aff"; // acthn
const char l6e[] PROGMEM = "gy"; // scthn
const char l6f[] PROGMEM = "aft"; // ascthn

const char l70[] PROGMEM = "my"; // whn
const char l71[] PROGMEM = ""; // awhn
const char l72[] PROGMEM = ""; // swhn
const char l73[] PROGMEM = ""; // aswhn
const char l74[] PROGMEM = "py"; // cwhn
const char l75[] PROGMEM = ""; // acwhn
const char l76[] PROGMEM = "spy"; // scwhn
const char l77[] PROGMEM = "asphy"; // ascwhn
const char l78[] PROGMEM = "kn"; // twhn
const char l79[] PROGMEM = "ackn"; // atwhn
const char l7a[] PROGMEM = "xy"; // stwhn
const char l7b[] PROGMEM = ""; // astwhn
const char l7c[] PROGMEM = "by"; // ctwhn
const char l7d[] PROGMEM = "aby"; // actwhn
const char l7e[] PROGMEM = ""; // scthnr
const char l7f[] PROGMEM = "affl"; // ascthnr

const char l80[] PROGMEM = "r"; 
const char l81[] PROGMEM = "ar"; // ar
const char l82[] PROGMEM = "ser"; // sr
const char l83[] PROGMEM = "arr"; // asr 
const char l84[] PROGMEM = "cr"; // cr
const char l85[] PROGMEM = "acr"; // acr 
const char l86[] PROGMEM = "scr"; // scr
const char l87[] PROGMEM = "ascr"; // ascr
const char l88[] PROGMEM = "tr";  // tr 
const char l89[] PROGMEM = "atr"; // atr
const char l8a[] PROGMEM = "str"; // str
const char l8b[] PROGMEM = ""; // astr 
const char l8c[] PROGMEM = "dr"; // ctr
const char l8d[] PROGMEM = "adr"; // actr
const char l8e[] PROGMEM = "gr"; // sctr
const char l8f[] PROGMEM = "agr"; // asctr

const char l90[] PROGMEM = "wr"; // wr
const char l91[] PROGMEM = ""; // awr
const char l92[] PROGMEM = ""; // swr
const char l93[] PROGMEM = ""; // aswr
const char l94[] PROGMEM = "pr"; // cwr
const char l95[] PROGMEM = "apr"; // acwr
const char l96[] PROGMEM = "spr"; // scwr
const char l97[] PROGMEM = "appr"; // ascwr 
const char l98[] PROGMEM = ""; // twr
const char l99[] PROGMEM = "attr"; // atwr
const char l9a[] PROGMEM = "xr"; // stwr
const char l9b[] PROGMEM = ""; // astwr
const char l9c[] PROGMEM = "der"; // ctwr
const char l9d[] PROGMEM = "addr"; // actwr
const char l9e[] PROGMEM = ""; // sctwr
const char l9f[] PROGMEM = "aggr"; // asctwr

const char la0[] PROGMEM = "rh"; // hr
const char la1[] PROGMEM = ""; // ahr 
const char la2[] PROGMEM = "shr"; // shr
const char la3[] PROGMEM = ""; // ashr
const char la4[] PROGMEM = "chr"; // chr
const char la5[] PROGMEM = "accr"; // achr
const char la6[] PROGMEM = ""; // schr
const char la7[] PROGMEM = ""; // aschr
const char la8[] PROGMEM = "thr"; // thr
const char la9[] PROGMEM = ""; // athr
const char laa[] PROGMEM = ""; // sthr
const char lab[] PROGMEM = ""; // asthr
const char lac[] PROGMEM = "fr"; // cthr
const char lad[] PROGMEM = "afr"; // acthr
const char lae[] PROGMEM = ""; // scthr
const char laf[] PROGMEM = "affr"; // ascthr

const char lb0[] PROGMEM = ""; // whr 
const char lb1[] PROGMEM = ""; // awhr
const char lb2[] PROGMEM = ""; // swhr
const char lb3[] PROGMEM = ""; // aswhr
const char lb4[] PROGMEM = "phr"; // cwhr
const char lb5[] PROGMEM = "aphr"; // acwhr 
const char lb6[] PROGMEM = ""; // scwhr 
const char lb7[] PROGMEM = ""; // ascwhr
const char lb8[] PROGMEM = "kr"; // twhr
const char lb9[] PROGMEM = ""; // atwhr
const char lba[] PROGMEM = ""; // stwhr
const char lbb[] PROGMEM = ""; // astwhr
const char lbc[] PROGMEM = "br"; // ctwhr
const char lbd[] PROGMEM = "abr"; // actwhr
const char lbe[] PROGMEM = ""; // sctwhr
const char lbf[] PROGMEM = "abbr"; // asctwhr

const char lc0[] PROGMEM = "l"; // nr
const char lc1[] PROGMEM = "al"; // anr
const char lc2[] PROGMEM = "sl"; // snr
const char lc3[] PROGMEM = "asl"; // asnr 
const char lc4[] PROGMEM = "cl"; // cnr
const char lc5[] PROGMEM = ""; // acnr
const char lc6[] PROGMEM = ""; // scnr
const char lc7[] PROGMEM = ""; // ascnr
const char lc8[] PROGMEM = "q"; // tnr
const char lc9[] PROGMEM = "aq"; // atnr
const char lca[] PROGMEM = "sq"; // stnr
const char lcb[] PROGMEM = "asq"; // astnr
const char lcc[] PROGMEM = "del"; // ctnr
const char lcd[] PROGMEM = "acq"; // actnr
const char lce[] PROGMEM = "gl"; // sctnr
const char lcf[] PROGMEM = "agl"; // asctnr

const char ld0[] PROGMEM = "mr"; // wnr
const char ld1[] PROGMEM = "all"; // awnr
const char ld2[] PROGMEM = ""; // swnr
const char ld3[] PROGMEM = ""; // aswnr
const char ld4[] PROGMEM = "pl"; // cwnr
const char ld5[] PROGMEM = "apl"; // acwnr
const char ld6[] PROGMEM = "spl"; // scwnr
const char ld7[] PROGMEM = "appl"; // ascwnr
const char ld8[] PROGMEM = "jer"; // twnr
const char ld9[] PROGMEM = ""; // atwnr
const char lda[] PROGMEM = "serv"; // stwnr
const char ldb[] PROGMEM = ""; // astwnr
const char ldc[] PROGMEM = ""; // ctwnr
const char ldd[] PROGMEM = "addl"; // actwnr
const char lde[] PROGMEM = ""; // sctwnr
const char ldf[] PROGMEM = "aggl"; // asctwnr

const char le0[] PROGMEM = "ly"; // hnr
const char le1[] PROGMEM = ""; // ahnr
const char le2[] PROGMEM = "sly"; // shnr
const char le3[] PROGMEM = ""; // ashnr
const char le4[] PROGMEM = "cry"; // chnr
const char le5[] PROGMEM = "accl"; // achnr
const char le6[] PROGMEM = ""; // schnr
const char le7[] PROGMEM = ""; // aschnr
const char le8[] PROGMEM = "try"; // thnr
const char le9[] PROGMEM = "athl"; // athnr
const char lea[] PROGMEM = "stry"; // sthnr
const char leb[] PROGMEM = ""; // asthnr
const char lec[] PROGMEM = "fl"; // cthnr
const char led[] PROGMEM = "afl"; // acthnr
const char lee[] PROGMEM = ""; // scthnr
const char lef[] PROGMEM = "affl"; // ascthnr

const char lf0[] PROGMEM = ""; // whnr
const char lf1[] PROGMEM = ""; // awhnr
const char lf2[] PROGMEM = ""; // swhnr
const char lf3[] PROGMEM = ""; // aswhnr
const char lf4[] PROGMEM = "phl"; // cwhnr
const char lf5[] PROGMEM = ""; // acwhnr
const char lf6[] PROGMEM = ""; // scwhnr
const char lf7[] PROGMEM = ""; // ascwhnr
const char lf8[] PROGMEM = "kl"; // twhnr
const char lf9[] PROGMEM = ""; // atwhnr
const char lfa[] PROGMEM = ""; // stwhnr
const char lfb[] PROGMEM = ""; // astwhnr
const char lfc[] PROGMEM = "bl"; // ctwhnr
const char lfd[] PROGMEM = "abl"; // actwhnr
const char lfe[] PROGMEM = ""; // sctwhnr
const char lff[] PROGMEM = ""; // asctwhnr

const char* const left_side[] PROGMEM = {
  l00, l01, l02, l03, l04, l05, l06, l07,
  l08, l09, l0a, l0b, l0c, l0d, l0e, l0f,

  l10, l11, l12, l13, l14, l15, l16, l17,
  l18, l19, l1a, l1b, l1c, l1d, l1e, l1f,

  l20, l21, l22, l23, l24, l25, l26, l27,
  l28, l29, l2a, l2b, l2c, l2d, l2e, l2f,

  l30, l31, l32, l33, l34, l35, l36, l37,
  l38, l39, l3a, l3b, l3c, l3d, l3e, l3f,

  l40, l41, l43, l43, l44, l45, l46, l47,
  l48, l49, l4a, l4b, l4c, l4d, l4e, l4f,

  l50, l51, l52, l53, l54, l55, l56, l57,
  l58, l59, l5a, l5b, l5c, l5d, l5e, l5f,

  l60, l61, l62, l63, l64, l65, l66, l67,
  l68, l69, l6a, l6b, l6c, l6d, l6e, l6f,

  l70, l71, l72, l73, l74, l75, l76, l77,
  l78, l79, l7a, l7b, l7c, l7d, l7e, l7f,

  l80, l81, l82, l83, l84, l85, l86, l87,
  l88, l89, l8a, l8b, l4c, l4d, l8e, l8f,

  l90, l91, l92, l93, l94, l95, l96, l97,
  l98, l99, l9a, l9b, l9c, l9d, l9e, l9f,

  la0, la1, la2, la3, la4, la5, la6, la7,
  la8, la9, laa, lab, lac, lad, lae, laf,

  lb0, lb1, lb2, lb3, lb4, lb5, lb6, lb7,
  lb8, lb9, lba, lbb, lbc, lbd, lbe, lbf,

  lc0, lc1, lc2, lc3, lc4, lc5, lc6, lc7,
  lc8, lc9, lca, lcb, lcc, lcd, lce, lcf,

  ld0, ld1, ld2, ld3, ld4, ld5, ld6, ld7,
  ld8, ld9, lda, ldb, ldc, ldd, lde, ldf,

  le0, le1, le2, le3, le4, le5, le6, le7,
  le8, le9, lea, leb, lec, led, lee, lef,

  lf0, lf1, lf2, lf3, lf4, lf5, lf6, lf7,
  lf8, lf9, lfa, lfb, lfc, lfd, lfe, lff
};

void sendLeft() {
  char buffer[10];
  strcpy_P(buffer, (char*)pgm_read_word(&(left_side[left])));
  spit(buffer);
}

// thumb strings with number key
const char cn0[] PROGMEM = "";
const char cn1[] PROGMEM = "ia"; // A
const char cn2[] PROGMEM = "oo"; // O
const char cn3[] PROGMEM = "ao"; // AO
const char cn4[] PROGMEM = "ee"; // E
const char cn5[] PROGMEM = "ae"; // AE
const char cn6[] PROGMEM = "eo"; // OE
const char cn7[] PROGMEM = "ei"; // AOE
const char cn8[] PROGMEM = "ua"; // U
const char cn9[] PROGMEM = "";
const char cn10[] PROGMEM = "uo"; // OU
const char cn11[] PROGMEM = "";
const char cn12[] PROGMEM = "eu"; // EU
const char cn13[] PROGMEM = "";
const char cn14[] PROGMEM = "io"; // OEU
const char cn15[] PROGMEM = "";

const char* const center_number_side[] PROGMEM = {
  cn0, cn1, cn2, cn3, cn4, cn5, cn6, cn7, cn8, cn9,
  cn10, cn11, cn12, cn13, cn14, cn15,
};

const char c0[] PROGMEM = "";
const char c1[] PROGMEM = "a"; // A
const char c2[] PROGMEM = "o"; // O
const char c3[] PROGMEM = "oa"; // AO
const char c4[] PROGMEM = "e"; // E
const char c5[] PROGMEM = "ea"; // AE
const char c6[] PROGMEM = "oe"; // OE
const char c7[] PROGMEM = "ie"; // AOE
const char c8[] PROGMEM = "u"; // U
const char c9[] PROGMEM = "au"; // AU
const char c10[] PROGMEM = "ou"; // OU
const char c11[] PROGMEM = "ui";
const char c12[] PROGMEM = "i"; // EU
const char c13[] PROGMEM = "ai";
const char c14[] PROGMEM = "oi"; // OEU
const char c15[] PROGMEM = "iu";

const char* const center_side[] PROGMEM = {
  c0, c1, c2, c3, c4, c5, c6, c7, c8, c9,
  c10, c11, c12, c13, c14, c15
};

void sendCenter() {
  char buffer[10];
  if(number) {
    strcpy_P(buffer, (char*)pgm_read_word(&(center_number_side[center])));
    spit(buffer); return;
  }
  strcpy_P(buffer, (char*)pgm_read_word(&(center_side[center])));
  spit(buffer); return;
}

// right hand strings
const char r0[] PROGMEM = "";
const char r1[] PROGMEM = "r";
const char r2[] PROGMEM = "n";
const char r3[] PROGMEM = "rn";
const char r4[] PROGMEM = "l";
const char r5[] PROGMEM = "rl";
const char r6[] PROGMEM = "s";
const char r7[] PROGMEM = "ll";
const char r8[] PROGMEM = "g";
const char r9[] PROGMEM = "rg";
const char r10[] PROGMEM = "ng"; 
const char r11[] PROGMEM = "gn"; 
const char r12[] PROGMEM = "lg";
const char r13[] PROGMEM = "";
const char r14[] PROGMEM = "d";
const char r15[] PROGMEM = "dl";
const char r16[] PROGMEM = "c";
const char r17[] PROGMEM = "rc";
const char r18[] PROGMEM = "nc";
const char r19[] PROGMEM = "";
const char r20[] PROGMEM = "p";
const char r21[] PROGMEM = "rp";
const char r22[] PROGMEM = "sp";
const char r23[] PROGMEM = "pl";
const char r24[] PROGMEM = "b";
const char r25[] PROGMEM = "rb";
const char r26[] PROGMEM = "gg";
const char r27[] PROGMEM = "";
const char r28[] PROGMEM = "bl";
const char r29[] PROGMEM = "";
const char r30[] PROGMEM = "ld";
const char r31[] PROGMEM = "lb";
const char r32[] PROGMEM = "h";
const char r33[] PROGMEM = "w";
const char r34[] PROGMEM = "v";
const char r35[] PROGMEM = "wn";
const char r36[] PROGMEM = "z";
const char r37[] PROGMEM = "wl";
const char r38[] PROGMEM = "sh";
const char r39[] PROGMEM = "lv";
const char r40[] PROGMEM = "gh";
const char r41[] PROGMEM = "rgh";
const char r42[] PROGMEM = "m";
const char r43[] PROGMEM = "rm";
const char r44[] PROGMEM = "x";
const char r45[] PROGMEM = "";
const char r46[] PROGMEM = "sm";
const char r47[] PROGMEM = "lm";
const char r48[] PROGMEM = "ch";
const char r49[] PROGMEM = "rch";
const char r50[] PROGMEM = "nch";
const char r51[] PROGMEM = "rv";
const char r52[] PROGMEM = "ph";
const char r53[] PROGMEM = "";
const char r54[] PROGMEM = "";
const char r55[] PROGMEM = "lch";
const char r56[] PROGMEM = "f";
const char r57[] PROGMEM = "rf";
const char r58[] PROGMEM = "mb";
const char r59[] PROGMEM = "";
const char r60[] PROGMEM = "lf";
const char r61[] PROGMEM = "";
const char r62[] PROGMEM = "mp";
const char r63[] PROGMEM = "";
const char r64[] PROGMEM = "t";
const char r65[] PROGMEM = "rt";
const char r66[] PROGMEM = "nt";
const char r67[] PROGMEM = "rnt";
const char r68[] PROGMEM = "lt";
const char r69[] PROGMEM = "";
const char r70[] PROGMEM = "st";
const char r71[] PROGMEM = "rst";
const char r72[] PROGMEM = "k";
const char r73[] PROGMEM = "rk";
const char r74[] PROGMEM = "nk";
const char r75[] PROGMEM = "";
const char r76[] PROGMEM = "kl";
const char r77[] PROGMEM = "";
const char r78[] PROGMEM = "sk";
const char r79[] PROGMEM = "lk";
const char r80[] PROGMEM = "ct";
const char r81[] PROGMEM = "";
const char r82[] PROGMEM = "tion";
const char r83[] PROGMEM = "";
const char r84[] PROGMEM = "pt";
const char r85[] PROGMEM = "";
const char r86[] PROGMEM = "nst";
const char r87[] PROGMEM = "lp";
const char r88[] PROGMEM = "ck";
const char r89[] PROGMEM = "";
const char r90[] PROGMEM = "bt";
const char r91[] PROGMEM = "";
const char r92[] PROGMEM = "ckl";
const char r93[] PROGMEM = "";
const char r94[] PROGMEM = "";
const char r95[] PROGMEM = "";
const char r96[] PROGMEM = "th";
const char r97[] PROGMEM = "rth";
const char r98[] PROGMEM = "nth";
const char r99[] PROGMEM = "wth";
const char r100[] PROGMEM = "lth";
const char r101[] PROGMEM = "";
const char r102[] PROGMEM = "";
const char r103[] PROGMEM = "";
const char r104[] PROGMEM = "ght";
const char r105[] PROGMEM = "wk";
const char r106[] PROGMEM = "ngth";
const char r107[] PROGMEM = "";
const char r108[] PROGMEM = "xt";
const char r109[] PROGMEM = "";
const char r110[] PROGMEM = "dth";
const char r111[] PROGMEM = "";
const char r112[] PROGMEM = "tch";
const char r113[] PROGMEM = "";
const char r114[] PROGMEM = "";
const char r115[] PROGMEM = "";
const char r116[] PROGMEM = "pth";
const char r117[] PROGMEM = "";
const char r118[] PROGMEM = "";
const char r119[] PROGMEM = "";
const char r120[] PROGMEM = "ft";
const char r121[] PROGMEM = "";
const char r122[] PROGMEM = "";
const char r123[] PROGMEM = "";
const char r124[] PROGMEM = "";
const char r125[] PROGMEM = "";
const char r126[] PROGMEM = "mpt";
const char r127[] PROGMEM = "";
const char r128[] PROGMEM = "s";
const char r129[] PROGMEM = "rs";
const char r130[] PROGMEM = "ns";
const char r131[] PROGMEM = "rns";
const char r132[] PROGMEM = "ls";
const char r133[] PROGMEM = "rls";
const char r134[] PROGMEM = "ss";
const char r135[] PROGMEM = "lls";
const char r136[] PROGMEM = "gs";
const char r137[] PROGMEM = "rgs";
const char r138[] PROGMEM = "ngs";
const char r139[] PROGMEM = "gns";
const char r140[] PROGMEM = "";
const char r141[] PROGMEM = "";
const char r142[] PROGMEM = "ds";
const char r143[] PROGMEM = "";
const char r144[] PROGMEM = "cs";
const char r145[] PROGMEM = "rcs";
const char r146[] PROGMEM = "nces";
const char r147[] PROGMEM = "";
const char r148[] PROGMEM = "ps";
const char r149[] PROGMEM = "rps";
const char r150[] PROGMEM = "sps";
const char r151[] PROGMEM = "ples";
const char r152[] PROGMEM = "bs";
const char r153[] PROGMEM = "rbs";
const char r154[] PROGMEM = "ggs";
const char r155[] PROGMEM = "";
const char r156[] PROGMEM = "bles";
const char r157[] PROGMEM = "";
const char r158[] PROGMEM = "lds";
const char r159[] PROGMEM = "lbs";
const char r160[] PROGMEM = "hs";
const char r161[] PROGMEM = "ws";
const char r162[] PROGMEM = "ves";
const char r163[] PROGMEM = "wns";
const char r164[] PROGMEM = "zes";
const char r165[] PROGMEM = "wls";
const char r166[] PROGMEM = "shes";
const char r167[] PROGMEM = "lves";
const char r168[] PROGMEM = "ghs";
const char r169[] PROGMEM = "";
const char r170[] PROGMEM = "ms";
const char r171[] PROGMEM = "rms";
const char r172[] PROGMEM = "xes";
const char r173[] PROGMEM = "";
const char r174[] PROGMEM = "sms";
const char r175[] PROGMEM = "lms";
const char r176[] PROGMEM = "d";
const char r177[] PROGMEM = "rd";
const char r178[] PROGMEM = "nd";
const char r179[] PROGMEM = "wd";
const char r180[] PROGMEM = "phs";
const char r181[] PROGMEM = "rld";
const char r182[] PROGMEM = "";
const char r183[] PROGMEM = "ld";
const char r184[] PROGMEM = "dg";
const char r185[] PROGMEM = "";
const char r186[] PROGMEM = "mbs";
const char r187[] PROGMEM = "";
const char r188[] PROGMEM = "";
const char r189[] PROGMEM = "";
const char r190[] PROGMEM = "mps";
const char r191[] PROGMEM = "dd";
const char r192[] PROGMEM = "ts";
const char r193[] PROGMEM = "rts";
const char r194[] PROGMEM = "nts";
const char r195[] PROGMEM = "";
const char r196[] PROGMEM = "lts";
const char r197[] PROGMEM = "";
const char r198[] PROGMEM = "sts";
const char r199[] PROGMEM = "rsts";
const char r200[] PROGMEM = "ks";
const char r201[] PROGMEM = "rks";
const char r202[] PROGMEM = "nks";
const char r203[] PROGMEM = "";
const char r204[] PROGMEM = "";
const char r205[] PROGMEM = "";
const char r206[] PROGMEM = "sks";
const char r207[] PROGMEM = "lks";
const char r208[] PROGMEM = "cts";
const char r209[] PROGMEM = "";
const char r210[] PROGMEM = "";
const char r211[] PROGMEM = "";
const char r212[] PROGMEM = "";
const char r213[] PROGMEM = "";
const char r214[] PROGMEM = "";
const char r215[] PROGMEM = "lps";
const char r216[] PROGMEM = "cks";
const char r217[] PROGMEM = "";
const char r218[] PROGMEM = "bts";
const char r219[] PROGMEM = "";
const char r220[] PROGMEM = "ckles";
const char r221[] PROGMEM = "";
const char r222[] PROGMEM = "";
const char r223[] PROGMEM = "";
const char r224[] PROGMEM = "ths";
const char r225[] PROGMEM = "rths"; 
const char r226[] PROGMEM = "nths";
const char r227[] PROGMEM = "wths";
const char r228[] PROGMEM = "";
const char r229[] PROGMEM = "";
const char r230[] PROGMEM = "";
const char r231[] PROGMEM = "";
const char r232[] PROGMEM = "ghts";
const char r233[] PROGMEM = "wks";
const char r234[] PROGMEM = "ngths";
const char r235[] PROGMEM = "";
const char r236[] PROGMEM = "";
const char r237[] PROGMEM = "";
const char r238[] PROGMEM = "dths";
const char r239[] PROGMEM = "";
const char r240[] PROGMEM = "ds";
const char r241[] PROGMEM = "rds";
const char r242[] PROGMEM = "nds";
const char r243[] PROGMEM = "wds";
const char r244[] PROGMEM = "pths";
const char r245[] PROGMEM = "rlds";
const char r246[] PROGMEM = "";
const char r247[] PROGMEM = "lds";
const char r248[] PROGMEM = "";
const char r249[] PROGMEM = "";
const char r250[] PROGMEM = "";
const char r251[] PROGMEM = "";
const char r252[] PROGMEM = "";
const char r253[] PROGMEM = "";
const char r254[] PROGMEM = "mpts";
const char r255[] PROGMEM = "";

const char* const right_side[] PROGMEM = {
  r0, r1, r2, r3, r4, r5, r6, r7, r8, r9,
  r10, r11, r12, r13, r14, r15, r16, r17, r18, r19,
  r20, r21, r22, r23, r24, r25, r26, r27, r28, r29,
  r30, r31, r32, r33, r34, r35, r36, r37, r38, r39,
  r40, r41, r42, r43, r44, r45, r46, r47, r48, r49,
  r50, r51, r52, r53, r54, r55, r56, r57, r58, r59,
  r60, r61, r62, r63, r64, r65, r66, r67, r68, r69,
  r70, r71, r72, r73, r74, r75, r76, r77, r78, r79,
  r80, r81, r82, r83, r84, r85, r86, r87, r88, r89,
  r90, r91, r92, r93, r94, r95, r96, r97, r98, r99,
  r100, r101, r102, r103, r104, r105, r106, r107, r108, r109,
  r110, r111, r112, r113, r114, r115, r116, r117, r118, r119,
  r120, r121, r122, r123, r124, r125, r126, r127, r128, r129,
  r130, r131, r132, r133, r134, r135, r136, r137, r138, r139,
  r140, r141, r142, r143, r144, r145, r146, r147, r148, r149,
  r150, r151, r152, r153, r154, r155, r156, r157, r158, r159,
  r160, r161, r162, r163, r164, r165, r166, r167, r168, r169,
  r170, r171, r172, r173, r174, r175, r176, r177, r178, r179,
  r180, r181, r182, r183, r184, r185, r186, r187, r188, r189,
  r190, r191, r192, r193, r194, r195, r196, r197, r198, r199,
  r200, r201, r202, r203, r204, r205, r206, r207, r208, r209,
  r210, r211, r212, r213, r214, r215, r216, r217, r218, r219,
  r220, r221, r222, r223, r224, r225, r226, r227, r228, r229,
  r230, r231, r232, r233, r234, r235, r236, r237, r238, r239,
  r240, r241, r242, r243, r244, r245, r246, r247, r248, r249,
  r250, r251, r252, r253, r254, r255
};

void sendRight() {
  char buffer[10];
  if(right == 0x0c) {
    if(ykey) {
      strcpy(buffer, "logy");
      spit(buffer);
      return;
    }
  }
  if(right == 0x50) {
    if (ekey) {
      strcpy(buffer, "cate");
      spit(buffer);
      return;
    }
  }
  if(right == 0xc0){
    if (ykey) {
      strcpy(buffer, "ys");
      spit(buffer);
      return;
    }
  }
  if(right == 0xc8) {
    if (ekey) {
      strcpy(buffer, "kes");
      spit(buffer);
      return;
    }
  }
  strcpy_P(buffer, (char*)pgm_read_word(&(right_side[right])));
  spit(buffer);
  if (ekey) spit("e");
  if (ykey) spit("y");
}

void run() {
  scan(); if(leaving) return;
  organize();
  if ((number) & (left == 0) & (right == 0) & (center == 0) & !star) {
    Keyboard.write (' '); return;
  }
  if ((star) & (left == 0) & (right == 0) & (center == 0) & !number) {
    Keyboard.write (KEY_BACKSPACE); return;
  }
  maybeSpace();
  if ((number) & (!star) & (!center)) {numbers(); return;}
  if (commands()) return;
  if (briefs()) return;
  sendLeft();
  sendCenter();
  sendRight();
}

void loop() {
  run();
}

