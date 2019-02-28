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
boolean skipping = false;
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
  Keyboard.print(a);
}

void spew(String  a, String b) {
  if(caps == true) {
    Keyboard.print(b);
  } else {
    Keyboard.print(a);
  }
  caps = false;
}

void maybeSpace() {
  if (!star & spacing) Keyboard.write(' ');
}

void layer() { // number key is pressed if we got here
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

int preprocess() {
  if((center == 0x0c) & (left == 0xaa)) {
    maybeSpace(); spit("I"); spacing = true; caps = false; return 1;
  }
  if(center == 0) {
    if(left == 0x14) {
      if(right == 0x14) {Keyboard.write('.'); caps = true; return 1;}
      if(right == 0x28) {Keyboard.write('!'); caps = true; return 1;}
    }
    if(left == 0x28) {
      if(right == 0x28) {Keyboard.write(','); return 1;}
      if(right == 0x14) {Keyboard.write('?'); caps = true; return 1;}
    }
    if(left == 0xf0) { // contractions
      if(right == 0x00) {spit("'"); return 1;}
      if(right == 0x0e) {spit("'d"); return 1;}
      if(right == 0xb0) {spit("'d"); return 1;}
      if(right == 0x40) {spit("'t"); return 1;}
      if(right == 0x80) {spit("'s"); return 1;}
      if(right == 0x06) {spit("'s"); return 1;}
      if(right == 0x2a) {spit("'m"); return 1;}
      if(right == 0x42) {spit("n't"); return 1;}
      if(right == 0x01) {spit("'re"); return 1;}
      if(right == 0x04) {spit("'ll"); return 1;}
      if(right == 0x22) {spit("'ve"); return 1;}
    }
    if((left == 0xa0) & (right == 0x0a)) {
      Keyboard.write(KEY_RETURN);
      spacing = false; return 1;
    }
    if((left == 0x80) & (right == 0x02)) {
      Keyboard.write(KEY_RETURN);
      caps = true; spacing = false; return 1;
    }
    if(left == 0xaa) {
      if(right == 0x16) {spit(" "); spacing = false; return 1;}
      if(right == 0xa8) {caps = true; spacing = false; return 1;}
      if(right == 0xaa) {caps = true; spacing = true; return 1;}
      if(right == 0x28) {caps = false; return 1;}
      if(right == 0x55) {Keyboard.write(KEY_ESC); return 1;}
      if(right == 0x15) {Keyboard.write(KEY_HOME); return 1;}
      if(right == 0x2a) {Keyboard.write(KEY_END); return 1;}
      if(right == 0x26) {Keyboard.write(KEY_PAGE_UP); return 1;}
      if(right == 0x19) {Keyboard.write(KEY_PAGE_DOWN); return 1;}
      if(right == 0x0a) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.write(KEY_LEFT_ARROW);
        Keyboard.release(KEY_LEFT_CTRL);
        return 1;
      }
      if(right == 0x28) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.write(KEY_RIGHT_ARROW);
        Keyboard.release(KEY_LEFT_CTRL);
        return 1;
      }
      if(right == 0x3f) {Keyboard.write(KEY_CAPS_LOCK); return 1;}
    }
    if(right == 0x13) { // rnc open
      if(left == 0x94) {maybeSpace(); spit("("); spacing = false; return 1;}
      if(left == 0xbc) {maybeSpace(); spit("["); spacing = false; return 1;}
      if(left == 0xac) {maybeSpace(); spit("{"); spacing = false; return 1;}
      if(left == 0x40) {maybeSpace(); spit("<"); spacing = false; return 1;}
      if(left == 0xc8) {maybeSpace(); spit("\""); spacing = false; return 1;}
      if(left == 0x08) {maybeSpace(); spit("'"); spacing = false; return 1;}
      if(left == 0x02) {maybeSpace(); spit("*"); spacing = false; return 1;}
    }
    if(right == 0x8c) { // lgs close
      if(left == 0x94) {spit(")"); return 1;} // pr paren
      if(left == 0xbc) {spit("]"); return 1;} // br bracket
      if(left == 0xac) {spit("}"); return 1;} // fr French
      if(left == 0x40) {spit(">"); return 1;} // n angle
      if(left == 0xc8) {spit("\""); return 1;} // q quote
      if(left == 0x08) {spit("'"); return 1;} // t tick
      if(left == 0x02) {spit("*"); return 1;} // star
    }
    if (right == 0x55) {  // rlct
      if (left == 0x08) {spit("~"); return 1;} // Tilde
      if (left == 0x60) {spit("@"); return 1;} // Yat
      if (left == 0x20) {spit("#"); return 1;} // Hash
      if (left == 0x0c) {spit("$"); return 1;} // Dollar
      if (left == 0x14) {spit("%"); return 1;} // Percent
      if (left == 0x84) {spit("^"); return 1;} // CaRat
      if (left == 0x50) {spit("&"); return 1;} // aMpesand
      if (left == 0x02) {spit("*"); return 1;} // Star
      if (left == 0xd4) {spit("+"); return 1;} // PLus
      if (left == 0xc8) {spit("="); return 1;} // eQual
      if (left == 0xc2) {spit("/"); return 1;} // SLash
      if (left == 0x3c) {spit("\\"); return 1;} // Backslash
      if (left == 0xc4) {spit(":"); return 1;} // CoLon
      if (left == 0x06) {spit(";"); return 1;} // SemiColon
      if (left == 0x0e) {spit("`"); return 1;} // Grave
      if (left == 0xcc) {Keyboard.write(KEY_DELETE); return 1;} // DeLete
      if (left == 0x40) {Keyboard.write(KEY_INSERT); return 1;} // iNseRt
      if (left == 0x8c) {spit("_"); return 1;} // unDeRscore
      if (left == 0x2c) {spit("-"); return 1;} // DasH
    }
    if(right == 0xc3) { // left modifiers
      if(left == 0x04) {Keyboard.press(KEY_LEFT_CTRL); return 1;}
      if(left == 0x02) {Keyboard.press(KEY_LEFT_SHIFT); return 1;}
      if(left == 0xc0) {Keyboard.press(KEY_LEFT_ALT); return 1;}
      if(left == 0x0e) {Keyboard.press(KEY_LEFT_GUI); return 1;}
      Keyboard.releaseAll(); return 1;
    }
    if(right == 0xcc) { // right modifiers
      if(left == 0x04) {Keyboard.press(KEY_RIGHT_CTRL); return 1;}
      if(left == 0x02) {Keyboard.press(KEY_RIGHT_SHIFT); return 1;}
      if(left == 0xc0) {Keyboard.press(KEY_RIGHT_ALT); return 1;}
      if(left == 0x0e) {Keyboard.press(KEY_RIGHT_GUI); return 1;}
      Keyboard.releaseAll(); return 1;
    }
    if(left == 0x98) { // function keys
      if(right == 0x26) {Keyboard.write(KEY_TAB); return 1;}
      if(right == 0x01) {Keyboard.write(KEY_F1); return 1;}
      if(right == 0x04) {Keyboard.write(KEY_F2); return 1;}
      if(right == 0x10) {Keyboard.write(KEY_F3); return 1;}
      if(right == 0x40) {Keyboard.write(KEY_F4); return 1;}
      if(right == 0x02) {Keyboard.write(KEY_F5); return 1;}
      if(right == 0x08) {Keyboard.write(KEY_F6); return 1;}
      if(right == 0x20) {Keyboard.write(KEY_F7); return 1;}
      if(right == 0x80) {Keyboard.write(KEY_F8); return 1;}
      if(right == 0x03) {Keyboard.write(KEY_F9); return 1;}
      if(right == 0x0c) {Keyboard.write(KEY_F10); return 1;}
      if(right == 0x30) {Keyboard.write(KEY_F11); return 1;}
      if(right == 0xc0) {Keyboard.write(KEY_F12); return 1;}
    }
  }
  return 0;
}

void sendLeft() {
  switch(left) {
    case 0x02 : {spew("s", "S"); break;}
    case 0x04 : {spew("c", "C"); break;}
    case 0x08 : {spew("t", "T"); break;}
    case 0x10 : {spew("w", "W"); break;}
    case 0x20 : {spew("h", "H"); break;}
    case 0x40 : {spew("n", "N"); break;}
    case 0x80 : {spew("r", "R"); break;}
    case 0x06 : {spew("sc", "Sc"); break;} // sc
    case 0x0a : {spew("st", "St"); break;} // st
    case 0x12 : {spew("sw", "Sw"); break;} // sw
    case 0x22 : {spew("sh", "Sh"); break;} // sh
    case 0x42 : {spew("sn", "Sn"); break;} // sn
    case 0x82 : {spew ("ser", "Ser"); break;} // sr
    case 0x0c : {spew("d", "D"); break;} // ct
    case 0x14 : {spew("p", "P"); break;} // cw
    case 0x24 : {spew("ch", "Ch"); break;} // ch
    case 0x44 : {spew("z", "Z"); break;} // cn
    case 0x84 : {spew("cr", "Cr"); break;} // cr
    case 0x18 : {spew("tw", "Tw"); break;} // tw
    case 0x28 : {spew("th", "Th"); break;} // th
    case 0x48 : {spew("v", "V"); break;} // tn
    case 0x88 : {spew("tr", "Tr"); break;} // tr 
    case 0x30 : {spew("wh", "Wh"); break;} // wh
    case 0x50 : {spew("m", "M"); break;} // wn
    case 0x90 : {spew("wr", "Wr"); break;} // wr
    case 0x60 : {spew("y", "Y"); break;} // hn
    case 0xa0 : {spew("rh", "Rh"); break;} // hr
    case 0xc0 : {spew("l", "L"); break;} // nr
    case 0x0e : {spew("g", "G"); break;} // sct
    case 0x16 : {spew("sp", "Sp"); break;} // scw
    case 0x26 : {spew("sch", "Sch"); break;} // sch
    case 0x46 : {spew("ss", "Ss"); break;} // scn
    case 0x86 : {spew("scr", "Scr"); break;} // scr
    case 0x1a : {spew("x", "X"); break;} // stw
    case 0x4a : {spew("sv", "Sv"); break;} // stn
    case 0x8a : {spew("str", "Str"); break;} // str
    case 0x52 : {spew("sm", "Sm"); break;} // swn
    case 0x62 : {spew("sy", "Sy"); break;} // shn
    case 0xa2 : {spew("shr", "Shr"); break;} // shr
    case 0xc2 : {spew("sl", "Sl"); break;} // snr
    case 0x2c : {spew("f", "F"); break;} // cth
    case 0x1c : {spew("dw", "Dw"); break;} // ctw
    case 0x4c : {spew("dev", "Dev"); break;} // ctn
    case 0x8c : {spew("dr", "Dr"); break;} // ctr
    case 0x34 : {spew("ph", "Ph"); break;} // cwh
    case 0x54 : {spew("pn", "Pn"); break;} // cwn
    case 0x94 : {spew("pr", "Pr"); break;} // cwr
    case 0x64 : {spew("cy", "Cy"); break;} // chn
    case 0xa4 : {spew("chr", "Chr"); break;} // chr
    case 0xc4 : {spew("cl", "Cl"); break;} // cnr
    case 0x38 : {spew("k", "K"); break;} // twh
    case 0x58 : {spew("j", "J"); break;} // twn
    case 0x68 : {spew( "ty", "Ty"); break;}// thn
    case 0xa8 : {spew("thr", "Thr"); break;}// thr
    case 0xc8 : {spew("q", "Q"); break;} // tnr
    case 0x70 : {spew("my", "My"); break;} // whn
    case 0xd0 : {spew("mr", "Mr"); break;} // wnr
    case 0xe0 : {spew("ly", "Ly"); break;} // hnr
    case 0x1e : {spew("gw", "Gw"); break;} // sctw
    case 0x2e : {spew("gh", "Gh"); break;} // scth
    case 0x4e : {spew("gn", "Gn"); break;} // sctn
    case 0x8e : {spew("gr", "Gr"); break;} // sctr
    case 0x36 : {spew("sph", "Sph"); break;} // scwh
    case 0x96 : {spew("spr", "Spr"); break;} // scwr
    case 0x3a : {spew("sk", "Sw"); break;} // stwh
    case 0x9a : {spew("xr", "Xr"); break;} // stwr
    case 0x6a : {spew("sty", "Sty"); break;} // sthn
    case 0xca : {spew("sq", "Sq"); break;} // stnr
    case 0xe2 : {spew("sly", "Sly"); break;} // shnr
    case 0x3c : {spew("b", "B"); break;} // ctwh
    case 0x5c : {spew("dem", "Dem"); break;} // ctwn
    case 0x9c : {spew("der", "Der"); break;} // ctwr
    case 0x6c : {spew("dy", "Dy"); break;} // cthn
    case 0xac : {spew("fr", "Fr"); break;} // cthr
    case 0xcc : {spew("del", "Del"); break;} // ctnr
    case 0x74 : {spew("py", "Py"); break;} // cwhn
    case 0xb4 : {spew("phr", "Phr"); break;} // cwhr
    case 0xd4 : {spew("pl", "Pl"); break;} // cwnr
    case 0xe4 : {spew("cry", "Cry"); break;} // chnr
    case 0x78 : {spew("kn", "Kn"); break;} // twhn
    case 0xb8 : {spew("kr", "Kr"); break;} // twhr
    case 0xd8 : {spew("jer", "Jer"); break;} // twnr
    case 0xe8 : {spew("try", "Try"); break;} // thnr
    case 0x6e : {spew("gy", "Gy"); break;} // scthn
    case 0xce : {spew("gl", "Gl"); break;} // sctnr
    case 0x76 : {spew("spy", "Spy"); break;} // scwhn
    case 0xd6 : {spew("spl", "Spl"); break;} // scwnr
    case 0x7a : {spew("xy", "Xy"); break;} // stwhn
    case 0xda : {spew("serv", "Serv"); break;} // stwnr
    case 0xea : {spew("stry", "Stry"); break;} // sthnr
    case 0x7c : {spew("by", "By"); break;} // ctwhn
    case 0xbc : {spew("br", "Br"); break;} // ctwhr
    case 0xec : {spew("fl", "Fl"); break;} // cthnr
    case 0xf4 : {spew("phl", "Phl"); break;} // cwhnr
    case 0xf8 : {spew("kl", "Kl"); break;} // twhnr
    case 0xfc : {spew("bl", "Bl"); break;} // ctwhnr

    default : ; 
  }
}

void sendCenter() {
  if (number) {
    if (center == 0x01) {spew("ia", "Ia"); return;} // A
    if (center == 0x03) {spew("ao", "Ao"); return;} // AO
    if (center == 0x02) {spew("oo", "Oo"); return;} // O
    if (center == 0x06) {spew("eo", "Eo"); return;} // OE
    if (center == 0x0c) {spew("eu", "Eu"); return;} // EU
    if (center == 0x05) {spew("ae", "Ae"); return;} // AE
    if (center == 0x0a) {spew("uo", "Uo"); return;} // OU
    if (center == 0x0e) {spew("io", "Io"); return;} // OEU
    if (center == 0x07) {spew("ei", "Ei"); return;} // AOE
    if (center == 0x08) {spew("ua", "Ua"); return;} // U
    if (center == 0x04) {spew("ee", "Ee"); return;} // E
  }
  switch(center) {
    case 0x01 : {spew("a", "A"); break;} // A
    case 0x02 : {spew("o", "O"); break;} // O
    case 0x03 : {spew("oa", "Oa"); break;} // AO
    case 0x04 : {spew("e", "E"); break;} // E
    case 0x05 : {spew("ea", "Ea"); break;} // AE
    case 0x06 : {spew("oe", "Oe"); break;} // OE
    case 0x07 : {spew("ie", "Ie"); break;} // AOE
    case 0x08 : {spew("u", "U"); break;} // U
    case 0x09 : {spew("au", "Au"); break;} // AU
    case 0x0a : {spew("ou", "Ou"); break;} // OU
    case 0x0b : {spew("ui", "Ui"); break;} // AOU
    case 0x0c : {spew("i", "I");break;} // EU
    case 0x0d : {spew("ai", "Ai"); break;} // AEU
    case 0x0e : {spew("oi", "Oi"); break;} // OEU
    case 0x0f : {spew("iu", "Iu");} break; // AOEU
    default : ;
  }
}

/*
void sendRight() {
  skipping = false;
  switch(right) {
    case 0x01 : { spit("r"); break;}
    case 0x03 : { spit("rn"); break;}
    case 0x05 : { spit("rl"); break;}
    case 0x09 : { spit("rg"); break;}
    case 0x11 : { spit("rc"); break;}
    case 0x21 : { spit("w"); break;}
    case 0x41 : { spit("rt"); break;}
    case 0x81 : { spit("rs"); break;}
    case 0x02 : { spit("n"); break;}
    case 0x06 : { spit("s"); break;}
    case 0x0a : { spit("ng"); break;}
    case 0x12 : { spit("nc"); break;}
    case 0x22 : { spit("v"); break;}
    case 0x42 : { spit("nt"); break;}
    case 0x82 : { spit("ns"); break;}
    case 0x04 : { spit("l"); break;}
    case 0x0c : {
      if (data[3] & 0x08) {
        spit("logy");
        skipping = true;
      } else {
        spit("lg");
      } break; 
    } // lg
    case 0x14 : { spit("p"); break;}
    case 0x24 : { spit("z"); break;}
    case 0x44 : { spit("lt"); break;}
    case 0x84 : { spit("ls"); break;}
    case 0x08 : { spit("g"); break;}
    case 0x18 : { spit("b"); break;}
    case 0x28 : { spit("gh"); break;}
    case 0x48 : { spit("k"); break;}
    case 0x88 : { spit("gs"); break;}
    case 0x10 : { spit("c"); break;}
    case 0x30 : { spit("ch"); break;}
    case 0x50 : {
      if (data[3] & 0x04) {
        spit("cate");
        skipping = true;
      } else {
        spit("ct");
      }
      break;
    } // ct
    case 0x90 : { spit("cs"); break;}
    case 0x20 : { spit("h"); break;}
    case 0x60 : { spit("th"); break;}
    case 0xa0 : { spit("hs"); break;}
    case 0x40 : { spit("t"); break;}
    case 0xc0 : {
      if (data[3] & 0x08) {
        spit("ys");
        skipping = true;
      } else {
        spit("ts");
      }
      break;
    } // ts
    case 0x80 : { spit("s"); break;}
    case 0x07 : { spit("ll"); break;}
    case 0x0b : { spit("gn"); break;}
    case 0x23 : { spit("wn"); break;}
    case 0x43 : { spit("rnt"); break;}
    case 0x83 : { spit("rns"); break;}
    case 0x15 : { spit("rp"); break;}
    case 0x25 : { spit("wl"); break;}
    case 0x85 : { spit("rls"); break;}
    case 0x19 : { spit("rb"); break;}
    case 0x29 : { spit("rgh"); break;}
    case 0x49 : { spit("rk"); break;}
    case 0x89 : { spit("rgs"); break;}
    case 0x31 : { spit("rch"); break;}
    case 0x91 : { spit("rcs"); break;}
    case 0x61 : { spit("rth"); break;}
    case 0xa1 : { spit("ws"); break;}
    case 0xc1 : { spit("rts"); break;}
    case 0x0e : { spit("d"); break;}  // nlg
    case 0x16 : { spit("sp"); break;}  // nlc
    case 0x26 : { spit("sh"); break;}  // nlh
    case 0x46 : { spit("st"); break;}  // nlt
    case 0x86 : { spit("ss"); break;}  // nls
    case 0x1a : { spit("gg"); break;}  // ngc
    case 0x2a : { spit("m"); break;}  // ngh
    case 0x4a : { spit("nk"); break;}  // ngt
    case 0x8a : { spit("ngs"); break;}  // ngs
    case 0x32 : { spit("nch"); break;}  // nch
    case 0x52 : { spit("tion"); break;}  // nct
    case 0x92 : { spit("nces"); break;}  // ncs
    case 0x62 : { spit("nth"); break;}  // nht
    case 0xa2 : { spit("ves"); break;}  // nhs
    case 0xc2 : { spit("nts"); break;}  // nts
    case 0x1c : { spit("bl"); break;}  // lgc
    case 0x2c : { spit("x"); break;}  // lgh
    case 0x4c : { spit("kl"); break;}  // lgt
    case 0x34 : { spit("ph"); break;}  // lch
    case 0x54 : { spit("pt"); break;}  // lct
    case 0x94 : { spit("ps"); break;}  // lcs
    case 0x64 : { spit("lth"); break;}  // lht
    case 0xa4 : { spit("zes"); break;}  // lhs
    case 0xc4 : { spit("lts"); break;}  // lts
    case 0x38 : { spit("f"); break;}  // gch
    case 0x58 : { spit("ck"); break;}  // gct
    case 0x98 : { spit("bs"); break;}  // gcs
    case 0x68 : { spit("ght"); break;}  // ght
    case 0xa8 : { spit("ghs"); break;}  // ghs
    case 0x1c8 : {spit("kes"); break;} // gtse
    case 0xc8 : {
      if (data[3] & 0x04) {
        spit("kes");
        skipping = true;
      } else {
        spit("ks");
      }
      break;
    }  // gts
    case 0x70 : { spit("tch"); break;}  // cht 
    case 0xb0 : { spit("d"); break;}  // chs 
    case 0xd0 : { spit("cts"); break;}  // cts 
    case 0xe0 : { spit("ths"); break;}  // hts
    case 0xf0 : { spit("dl"); break;}  // rnlg 
    case 0x17 : { spit("pl"); break;}  // rnlc 
    case 0x27 : { spit("lv"); break;}  // rnlh 
    case 0x47 : { spit("rst"); break;}  // rnlt 
    case 0x87 : { spit("lls"); break;}  // rnls
    case 0x2b : { spit("rm"); break;}  // rngh 
    case 0x8b : { spit("gns"); break;}  // rngs 
    case 0x33 : { spit("rv"); break;}  // rnch 
    case 0x63 : { spit("wth"); break;}  // rnht 
    case 0xa3 : { spit("wns"); break;}  // rnhs 
    case 0x95 : { spit("rps"); break;}  // rlcs 
    case 0xa5 : { spit("wls"); break;}  // rlhs 
    case 0x39 : { spit("rf"); break;}  // rgch 
    case 0x99 : { spit("rbs"); break;}  // rgcs 
    case 0x69 : { spit("wk"); break;}  // rght 
    case 0xc9 : { spit("rks"); break;}  // rgts 
    case 0xb1 : { spit("rd"); break;}  // rchs 
    case 0xe1 : { spit("rths"); break;}  // rhts 
    case 0x1e : { spit("ld"); break;}  // nlgc 
    case 0x2e : { spit("sm"); break;}  // nlgh 
    case 0x4e : { spit("sk"); break;}  // nlgt 
    case 0x8e : { spit("ds"); break;}  // nlgs 
    case 0x56 : { spit("nst"); break;}  // nlct 
    case 0x96 : { spit("sps"); break;}  // nlcs 
    case 0xa6 : { spit("shes"); break;}  // nlhs 
    case 0xc6 : { spit("sts"); break;}  // nlts 
    case 0x3a : { spit("mb"); break;}  // ngch 
    case 0x5a : { spit("bt"); break;}  // ngct 
    case 0x9a : { spit("ggs"); break;}  // ngcs 
    case 0x6a : { spit("ngth"); break;}  // nght 
    case 0xaa : { spit("ms"); break;}  // nghs 
    case 0xca : { spit("nks"); break;}  // ngts 
    case 0xb2 : { spit("nd"); break;}  // nchs 
    case 0xe2 : { spit("nths"); break;}  // nhts 
    case 0x3c : { spit("lf"); break;}  // lgch 
    case 0x5c : { spit("ckl"); break;}  // lgct 
    case 0x9c : { spit("bles"); break;}  // lgcs 
    case 0x6c : { spit("xt"); break;}  // lght 
    case 0xac : { spit("xes"); break;}  // lghs 
    case 0x74 : { spit("pth"); break;}  // lcht 
    case 0xb4 : { spit("phs"); break;}  // lchs 
    case 0x78 : { spit("ft"); break;}  // gcht 
    case 0xb8 : { spit("dg"); break;}  // gchs 
    case 0xd8 : { spit("cks"); break;}  // gcts 
    case 0xe8 : { spit("ghts"); break;}  // ghts 
    case 0x0f : { spit("ds"); break;}  // chts
    case 0x1f : { spit("lb"); break;}  // rnlgc
    case 0x2f : { spit("lm"); break;}  // rnlgh
    case 0x4f : { spit("lk"); break;}  // rnlgt
    case 0x37 : { spit("lch"); break;}  // rnlch
    case 0x57 : { spit("lp"); break;}  // rnlct
    case 0x97 : { spit("ples"); break;}  // rnlcs
    case 0xa7 : { spit("lves"); break;}  // rnlhs
    case 0xc7 : { spit("rsts"); break;}  // rnlts
    case 0xab : { spit("rms"); break;}  // rnghs
    case 0xb3 : { spit("wd"); break;}  // rnchs
    case 0xe3 : { spit("wths"); break;}  // rnhts
    case 0xad : { spit("rld"); break;}  // rlghs
    case 0xe9 : { spit("wks"); break;}  // rghts
    case 0xf1 : { spit("rds"); break;}  // rchts
    case 0x3e : { spit("mp"); break;}  // nlgch
    case 0x9e : { spit("lds"); break;}  // nlgcs
    case 0x6e : { spit("dth"); break;}  // nlght
    case 0xae : { spit("sms"); break;}  // nlghs
    case 0xce : { spit("sks"); break;}  // nlgts
    case 0xba : { spit("mbs"); break;}  // ngchs
    case 0xda : { spit("bts"); break;}  // ngcts
    case 0xea : { spit("ngths"); break;}  // nghts
    case 0xf2 : { spit("nds"); break;}  // nchts
    case 0xdc : { spit("ckles"); break;}  // lgcts
    case 0xf4 : { spit("pths"); break;}  // lchts
    case 0x9f : { spit("lbs"); break;}  // rnlgcs
    case 0xaf : { spit("lms"); break;}  // rnlghs
    case 0xcf : { spit("lks"); break;}  // rnlgts
    case 0xb7 : { spit("ld"); break;}  // rnlchs
    case 0xd7 : { spit("lps"); break;}  // rnlcts
    case 0xf3 : { spit("wds"); break;}  // rnchts
    case 0xf5 : { spit("rlds"); break;}  // rlchts
    case 0x7e : { spit("mpt"); break;}  // nlgcht
    case 0xbe : { spit("mps"); break;}  // nlgchs
    case 0xee : { spit("dths"); break;}  // nlghts
    case 0xf7 : { spit("lds"); break;}  // rnlchts
    case 0xfe : { spit("mpts"); break;}  // nlgchts
  }
}
*/

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
    delay(20); // debounce the key, so to speak?
    if(data[2] == 0x29) {moving(KEY_BACKSPACE); spacing = false; return 1;}
    if(data[2] == 0x02) {moving(KEY_LEFT_ARROW); return 1;}
    if(data[2] == 0x04) {moving(KEY_UP_ARROW); return 1;}
    if(data[2] == 0x08) {moving(KEY_DOWN_ARROW); return 1;}
    if(data[2] == 0x20) {moving(KEY_RIGHT_ARROW); return 1;}
  }
  return 0;
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

// right hand strings
const char r0[] PROGMEM = "";
const char r1[] PROGMEM = "r";
const char r2[] PROGMEM = "n";
const char r3[] PROGMEM = "rn";
const char r4[] PROGMEM = "l";
const char r5[] PROGMEM = "rl";
const char r6[] PROGMEM = "nl";
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
const char r30[] PROGMEM = "";
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
const char r158[] PROGMEM = "";
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
const char r191[] PROGMEM = "";
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

void emit(int ind) {
  char buffer[10];
  strcpy_P(buffer, (char*)pgm_read_word(&(right_side[ind])));
  Keyboard.print(buffer);
}

void run() {
  scan(); if(leaving) return;
  organize();
  if(star & !right & !left & !center) {
    Keyboard.write(KEY_BACKSPACE); spacing = false; return;
  }
  if ((number) & (!star) & (!center)) {
    layer(); return;
  }
  if (preprocess()) return;
  maybeSpace();
  sendLeft();
  sendCenter();
//  sendRight();
//  spit(right_side[right]);
  emit(right);
  if (!skipping) {
    if (data[3] & 0x04) spit("e");
    if (data[3] & 0x08) spit("y");
  }
  spacing = true;
}

void loop() {
  run();
}

