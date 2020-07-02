// This example program is in the public domain
#include <Keyboard.h>
#include <Wire.h>

const byte i2c1 = 0x20;
const byte i2c2 = 0x21;

boolean pressed;

// parts of the keyboard
byte left = 0;
byte center = 0;
byte right = 0;
int controller = 0;
boolean number = false;
boolean caps = false;
boolean ekey = false;
boolean ykey = false;
boolean spacing = false;
boolean autospace = true;

int read_raw_keys () {
  int a=0;
  a |= digitalRead (A1);       // 10
  a |= digitalRead (A2) << 1;  // 20
  a |= digitalRead (A3) << 2;  // 40
  a |= digitalRead (A4) << 3;  // 80
  a ^=0x0f;
  return a;
}

// read the i2c port expander with the given address
unsigned int read_AB (int address) {
  Wire.beginTransmission (address);
  Wire.write (0x12);  // GPIOA
  Wire.endTransmission ();
  Wire.requestFrom (address, 2);
  unsigned int a = Wire.read ();
  int b = Wire.read ();
  a |= b << 8;
  a ^= 0xffff;
  a &= 0xffff;
  return a;
}

// global variables keep track of keys stroked
unsigned int raw_keys=0;
unsigned int serial_keys1=0;
unsigned int serial_keys2=0;

// read and accumulate keys until all released
void scan () {
  int a = 0; int b = 0; int c = 0;
  serial_keys1 = 0;
  serial_keys2 = 0;
  raw_keys = 0;
  do {
    do {
      a = read_AB (i2c1 );
      b = read_raw_keys ();
      c = read_AB (i2c2);
    } while ((a == 0) & (b == 0) & (c == 0));  // not pressed
    delay (30);
    a = read_AB (i2c1);
    b = read_raw_keys ();
    c = read_AB (i2c2);
  } while ((a == 0) & (b == 0) & (c == 0));  // not pressed
  do {
    a = read_AB (i2c1);
    b = read_raw_keys ();
    c = read_AB (i2c2);
    serial_keys1 |= a;
    raw_keys |= b;
    serial_keys2 |= c;
  } while ((a != 0) | (b != 0) | (c != 0));  // pressed
}

// translate scan into a hardware neutral representation
void organize(){
  left = 0; // initial consonants
  if (serial_keys2 & 0x2000) left |= 0x01; // a
  if (serial_keys2 & 0x1000) left |= 0x04; // c
  if (serial_keys2 & 0x0800) left |= 0x10; // w
  if (serial_keys1 & 0x8000) left |= 0x40; // n
  if (serial_keys2 & 0x0004) left |= 0x02; // s
  if (serial_keys2 & 0x0008) left |= 0x08; // t
  if (serial_keys2 & 0x0010) left |= 0x20; // h
  if (serial_keys1 & 0x0001) left |= 0x80; // r
  center = 0; // vowels
  if (raw_keys & 0x01) center |= 0x01; // i
  if (raw_keys & 0x02) center |= 0x02; // e
  if (raw_keys & 0x04) center |= 0x04; // a
  if (raw_keys & 0x08) center |= 0x08; // o
  if ( serial_keys2 & 0x0200) center |= 0x10; // u
  right = 0; // final consonants
  if ( serial_keys1 & 0x2000) right |= 0x01; // r
  if ( serial_keys1 & 0x1000) right |= 0x04; // l
  if ( serial_keys1 & 0x0800) right |= 0x10; // c
  if ( serial_keys1 & 0x0400) right |= 0x40; // t
  if ( serial_keys1 & 0x0004) right |= 0x02; // n
  if ( serial_keys1 & 0x0008) right |= 0x08; // g
  if ( serial_keys1 & 0x0010) right |= 0x20; // h
  if ( serial_keys1 & 0x0020) right |= 0x80; // s
  ekey = false; if ( serial_keys1 & 0x0200) ekey = true;
  ykey = false; if ( serial_keys1 & 0x0040) ykey = true;
  spacing = false;
  if (serial_keys1 & 0x4000) spacing = true; // *
  if (serial_keys1 & 0x0002) spacing = true; // *
  controller = 0; // 5 keys beyond the basic TinyMod
  if (serial_keys2 & 0x0020) controller |= 0x01;
  if (serial_keys2 & 0x0040) controller |= 0x02;
  if (serial_keys2 & 0x0100) controller |= 0x04;
  if (serial_keys2 & 0x0400) controller |= 0x08;
  number = false;
  if (serial_keys2 & 0x0080) number = true;
} 

// optionally capitalize first character of the string
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
const char l2a[] PROGMEM = "'"; // sth
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
const char l68[] PROGMEM = "ty"; // thn
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
const char l8b[] PROGMEM = "astr"; // astr 
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
const char l98[] PROGMEM = "qu"; // twr
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
  l88, l89, l8a, l8b, l8c, l8d, l8e, l8f,
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

// lookup string using key stroke as index and print it
void sendLeft() {
  char buffer[10];
  strcpy_P(buffer, (char*)pgm_read_word(&(left_side[left])));
  spit(buffer);
}

void sendCenter () {
  if (center & 0x01) {
    if ((controller & 0x01) | (controller & 0x02)) {
      spit ("u");
    } else {
      spit ("i");
    }
  }
  if (center & 0x02) {
    if ((controller & 0x01) | (controller & 0x02)) {
      spit ("o");
    } else {
      spit ("e");
    }
  }
  if (center & 0x04) spit ("a");
  if (center & 0x08) {
    if ((controller & 0x04) | (controller & 0x08)) {
      spit ("e");
    } else { 
      spit ("o");
    }
  }
  if (center & 0x10) {
    if ((controller & 0x04) | (controller & 0x08)) {
      spit ("i");
    } else {
      spit ("u");
    }
  }
}

// right hand strings
const char r00[] PROGMEM = "";
const char r01[] PROGMEM = "r";
const char r02[] PROGMEM = "n";
const char r03[] PROGMEM = "rn";
const char r04[] PROGMEM = "l";
const char r05[] PROGMEM = "rl";
const char r06[] PROGMEM = "s";
const char r07[] PROGMEM = "ll"; // rnl
const char r08[] PROGMEM = "g";
const char r09[] PROGMEM = "rg";
const char r0a[] PROGMEM = "ng"; 
const char r0b[] PROGMEM = "gn"; // rng
const char r0c[] PROGMEM = "lg";
const char r0d[] PROGMEM = "";
const char r0e[] PROGMEM = "d"; // npg
const char r0f[] PROGMEM = "dl"; // rnlg
const char r10[] PROGMEM = "c";
const char r11[] PROGMEM = "rc";
const char r12[] PROGMEM = "nc";
const char r13[] PROGMEM = "";
const char r14[] PROGMEM = "p"; // lc
const char r15[] PROGMEM = "rp"; // rlc
const char r16[] PROGMEM = "sp"; // nlc
const char r17[] PROGMEM = "pl"; // rnlc
const char r18[] PROGMEM = "b"; // gc
const char r19[] PROGMEM = "rb";
const char r1a[] PROGMEM = "gg";
const char r1b[] PROGMEM = "";
const char r1c[] PROGMEM = "bl";
const char r1d[] PROGMEM = "";
const char r1e[] PROGMEM = "ld"; // CWS 25Jan20
const char r1f[] PROGMEM = "lb";
const char r20[] PROGMEM = "h";
const char r21[] PROGMEM = "w";
const char r22[] PROGMEM = "v";
const char r23[] PROGMEM = "wn";
const char r24[] PROGMEM = "z";
const char r25[] PROGMEM = "wl";
const char r26[] PROGMEM = "sh";
const char r27[] PROGMEM = "lv";
const char r28[] PROGMEM = "gh";
const char r29[] PROGMEM = "rgh";
const char r2a[] PROGMEM = "m";
const char r2b[] PROGMEM = "rm";
const char r2c[] PROGMEM = "x";
const char r2d[] PROGMEM = "";
const char r2e[] PROGMEM = "sm";
const char r2f[] PROGMEM = "lm";
const char r30[] PROGMEM = "ch";
const char r31[] PROGMEM = "rch";
const char r32[] PROGMEM = "nch";
const char r33[] PROGMEM = "rv";
const char r34[] PROGMEM = "ph";
const char r35[] PROGMEM = "";
const char r36[] PROGMEM = "";
const char r37[] PROGMEM = "lch";
const char r38[] PROGMEM = "f";
const char r39[] PROGMEM = "rf";
const char r3a[] PROGMEM = "mb";
const char r3b[] PROGMEM = "";
const char r3c[] PROGMEM = "lf";
const char r3d[] PROGMEM = "";
const char r3e[] PROGMEM = "mp";
const char r3f[] PROGMEM = "";
const char r40[] PROGMEM = "t";
const char r41[] PROGMEM = "rt";
const char r42[] PROGMEM = "nt";
const char r43[] PROGMEM = "rnt";
const char r44[] PROGMEM = "lt";
const char r45[] PROGMEM = "";
const char r46[] PROGMEM = "st";
const char r47[] PROGMEM = "rst";
const char r48[] PROGMEM = "k";
const char r49[] PROGMEM = "rk";
const char r4a[] PROGMEM = "nk";
const char r4b[] PROGMEM = "";
const char r4c[] PROGMEM = "kl";
const char r4d[] PROGMEM = "";
const char r4e[] PROGMEM = "sk";
const char r4f[] PROGMEM = "lk";
const char r50[] PROGMEM = "ct";
const char r51[] PROGMEM = "";
const char r52[] PROGMEM = "tion";
const char r53[] PROGMEM = "ction"; // CWS 25Jan20
const char r54[] PROGMEM = "pt";
const char r55[] PROGMEM = "";
const char r56[] PROGMEM = "nst";
const char r57[] PROGMEM = "lp";
const char r58[] PROGMEM = "ck";
const char r59[] PROGMEM = "";
const char r5a[] PROGMEM = "bt";
const char r5b[] PROGMEM = "";
const char r5c[] PROGMEM = "ckl";
const char r5d[] PROGMEM = "";
const char r5e[] PROGMEM = "";
const char r5f[] PROGMEM = "";
const char r60[] PROGMEM = "th";
const char r61[] PROGMEM = "rth";
const char r62[] PROGMEM = "nth";
const char r63[] PROGMEM = "wth";
const char r64[] PROGMEM = "lth";
const char r65[] PROGMEM = "";
const char r66[] PROGMEM = "";
const char r67[] PROGMEM = "";
const char r68[] PROGMEM = "ght";
const char r69[] PROGMEM = "wk";
const char r6a[] PROGMEM = "ngth";
const char r6b[] PROGMEM = "";
const char r6c[] PROGMEM = "xt";
const char r6d[] PROGMEM = "";
const char r6e[] PROGMEM = "dth";
const char r6f[] PROGMEM = "";
const char r70[] PROGMEM = "tch";
const char r71[] PROGMEM = "";
const char r72[] PROGMEM = "";
const char r73[] PROGMEM = "";
const char r74[] PROGMEM = "pth";
const char r75[] PROGMEM = "";
const char r76[] PROGMEM = "";
const char r77[] PROGMEM = "";
const char r78[] PROGMEM = "ft";
const char r79[] PROGMEM = "";
const char r7a[] PROGMEM = "";
const char r7b[] PROGMEM = "";
const char r7c[] PROGMEM = "";
const char r7d[] PROGMEM = "";
const char r7e[] PROGMEM = "mpt";
const char r7f[] PROGMEM = "";
const char r80[] PROGMEM = "s";
const char r81[] PROGMEM = "rs";
const char r82[] PROGMEM = "ns";
const char r83[] PROGMEM = "rns";
const char r84[] PROGMEM = "ls";
const char r85[] PROGMEM = "rls";
const char r86[] PROGMEM = "ss";
const char r87[] PROGMEM = "lls";
const char r88[] PROGMEM = "gs";
const char r89[] PROGMEM = "rgs";
const char r8a[] PROGMEM = "ngs";
const char r8b[] PROGMEM = "gns";
const char r8c[] PROGMEM = "";
const char r8d[] PROGMEM = "";
const char r8e[] PROGMEM = "ds";
const char r8f[] PROGMEM = "";
const char r90[] PROGMEM = "cs";
const char r91[] PROGMEM = "rcs";
const char r92[] PROGMEM = "nces";
const char r93[] PROGMEM = "";
const char r94[] PROGMEM = "ps";
const char r95[] PROGMEM = "rps";
const char r96[] PROGMEM = "sps";
const char r97[] PROGMEM = "ples";
const char r98[] PROGMEM = "bs";
const char r99[] PROGMEM = "rbs";
const char r9a[] PROGMEM = "ggs";
const char r9b[] PROGMEM = "";
const char r9c[] PROGMEM = "bles";
const char r9d[] PROGMEM = "";
const char r9e[] PROGMEM = "lds";
const char r9f[] PROGMEM = "lbs";
const char ra0[] PROGMEM = "hs";
const char ra1[] PROGMEM = "ws";
const char ra2[] PROGMEM = "ves";
const char ra3[] PROGMEM = "wns";
const char ra4[] PROGMEM = "zes";
const char ra5[] PROGMEM = "wls";
const char ra6[] PROGMEM = "shes";
const char ra7[] PROGMEM = "lves";
const char ra8[] PROGMEM = "ghs";
const char ra9[] PROGMEM = "";
const char raa[] PROGMEM = "ms";
const char rab[] PROGMEM = "rms";
const char rac[] PROGMEM = "xes";
const char rad[] PROGMEM = "";
const char rae[] PROGMEM = "sms";
const char raf[] PROGMEM = "lms";
const char rb0[] PROGMEM = "d";
const char rb1[] PROGMEM = "rd";
const char rb2[] PROGMEM = "nd";
const char rb3[] PROGMEM = "wd";
const char rb4[] PROGMEM = "phs";
const char rb5[] PROGMEM = "rld";
const char rb6[] PROGMEM = "";
const char rb7[] PROGMEM = "ld";
const char rb8[] PROGMEM = "dg";
const char rb9[] PROGMEM = "";
const char rba[] PROGMEM = "mbs";
const char rbb[] PROGMEM = "";
const char rbc[] PROGMEM = "";
const char rbd[] PROGMEM = "";
const char rbe[] PROGMEM = "mps";
const char rbf[] PROGMEM = "dd";
const char rc0[] PROGMEM = "ts";
const char rc1[] PROGMEM = "rts";
const char rc2[] PROGMEM = "nts";
const char rc3[] PROGMEM = "";
const char rc4[] PROGMEM = "lts";
const char rc5[] PROGMEM = "";
const char rc6[] PROGMEM = "sts";
const char rc7[] PROGMEM = "rsts";
const char rc8[] PROGMEM = "ks";
const char rc9[] PROGMEM = "rks";
const char rca[] PROGMEM = "nks";
const char rcb[] PROGMEM = "";
const char rcc[] PROGMEM = "";
const char rcd[] PROGMEM = "";
const char rce[] PROGMEM = "sks";
const char rcf[] PROGMEM = "lks";
const char rd0[] PROGMEM = "cts";
const char rd1[] PROGMEM = "";
const char rd2[] PROGMEM = "";
const char rd3[] PROGMEM = "";
const char rd4[] PROGMEM = "";
const char rd5[] PROGMEM = "";
const char rd6[] PROGMEM = "";
const char rd7[] PROGMEM = "lps";
const char rd8[] PROGMEM = "cks";
const char rd9[] PROGMEM = "";
const char rda[] PROGMEM = "bts";
const char rdb[] PROGMEM = "";
const char rdc[] PROGMEM = "ckles";
const char rdd[] PROGMEM = "";
const char rde[] PROGMEM = "";
const char rdf[] PROGMEM = "";
const char re0[] PROGMEM = "ths";
const char re1[] PROGMEM = "rths"; 
const char re2[] PROGMEM = "nths";
const char re3[] PROGMEM = "wths";
const char re4[] PROGMEM = "";
const char re5[] PROGMEM = "";
const char re6[] PROGMEM = "";
const char re7[] PROGMEM = "";
const char re8[] PROGMEM = "ghts";
const char re9[] PROGMEM = "wks";
const char rea[] PROGMEM = "ngths";
const char reb[] PROGMEM = "";
const char rec[] PROGMEM = "";
const char red[] PROGMEM = "";
const char ree[] PROGMEM = "dths";
const char ref[] PROGMEM = "";
const char rf0[] PROGMEM = "ds";
const char rf1[] PROGMEM = "rds";
const char rf2[] PROGMEM = "nds";
const char rf3[] PROGMEM = "wds";
const char rf4[] PROGMEM = "pths";
const char rf5[] PROGMEM = "rlds";
const char rf6[] PROGMEM = "";
const char rf7[] PROGMEM = "lds";
const char rf8[] PROGMEM = "";
const char rf9[] PROGMEM = "";
const char rfa[] PROGMEM = "";
const char rfb[] PROGMEM = "";
const char rfc[] PROGMEM = "";
const char rfd[] PROGMEM = "";
const char rfe[] PROGMEM = "mpts";
const char rff[] PROGMEM = "";

const char* const right_side[] PROGMEM = {
  r00, r01, r02, r03, r04, r05, r06, r07,
  r08, r09, r0a, r0b, r0c, r0d, r0e, r0f,
  r10, r11, r12, r13, r14, r15, r16, r17,
  r18, r19, r1a, r1b, r1c, r1d, r1e, r1f,
  r20, r21, r22, r23, r24, r25, r26, r27,
  r28, r29, r2a, r2b, r2c, r2d, r2e, r2f,
  r30, r31, r32, r33, r34, r35, r36, r37,
  r38, r39, r3a, r3b, r3c, r3d, r3e, r3f,
  r40, r41, r42, r43, r44, r45, r46, r47,
  r48, r49, r4a, r4b, r4c, r4d, r4e, r4f,
  r50, r51, r52, r53, r54, r55, r56, r57,
  r58, r59, r5a, r5b, r5c, r5d, r5e, r5f,
  r60, r62, r62, r63, r64, r65, r66, r67,
  r68, r69, r6a, r6b, r6c, r6d, r6e, r6f,
  r70, r71, r72, r73, r74, r75, r76, r77,
  r78, r79, r7a, r7b, r7c, r7d, r7e, r7f,
  r80, r81, r82, r83, r84, r85, r86, r87,
  r88, r89, r8a, r8b, r8c, r8d, r8e, r8f,
  r90, r91, r92, r93, r94, r95, r96, r97,
  r98, r99, r9a, r9b, r9c, r9d, r9e, r9f,
  ra0, ra2, ra2, ra3, ra4, ra5, ra6, ra7,
  ra8, ra9, raa, rab, rac, rad, rae, raf,
  rb0, rb1, rb2, rb3, rb4, rb5, rb6, rb7,
  rb8, rb9, rba, rbb, rbc, rbd, rbe, rbf,
  rc0, rc1, rc2, rc3, rc4, rc5, rc6, rc7,
  rc8, rc9, rca, rcb, rcc, rcd, rce, rcf,
  rd0, rd1, rd2, rd3, rd4, rd5, rd6, rd7,
  rd8, rd9, rda, rdb, rdc, rdd, rde, rdf,
  re0, re2, re2, re3, re4, re5, re6, re7,
  re8, re9, rea, reb, rec, red, ree, ref,
  rf0, rf1, rf2, rf3, rf4, rf5, rf6, rf7,
  rf8, rf9, rfa, rfb, rfc, rfd, rfe, rff,
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
// lookup string using stroke as index and print
  strcpy_P(buffer, (char*)pgm_read_word(&(right_side[right])));
  spit(buffer);
  if (ekey) spit("e");
  if (ykey) spit("y");
}

// if we're here then the number key was pressed
void numbers () {
//  if (serial_keys2 & 0x0100) {
  if (controller == 0x04) {
    if (right & 0x40) spit ("9");
    if (right & 0x10) spit ("8");
    if (right & 0x04) spit ("7");
    if (right & 0x01) spit ("6");
    if (center & 0x02) spit ("0");
    if (center & 0x01) spit ("5");
    if (left & 0x40) spit ("4");
    if (left & 0x10) spit ("3");
    if (left & 0x04) spit ("2");
    if (left & 0x01) spit ("1");
  } else {
    if (left & 0x01) spit ("1");
    if (left & 0x04) spit ("2");
    if (left & 0x10) spit ("3");
    if (left & 0x40) spit ("4");
    if (center & 0x01) spit ("5");
    if (center & 0x02) spit ("0");
    if (right & 0x01) spit ("6");
    if (right & 0x04) spit ("7");
    if (right & 0x10) spit ("8");
    if (right & 0x40) spit ("9");
  }
}

void unspace () {
  Keyboard.press (KEY_LEFT_CTRL);
  Keyboard.write (KEY_LEFT_ARROW);
  Keyboard.release (KEY_LEFT_CTRL);
  Keyboard.write (KEY_BACKSPACE);
  Keyboard.press (KEY_LEFT_CTRL);
  Keyboard.write (KEY_RIGHT_ARROW);
  Keyboard.release (KEY_LEFT_CTRL);
}

void run() {
  scan ();
  organize ();
//                   ...x     not o or u      
  if ((controller == 0x08) & ((center & 0x18) == 0)) {
    caps = true; 
  }
//                   .xx.     no vowels
  if ((controller == 0x06) & (center == 0)) {
    autospace = !autospace;
    return;
  }
//                   x..x     no vowels
  if ((controller == 0x09) & (center == 0)) Keyboard.write (KEY_CAPS_LOCK);
//  if (spacing & number) {
  if (spacing & !number & (center == 0) & (right == 0)) {
    if (left == 0xaa) {
      Keyboard.write (KEY_DELETE); return;
    }
    if (left == 0) {
      Keyboard.write (KEY_BACKSPACE); return;
    }
  }
  if (number & (left == 0) & (right == 0)
     & (center == 0) & (controller == 0)) {
    Keyboard.print (" ");
  }
  if (number) {
    numbers ();
    return;
  }
  if ((center == 0) & (left == 0xa0) & (right == 0x0a)) {
    Keyboard.write (KEY_RETURN); return;
  }
  if ((center == 0) & (left == 0x50) & (right == 0x05)) {
    Keyboard.write (KEY_TAB); return;
  }
  if ((left == 0x55) & (right == 0x55)) {
    Keyboard.write (KEY_ESC); return;
  }
  if (left == 0xaa) {
    if ((right == 0) & (center == 0) & !number & !spacing) {
      Keyboard.releaseAll (); return;
    }
    if (right == 0x2a) {unspace (); return;}
    if (right == 0x0a) {
      Keyboard.press (KEY_LEFT_CTRL);
      Keyboard.write (KEY_LEFT_ARROW);
      Keyboard.release (KEY_LEFT_CTRL);
      return;
    }
    if (right == 0x28) {
      Keyboard.press (KEY_LEFT_CTRL);
      Keyboard.write (KEY_RIGHT_ARROW);
      Keyboard.release (KEY_LEFT_CTRL);
      return;
    }
    if (center == 0x01) Keyboard.press (KEY_LEFT_SHIFT);
    if (center == 0x02) Keyboard.press (KEY_LEFT_CTRL);
    if (center == 0x08) Keyboard.press (KEY_LEFT_ALT);
    if (center == 0x10) Keyboard.press (KEY_LEFT_GUI);
    if (spacing & number) {
      Keyboard.write (KEY_DELETE);
      return;
    }
    if (right == 0x01) {
      Keyboard.write (KEY_HOME);
      return;
    }
    if (right == 0x40) {
      Keyboard.write (KEY_END);
      return;
    }
    if (right == 0x04) {
      Keyboard.write (KEY_PAGE_UP);
      return;
    }
    if (right == 0x10) {
      Keyboard.write (KEY_PAGE_DOWN);
      return;
    }
    if (right == 0x02) {
      Keyboard.write (KEY_LEFT_ARROW);
//      Keyboard.releaseAll ();
      return;
    }
    if (right == 0x08) {
      Keyboard.write (KEY_UP_ARROW);
//      Keyboard.releaseAll ();
      return;
    }
    if (right == 0x20) {
      Keyboard.write (KEY_DOWN_ARROW);
//      Keyboard.releaseAll ();
      return;
    }
    if (right == 0x80) {
      Keyboard.write (KEY_RIGHT_ARROW);
//      Keyboard.releaseAll ();
      return;
    }
    return;
  }
  if (right == 0x55) {
    if (left == 0x90) spit ("(");
    if (left == 0xb0) spit ("[");
    if (left == 0x94) spit ("{");
    if (left == 0x98) spit ("<");
    if (left == 0x03) spit (":");
    if (left == 0x0c) spit ("\"");
    if (left == 0x30) spit ("|");
    if (left == 0x05) spit ("@");
    if (left == 0x0a) spit ("$");
    if (left == 0x50) spit ("-");
    if (left == 0xa0) spit ("^");
    if (left == 0x5a) spit ("/");
    if (left == 0xc0) spit ("+");
    if (left == 0xf0) spit ("*");
    if (left == 0x01) spit (".");
    if (left == 0x04) spit (",");
    if (left == 0x10) spit ("?");
    if (left == 0x40) spit ("!");
    return;
  }
  if (right == 0xa9) {
    if (left == 0x90) spit (")");
    if (left == 0xb0) spit ("]");
    if (left == 0x94) spit ("}");
    if (left == 0x98) spit (">");
    if (left == 0x03) spit (";");
    if (left == 0x0c) spit ("\'");
    if (left == 0x30) spit ("~");
    if (left == 0x05) spit ("#");
    if (left == 0x0a) spit ("%");
    if (left == 0x50) spit ("_");
    if (left == 0xa0) spit ("&");
    if (left == 0x5a) spit ("\\");
    if (left == 0xc0) spit ("=");
    return;
  }
  if ((left == 0x14) & (center == 0)) {
    if (right == 0x14) {spit ("."); caps = true; return;}
    if (right == 0x28) {spit ("!"); caps = true; return;}
  }
  if ((left == 0x28) & (center == 0)) {
    if (right == 0x14) {spit ("?"); caps = true; return;}
    if (right == 0x28) {spit (","); return;}
  }
//  if ((spacing) & (center == 0) & (left == 0) &
//      (right == 0) & (controller == 0) & (!ekey) & (!ykey)) {
//    Keyboard.print (" ");
//    return;
//  }
  if (((spacing) & (!autospace)) | ((!spacing) & (autospace))) {
    Keyboard.print (" ");
//    spacing = false;
  }
  sendLeft(); 
  sendCenter();
  sendRight();
}

void setup() {
  Serial.begin (9600);
  Keyboard.begin ();
// raw pins
  pinMode (A1, INPUT_PULLUP);
  pinMode (A2, INPUT_PULLUP);
  pinMode (A3, INPUT_PULLUP);
  pinMode (A4, INPUT_PULLUP);
// port expander pins
  Wire.begin ();
  Wire.beginTransmission (i2c1);
  Wire.write (0x0c); //GPPUA
  Wire.write (0xff);
  Wire.write (0xff);
  Wire.endTransmission ();
  Wire.begin ();
  Wire.beginTransmission (i2c2);
  Wire.write (0x0c); //GPPUA
  Wire.write (0xff);
  Wire.write (0xff);
  Wire.endTransmission ();
  delay (3000);
}

void loop () {
  run ();
}
