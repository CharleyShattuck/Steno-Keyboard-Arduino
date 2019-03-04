#include <Keyboard.h>

// Multiplexed Steno Keyboard
// This example program is in the public domain
// modified August 5, 2018 to make row pins HIGH
// for the diodes.
// modified August 18-19 for NKRO protocol
// modified February 12, 2019 for Jackdaw

boolean pressed;

// four bytes for the TX Bolt protocol
#define NO_BYTES 4
byte data[NO_BYTES];

// matrix wiring
#define NO_ROWS 4
const byte row[]={9, 10, 11, 12};
#define NO_COLS 6
const byte column[]={A0, A1, A2, A3, A4, A5};

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

void setup() {
  for(int i=0; i<NO_COLS; i++)  pinMode(column[i], INPUT_PULLUP);
  for(int i=0; i<NO_ROWS; i++){
    pinMode(row[i], OUTPUT); digitalWrite(row[i], HIGH);
  }
  pinMode(13, OUTPUT); digitalWrite(13, HIGH);
  Keyboard.begin();
  delay(3000);
}

void organize(){
  left = 0;
  // A key has room here for future hardware
  if (data[0] & 0x01) left |= 0x02;
  if (data[0] & 0x02) left |= 0x04;
  if (data[0] & 0x04) left |= 0x08;
  if (data[0] & 0x08) left |= 0x10;
  if (data[0] & 0x10) left |= 0x20;
  if (data[0] & 0x20) left |= 0x40;
  if (data[1] & 0x01) left |= 0x80;
  center = 0;
  if (data[1] & 0x02) center |= 0x01;
  if (data[1] & 0x04) center |= 0x02;
  if (data[1] & 0x10) center |= 0x04;
  if (data[1] & 0x20) center |= 0x08;
  right = 0;
  if (data[2] & 0x01) right |= 0x01;
  if (data[2] & 0x02) right |= 0x02;
  if (data[2] & 0x04) right |= 0x04;
  if (data[2] & 0x08) right |= 0x08;
  if (data[2] & 0x10) right |= 0x10;
  if (data[2] & 0x20) right |= 0x20;
  if (data[3] & 0x01) right |= 0x40;
  if (data[3] & 0x02) right |= 0x80;
  star = false; if (data[1] & 0x08) star = true;
  number = false; if (data[3] & 0x10) number = true;
  vowels = false; if (data[1] & 0x36) vowels = true;
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
  if (!star & spacing) Keyboard.write(' ');
}

void numbers() { // number key is pressed if we got here
  if (data[3] & 0x04) {
    if (left & 0x02) {spit("11"); return;}
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
    if (left & 0x02) Keyboard.write('1');
    return;
  }
  if (left & 0x02) Keyboard.write('1');
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
  if(star & !right & !left & !center & !(data[3] & 0x0c)) {
    Keyboard.write(KEY_BACKSPACE); spacing = false; return true;
  }
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
      if (left == 0x3c) {
        Keyboard.write(KEY_BACKSPACE); spacing = false;
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
      if(right == 0x28) {caps = false; return true;}
      if(right == 0x55) {Keyboard.write(KEY_ESC); return true;}
      if(right == 0x15) {Keyboard.write(KEY_HOME); return true;}
      if(right == 0x2a) {Keyboard.write(KEY_END); return true;}
      if(right == 0x26) {Keyboard.write(KEY_PAGE_UP); return true;}
      if(right == 0x19) {Keyboard.write(KEY_PAGE_DOWN); return true;}
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

boolean briefs() { // after "maybeSpace()"
  if((center == 0x0c) & (left == 0xaa)) {
    spit("I"); spacing = true; caps = false; return true;
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
  if((data[0] == 0x15) & (data[1] == 0x01) & (data[3] == 0)) {
    delay(20); // debounce the key?
      if(data[2] == 0x29) {moving(KEY_BACKSPACE); spacing = false; return true;}
      if(data[2] == 0x02) {moving(KEY_LEFT_ARROW); return true;}
      if(data[2] == 0x04) {moving(KEY_UP_ARROW); return true;}
      if(data[2] == 0x08) {moving(KEY_DOWN_ARROW); return true;}
      if(data[2] == 0x20) {moving(KEY_RIGHT_ARROW); return true;}
    }
  return false;
}

// wait for keypress and scan until all released
void scan(){
  leaving = false;
  do {
    for(int i=0; i<NO_BYTES; i++) data[i] = 0; // i * 0x40; // zero data
    do {look();} while(!pressed); delay(20);
  } while(!pressed);
  digitalWrite(13, HIGH);
  do {
    look();
    if (movement()) {digitalWrite(13, LOW); leaving = true; return;}
  } while(pressed);
  digitalWrite(13, LOW);
}

// left hand strings
const char l0[] PROGMEM = "";
const char l1[] PROGMEM = "";
const char l2[] PROGMEM = "s";
const char l3[] PROGMEM = "";
const char l4[] PROGMEM = "c";
const char l5[] PROGMEM = "";
const char l6[] PROGMEM = "sc"; // sc
const char l7[] PROGMEM = "";
const char l8[] PROGMEM = "t";
const char l9[] PROGMEM = "";
const char l10[] PROGMEM = "st"; // st
const char l11[] PROGMEM = "";
const char l12[] PROGMEM = "d"; // ct
const char l13[] PROGMEM = "";
const char l14[] PROGMEM = "g"; // sct
const char l15[] PROGMEM = "";
const char l16[] PROGMEM = "w";
const char l17[] PROGMEM = "";
const char l18[] PROGMEM = "sw"; // sw
const char l19[] PROGMEM = "";
const char l20[] PROGMEM = "p"; // cw
const char l21[] PROGMEM = "";
const char l22[] PROGMEM = "sp"; // scw
const char l23[] PROGMEM = "";
const char l24[] PROGMEM = "tw"; // tw
const char l25[] PROGMEM = "";
const char l26[] PROGMEM = "x"; // stw
const char l27[] PROGMEM = "";
const char l28[] PROGMEM = "dw"; // ctw
const char l29[] PROGMEM = "";
const char l30[] PROGMEM = "gw"; // sctw
const char l31[] PROGMEM = "";
const char l32[] PROGMEM = "h";
const char l33[] PROGMEM = "";
const char l34[] PROGMEM = "sh"; // sh
const char l35[] PROGMEM = "";
const char l36[] PROGMEM = "ch"; // ch
const char l37[] PROGMEM = "";
const char l38[] PROGMEM = "sch"; // sch
const char l39[] PROGMEM = "";
const char l40[] PROGMEM = "th"; // th
const char l41[] PROGMEM = "";
const char l42[] PROGMEM = "";
const char l43[] PROGMEM = "";
const char l44[] PROGMEM = "f"; // cth
const char l45[] PROGMEM = "";
const char l46[] PROGMEM = "gh"; // scth
const char l47[] PROGMEM = "";
const char l48[] PROGMEM = "wh"; // wh
const char l49[] PROGMEM = "";
const char l50[] PROGMEM = "";
const char l51[] PROGMEM = "";
const char l52[] PROGMEM = "ph"; // cwh
const char l53[] PROGMEM = "";
const char l54[] PROGMEM = "sph"; // scwh
const char l55[] PROGMEM = "";
const char l56[] PROGMEM = "k"; // twh
const char l57[] PROGMEM = "";
const char l58[] PROGMEM = "sk"; // stwh
const char l59[] PROGMEM = "";
const char l60[] PROGMEM = "b"; // ctwh
const char l61[] PROGMEM = "";
const char l62[] PROGMEM = "";
const char l63[] PROGMEM = "";
const char l64[] PROGMEM = "n"; 
const char l65[] PROGMEM = "";
const char l66[] PROGMEM = "sn"; // sn
const char l67[] PROGMEM = "";
const char l68[] PROGMEM = "z"; // cn
const char l69[] PROGMEM = "";
const char l70[] PROGMEM = "ss"; // scn
const char l71[] PROGMEM = "";
const char l72[] PROGMEM = "v"; // tn
const char l73[] PROGMEM = "";
const char l74[] PROGMEM = "sv"; // stn
const char l75[] PROGMEM = "";
const char l76[] PROGMEM = "dev"; // ctn
const char l77[] PROGMEM = "";
const char l78[] PROGMEM = "gn"; // sctn
const char l79[] PROGMEM = "";
const char l80[] PROGMEM = "m"; // wn
const char l81[] PROGMEM = "";
const char l82[] PROGMEM = "sm"; // swn
const char l83[] PROGMEM = "";
const char l84[] PROGMEM = "pn"; // cwn
const char l85[] PROGMEM = "";
const char l86[] PROGMEM = "";
const char l87[] PROGMEM = "";
const char l88[] PROGMEM = "j"; // twn
const char l89[] PROGMEM = "";
const char l90[] PROGMEM = "";
const char l91[] PROGMEM = "";
const char l92[] PROGMEM = "dem"; // ctwn
const char l93[] PROGMEM = "";
const char l94[] PROGMEM = "";
const char l95[] PROGMEM = "";
const char l96[] PROGMEM = "y"; // hn
const char l97[] PROGMEM = "";
const char l98[] PROGMEM = "sy"; // shn
const char l99[] PROGMEM = "";
const char l100[] PROGMEM = "cy"; // chn
const char l101[] PROGMEM = "";
const char l102[] PROGMEM = "";
const char l103[] PROGMEM = "";
const char l104[] PROGMEM =  "ty"; // thn
const char l105[] PROGMEM = "";
const char l106[] PROGMEM = "sty"; // sthn
const char l107[] PROGMEM = "";
const char l108[] PROGMEM = "dy"; // cthn
const char l109[] PROGMEM = "";
const char l110[] PROGMEM = "gy"; // scthn
const char l111[] PROGMEM = "";
const char l112[] PROGMEM = "my"; // whn
const char l113[] PROGMEM = "";
const char l114[] PROGMEM = "";
const char l115[] PROGMEM = "";
const char l116[] PROGMEM = "py"; // cwhn
const char l117[] PROGMEM = "";
const char l118[] PROGMEM = "spy"; // scwhn
const char l119[] PROGMEM = "";
const char l120[] PROGMEM = "kn"; // twhn
const char l121[] PROGMEM = "";
const char l122[] PROGMEM = "xy"; // stwhn
const char l123[] PROGMEM = "";
const char l124[] PROGMEM = "by"; // ctwhn
const char l125[] PROGMEM = "";
const char l126[] PROGMEM = "";
const char l127[] PROGMEM = "";
const char l128[] PROGMEM = "r"; 
const char l129[] PROGMEM = ""; 
const char l130[] PROGMEM = "ser"; // sr
const char l131[] PROGMEM = ""; 
const char l132[] PROGMEM = "cr"; // cr
const char l133[] PROGMEM = ""; 
const char l134[] PROGMEM = "scr"; // scr
const char l135[] PROGMEM = ""; 
const char l136[] PROGMEM = "tr";  // tr 
const char l137[] PROGMEM = ""; 
const char l138[] PROGMEM = "str"; // str
const char l139[] PROGMEM = ""; 
const char l140[] PROGMEM = "dr"; // ctr
const char l141[] PROGMEM = ""; 
const char l142[] PROGMEM = "gr"; // sctr
const char l143[] PROGMEM = ""; 
const char l144[] PROGMEM = "wr"; // wr
const char l145[] PROGMEM = ""; 
const char l146[] PROGMEM = ""; 
const char l147[] PROGMEM = ""; 
const char l148[] PROGMEM = "pr"; // cwr
const char l149[] PROGMEM = ""; 
const char l150[] PROGMEM = "spr"; // scwr
const char l151[] PROGMEM = ""; 
const char l152[] PROGMEM = ""; 
const char l153[] PROGMEM = ""; 
const char l154[] PROGMEM = "xr"; // stwr
const char l155[] PROGMEM = ""; 
const char l156[] PROGMEM = "der"; // ctwr
const char l157[] PROGMEM = ""; 
const char l158[] PROGMEM = ""; 
const char l159[] PROGMEM = ""; 
const char l160[] PROGMEM = "rh"; // hr
const char l161[] PROGMEM = ""; 
const char l162[] PROGMEM = "shr"; // shr
const char l163[] PROGMEM = ""; 
const char l164[] PROGMEM = "chr"; // chr
const char l165[] PROGMEM = ""; 
const char l166[] PROGMEM = ""; 
const char l167[] PROGMEM = ""; 
const char l168[] PROGMEM = "thr"; // thr
const char l169[] PROGMEM = ""; 
const char l170[] PROGMEM = ""; 
const char l171[] PROGMEM = ""; 
const char l172[] PROGMEM = "fr"; // cthr
const char l173[] PROGMEM = ""; 
const char l174[] PROGMEM = ""; 
const char l175[] PROGMEM = ""; 
const char l176[] PROGMEM = ""; 
const char l177[] PROGMEM = ""; 
const char l178[] PROGMEM = ""; 
const char l179[] PROGMEM = ""; 
const char l180[] PROGMEM = "phr"; // cwhr
const char l181[] PROGMEM = ""; 
const char l182[] PROGMEM = ""; 
const char l183[] PROGMEM = ""; 
const char l184[] PROGMEM = "kr"; // twhr
const char l185[] PROGMEM = ""; 
const char l186[] PROGMEM = ""; 
const char l187[] PROGMEM = ""; 
const char l188[] PROGMEM = "br"; // ctwhr
const char l189[] PROGMEM = ""; 
const char l190[] PROGMEM = ""; 
const char l191[] PROGMEM = ""; 
const char l192[] PROGMEM = "l"; // nr
const char l193[] PROGMEM = ""; 
const char l194[] PROGMEM = "sl"; // snr
const char l195[] PROGMEM = ""; 
const char l196[] PROGMEM = "cl"; // cnr
const char l197[] PROGMEM = ""; 
const char l198[] PROGMEM = ""; 
const char l199[] PROGMEM = ""; 
const char l200[] PROGMEM = "q"; // tnr
const char l201[] PROGMEM = ""; 
const char l202[] PROGMEM = "sq"; // stnr
const char l203[] PROGMEM = ""; 
const char l204[] PROGMEM = "del"; // ctnr
const char l205[] PROGMEM = ""; 
const char l206[] PROGMEM = "gl"; // sctnr
const char l207[] PROGMEM = ""; 
const char l208[] PROGMEM = "mr"; // wnr
const char l209[] PROGMEM = ""; 
const char l210[] PROGMEM = ""; 
const char l211[] PROGMEM = ""; 
const char l212[] PROGMEM = "pl"; // cwnr
const char l213[] PROGMEM = ""; 
const char l214[] PROGMEM = "spl"; // scwnr
const char l215[] PROGMEM = ""; 
const char l216[] PROGMEM = "jer"; // twnr
const char l217[] PROGMEM = ""; 
const char l218[] PROGMEM = "serv"; // stwnr
const char l219[] PROGMEM = ""; 
const char l220[] PROGMEM = ""; 
const char l221[] PROGMEM = ""; 
const char l222[] PROGMEM = ""; 
const char l223[] PROGMEM = ""; 
const char l224[] PROGMEM = "ly"; // hnr
const char l225[] PROGMEM = ""; 
const char l226[] PROGMEM = "sly"; // shnr
const char l227[] PROGMEM = ""; 
const char l228[] PROGMEM = "cry"; // chnr
const char l229[] PROGMEM = ""; 
const char l230[] PROGMEM = ""; 
const char l231[] PROGMEM = ""; 
const char l232[] PROGMEM = "try"; // thnr
const char l233[] PROGMEM = ""; 
const char l234[] PROGMEM = "stry"; // sthnr
const char l235[] PROGMEM = ""; 
const char l236[] PROGMEM = "fl"; // cthnr
const char l237[] PROGMEM = ""; 
const char l238[] PROGMEM = ""; 
const char l239[] PROGMEM = ""; 
const char l240[] PROGMEM = ""; 
const char l241[] PROGMEM = ""; 
const char l242[] PROGMEM = ""; 
const char l243[] PROGMEM = "phl"; // cwhnr
const char l244[] PROGMEM = ""; 
const char l245[] PROGMEM = ""; 
const char l246[] PROGMEM = ""; 
const char l247[] PROGMEM = "kl"; // twhnr
const char l248[] PROGMEM = ""; 
const char l249[] PROGMEM = ""; 
const char l250[] PROGMEM = ""; 
const char l251[] PROGMEM = "bl"; // ctwhnr
const char l252[] PROGMEM = ""; 
const char l253[] PROGMEM = ""; 
const char l254[] PROGMEM = ""; 
const char l255[] PROGMEM = ""; 

const char* const left_side[] PROGMEM = {
  l0, l1, l2, l3, l4, l5, l6, l7, l8, l9,
  l10, l11, l12, l13, l14, l15, l16, l17, l18, l19,
  l20, l21, l22, l23, l24, l25, l26, l27, l28, l29,
  l30, l31, l32, l33, l34, l35, l36, l37, l38, l39,
  l40, l41, l42, l43, l44, l45, l46, l47, l48, l49,
  l50, l51, l52, l53, l54, l55, l56, l57, l58, l59,
  l60, l61, l62, l63, l64, l65, l66, l67, l68, l69,
  l70, l71, l72, l73, l74, l75, l76, l77, l78, l79,
  l80, l81, l82, l83, l84, l85, l86, l87, l88, l89,
  l90, l91, l92, l93, l94, l95, l96, l97, l98, l99,
  l100, l101, l102, l103, l104, l105, l106, l107, l108, l109,
  l110, l111, l112, l113, l114, l115, l116, l117, l118, l119,
  l120, l121, l122, l123, l124, l125, l126, l127, l128, l129,
  l130, l131, l132, l133, l134, l135, l136, l137, l138, l139,
  l140, l141, l142, l143, l144, l145, l146, l147, l148, l149,
  l150, l151, l152, l153, l154, l155, l156, l157, l158, l159,
  l160, l161, l162, l163, l164, l165, l166, l167, l168, l169,
  l170, l171, l172, l173, l174, l175, l176, l177, l178, l179,
  l180, l181, l182, l183, l184, l185, l186, l187, l188, l189,
  l190, l191, l192, l193, l194, l195, l196, l197, l198, l199,
  l200, l201, l202, l203, l204, l205, l206, l207, l208, l209,
  l210, l211, l212, l213, l214, l215, l216, l217, l218, l219,
  l220, l221, l222, l223, l224, l225, l226, l227, l228, l229,
  l230, l231, l232, l233, l234, l235, l236, l237, l238, l239,
  l240, l241, l242, l243, l244, l245, l246, l247, l248, l249,
  l250, l251, l252, l253, l254, l255
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
    if(data[3] & 0x08) {
      strcpy(buffer, "logy");
      spit(buffer);
      return;
    }
  }
  if(right == 0x50) {
    if (data[3] & 0x04) {
      strcpy(buffer, "cate");
      spit(buffer);
      return;
    }
  }
  if(right == 0xc0){
    if (data[3] & 0x08) {
      strcpy(buffer, "ys");
      spit(buffer);
      return;
    }
  }
  if(right == 0xc8) {
    if (data[3] & 0x04) {
      strcpy(buffer, "kes");
      spit(buffer);
      return;
    }
  }
  strcpy_P(buffer, (char*)pgm_read_word(&(right_side[right])));
  spit(buffer);
  if (data[3] & 0x04) spit("e");
  if (data[3] & 0x08) spit("y");
}

void run() {
  scan(); if(leaving) return;
  organize();
  if ((number) & (!star) & (!center)) {numbers(); return;}
  if (commands()) return;
  maybeSpace();
  if (briefs()) return;
  sendLeft();
  sendCenter();
  sendRight();
  spacing = true;
}

void loop() {
  run();
}

