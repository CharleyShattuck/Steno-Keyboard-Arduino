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

void layer() {  // this changes if we make an A key
  if (left & 0x02) Keyboard.write('1');
  if (left & 0x04) Keyboard.write('2');
  if (left & 0x10) Keyboard.write('3');
  if (left & 0x40) Keyboard.write('4');
  if (center & 0x01) Keyboard.write('5');
  if (center & 0x02) Keyboard.write('0');
  if (right &  0x01) Keyboard.write('6');
  if (right &  0x04) Keyboard.write('7');
  if (right & 0x10) Keyboard.write('8');
  if (right & 0x40) Keyboard.write('9');
  return;
}

int preprocess() {
  if(center == 0) {
    if(left == 0x14) {
      if(right == 0x14) {Keyboard.write('.'); caps = true; return 1;}
      if(right == 0x28) {Keyboard.write('!'); caps = true; return 1;}
    }
    if(left == 0x28) {
      if(right == 0x28) {Keyboard.write(','); return 1;}
      if(right == 0x14) {Keyboard.write('?'); caps = true; return 1;}
    }
    if(left == 0xaa) {
      if(right == 0x2a) {caps = true; spacing = false; return 1;}
      if(right == 0xaa) {caps = true; spacing = true; return 1;}
      if(right == 0x55) {Keyboard.write(KEY_ESC); return 1;}
    }
    if(left == 0x2a) {
      if(right == 0x40) {spit("'t"); return 1;}
      if(right == 0x80) {spit("'s"); return 1;}
      if(right == 0x2a) {spit("'m"); return 1;}
      if(right == 0x42) {spit("n't"); return 1;}
      if(right == 0x2c) {spit("'d"); return 1;}
      if(right == 0x01) {spit("'re"); return 1;}
    }
    if((left == 0xa0) & (right == 0x0a)) {
      Keyboard.write(KEY_RETURN); caps = true; return 1;
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
  switch(center) {
    case 0x01 : {spew("a", "A"); break;}
    case 0x02 : {spew("o", "O"); break;}
    case 0x04 : {spew("e", "E"); break;}
    case 0x08 : {spew("u", "U"); break;}
    case 0x0d : {spew("ai", "Ai"); break;} 
    case 0x09 : {spew("au", "Au"); break;}
    case 0x05 : {spew("ea", "Ea"); break;}
    case 0x07 : {spew("ee", "Ee"); break;}
    case 0x0c : {
      if (!left & !right) {
        spit("I"); caps = false;
      } else {
        spew("i", "I");
      }
      break;
    }
    case 0x06 : {spew("oe", "Oe"); break;}
    case 0x03 : {spew("io", "Io"); break;}
    case 0x0e : {spew("oi", "Oi"); break;}
    case 0x0b : {spew("oo", "Oo"); break;}
    case 0x0a : {spew("ou", "Ou"); break;}
    case 0x0f : {spew("ie", "Ie"); break;
    default : ;
    }
  }
}

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
      if (data[3] == 0x08) {
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
      if (data[3] == 0x04) {
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
      if (data[3] == 0x08) {
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
      if (data[3] == 0x04) {
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
    case 0xfd : { spit("mpts"); break;}  // rlgchts
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

// allow key repeat for some keys
void moving(byte a) {
  Keyboard.press(a);
  do {look();} while(pressed);
  Keyboard.release(a);
}

boolean movement() {
  if((data[0] == 0) & (data[1] == 0x08) & (data[2] == 0) & (data[3] == 0)) {
    moving(KEY_BACKSPACE); return 1;
  }
  if((data[0] == 0x15) & (data[1] == 0x01) & (data[3] == 0)) {
    if(data[2] == 0x02) {
      moving(KEY_LEFT_ARROW); return 1;
    }
    if(data[2] == 0x04) {
      moving(KEY_UP_ARROW); return 1;
    }
    if(data[2] == 0x08) {
      moving(KEY_DOWN_ARROW); return 1;
    }
    if(data[2] == 0x20) {
      moving(KEY_RIGHT_ARROW); return 1;
    }
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
    if (movement()) { digitalWrite(13, LOW); leaving = true; return;}
  } while(pressed);
  digitalWrite(13, LOW);
}

void run() {
  scan(); if(leaving) return;
  organize();
  if (number) {layer(); return;}
  if (preprocess()) return;
  if (star & number & !right & !center & !left) {
    caps = true; return;
  }
  maybeSpace();
  sendLeft();
  sendCenter();
  sendRight();
  if (!skipping) {
    if (data[3] & 0x04) spit("e");
    if (data[3] & 0x08) spit("y");
  }
  spacing = true;
}

void loop() {
  run();
}

