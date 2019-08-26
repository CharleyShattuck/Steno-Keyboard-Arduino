/* Tiny interpreter,
   similar to myforth's Standalone Interpreter
   This example code is in the public domain */

#include <Keyboard.h>
#include <Wire.h>

/* Structure of a dictionary entry */
typedef struct {
  const char *name;
  void (*function)();
} entry;

/* Data stack for parameter passing
   This "stack" is circular,
   like a Green Arrays F18A data stack,
   so overflow and underflow are not possible
   Number of items must be a power of 2 */
const int STKSIZE = 8;
const int STKMASK = 7;
int stack[STKSIZE];
int p = 0;

/* TOS is Top Of Stack */
#define TOS stack[p]
/* NAMED creates a string in flash */
#define NAMED(x, y) const char x[] PROGMEM = y

/* Terminal Input Buffer for interpreter */
const byte maxtib = 16;
char tib[maxtib];
/* buffer required for strings read from flash */
char namebuf[maxtib];
byte pos;

/* push n to top of data stack */
void PUSH(int n) {
  p = (p + 1)& STKMASK;
  TOS = n;
}

/* return top of stack */
int POP() {
  int n = TOS;
  p = (p - 1)& STKMASK;
  return n;
}

/* Global delay timer */
int spd = 15;

/* top of stack becomes current spd */
NAMED(_speed, "speed");
void speed() {
  spd = POP();
}

/* discard top of stack */
NAMED(_drop, "drop");
void DROP() {
  POP();
}

/* recover dropped stack item */
NAMED(_back, "back");
void BACK() {
  for (int i = 1; i < STKSIZE; i++) DROP();
}

/* copy top of stack */
NAMED(_dup, "dup");
void DUP() {
  PUSH(TOS);
}

/* exchange top two stack items */
NAMED(_swap, "swap");
void SWAP() {
  int a;
  int b;
  a = POP();
  b = POP();
  PUSH(a);
  PUSH(b);
}

/* copy second on stack to top */
NAMED(_over, "over");
void OVER() {
  int a;
  int b;
  a = POP();
  b = POP();
  PUSH(b);
  PUSH(a);
  PUSH(b);
}
/* add top two items */
NAMED(_add, "+");
void ADD() {
  int a = POP();
  TOS = a + TOS;
}

/* bitwise and top two items */
NAMED(_and, "and");
void AND() {
  int a = POP();
  TOS = a & TOS;
}

/* inclusive or top two items */
NAMED(_or, "or");
void OR() {
  int a = POP();
  TOS = a | TOS;
}

/* exclusive or top two items */
NAMED(_xor, "xor");
void XOR() {
  int a = POP();
  TOS = a ^ TOS;
}

/* invert all bits in top of stack */
NAMED(_invert, "invert");
void INVERT() {
  TOS = ~(TOS);
}

/* negate top of stack */
NAMED(_negate, "negate");
void NEGATE() {
  TOS = -(TOS);
}

/* destructively display top of stack, decimal */
NAMED(_dot, ".");
void dot() {
  Serial.print(POP());
  Serial.print(F(" "));
}

/* destructively display top of stack, hex */
NAMED(_dotHEX, ".h");
void dotHEX() {
  Serial.print(0xffff & POP(), HEX);
  Serial.print(" ");
}

/* display whole stack, hex */
NAMED(_dotShex, ".sh");
void dotShex() {
  for (int i = 0; i < STKSIZE; i++){
    BACK(); dotHEX(); BACK();
  } 
}

/* display whole stack, decimal */
NAMED(_dotS, ".s");
void dotS() {
  for (int i = 0; i < STKSIZE; i++){
    BACK(); dot(); BACK();
  }
}

/* delay TOS # of milliseconds */
NAMED(_delay, "ms");
void MS() {
  delay(POP());
}

/* Toggle pin at TOS and delay(spd), repeat... */
NAMED(_wiggle, "wiggle");
void wiggle() {
  int a = POP();
  pinMode(a, OUTPUT);
  for (int i = 0; i < 20; i++) {
    digitalWrite(a, HIGH);
    delay(100);
    digitalWrite(a, LOW);
    delay(100);
  }
}

/* TOS is pin number, set it HIGH */
NAMED(_high, "high");
void high() {
  digitalWrite(POP(), HIGH);
}

/* set TOS pin LOW */
NAMED(_low, "low");
void low() {
  digitalWrite(POP(), LOW);
}

/* read TOS pin */
NAMED(_in, "in");
void in() {
  TOS = digitalRead(TOS);
}

/* make TOS pin an input */
NAMED(_input, "input");
void input() {
  pinMode(POP(), INPUT);
}

/* make TOS pin an output */
NAMED(_output, "output");
void output() {
  pinMode(POP(), OUTPUT);
}

/* make TOS pin an input with weak pullup */
NAMED(_input_pullup, "input_pullup");
void input_pullup() {
  pinMode(POP(), INPUT_PULLUP);
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
  char buffer[5] = "";
  char *ram;
  int p = POP();
  ram = (char*)p;
  sprintf(buffer, "%4x", p);
  Serial.print(buffer);
  Serial.print(F("   "));
  for (int i = 0; i < 16; i++) {
    char c = *ram++;
    sprintf(buffer, " %2x", (c & 0xff));
    Serial.print(buffer);
  }
  ram = (char*)p;
  Serial.print(F("   "));
  for (int i = 0; i < 16; i++) {
    buffer[0] = *ram++;
    if (buffer[0] > 0x7f || buffer[0] < ' ') buffer[0] = '.';
    buffer[1] = '\0';
    Serial.print(buffer);
  }
  PUSH(p + 16);
}

/* dump 256 bytes of RAM */
NAMED(_dumpr, "dump");
void rdumps() {
  for (int i = 0; i < 16; i++) {
    Serial.println();
    dumpRAM();
  }
}

/* End of Forth interpreter words */
/* ******************************************** */
/* Beginning of application words */

// the slider switch
#define PROTOCOL 7

NAMED(_init, "init");
void INIT() {
// init the raw pins
  pinMode(7, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
// init the port expander pins
  Wire.begin ();
  Wire.beginTransmission (0x20);
  Wire.write (0x0c); // GPPUA
  Wire.write (0xff);
  Wire.write (0xff);
  Wire.endTransmission ();  
}

NAMED(_key, "key");
void KEY() {
  PUSH (digitalRead(POP()));
}

NAMED(_readraw, "rawkeys");
void read_raw_keys () {
  int a = 0;
  a |= digitalRead (9);       // 1
  a |= (digitalRead (10) << 1); // 2
  a |= (digitalRead (11) << 2); // 4
  a |= (digitalRead (12) << 3); // 8
  a |= (digitalRead (A1) << 4); // 10
  a |= (digitalRead (A2) << 5); // 20
  a |= (digitalRead (A3) << 6); // 40
  a |= (digitalRead (A4) << 7); // 80
  a |= (digitalRead (A5) << 8); // 100
  a ^= 0x01ff;
  PUSH (a);
}

NAMED(_readAB, "readAB");
void read_AB () {
  Wire.beginTransmission (0x20);
  Wire.write (0x12); // GPIOA
  Wire.endTransmission ();
  Wire.requestFrom (0x20, 2);
  int a = Wire.read ();
  int b = Wire.read ();
  a |= b << 8;
  a ^= 0xffff;
  PUSH (a);
}

boolean pressed = false;

NAMED(_readall, "readall");
void read_all () {
  int a = 0;
  pressed = true;
  read_raw_keys ();
  read_AB ();
  OVER (); OVER (); OR ();
  a = POP ();
  if (a == 0) pressed = false;
}

NAMED(_scan, "scan");
void scan () {
  int a = 0; int b = 0;
  do {
    do {
      read_all ();
    } while (!pressed);
    delay (30);
    read_all ();
  } while (!pressed);
  a = 0; b = 0;
  do {
    read_all ();
    b |= POP ();
    a |= POP ();
  } while (pressed);
  PUSH (a); PUSH (b);
}

NAMED(_test, "test");
void test () {
  while (true) {
    scan (); dotShex (); Serial.println ();
  }
}

boolean first = true;

void spit (char a) {
  Keyboard.press (a); delay (2);
  if (first == true) {
    first = false;
  } else {
    Keyboard.release (a);
  }
}

void send_NKRO () {
  int b = POP ();
  int a = POP ();
  first = true;

  if (a & 0x10) spit ('q');
  if (a & 0x20) spit ('w');
  if (a & 0x40) spit ('e');
  if (a & 0x80) spit ('r');
  if (a & 0x100) spit ('t');

  if (b & 0x8000) spit ('u');
  if (b & 0x4000) spit ('i');
  if (b & 0x2000) spit ('o');
  if (b & 0x1000) spit ('p');
  if (b & 0x100) spit ('[');

  if (a & 0x08) spit ('a');
  if (a & 0x04) spit ('s');
  if (a & 0x02) spit ('d');
  if (a & 0x01) spit ('f');
  if (b & 0x200) spit ('g');

  if (b & 0x01) spit ('j');
  if (b & 0x02) spit ('k');
  if (b & 0x04) spit ('l');
  if (b & 0x800) spit (';');
  if (b & 0x400) spit ('\'');

  if (b & 0x08) spit ('c');
  if (b & 0x10) spit ('v');
  if (b & 0x20) spit ('3');
  if (b & 0x40) spit ('n');
  if (b & 0x80) spit ('m');

  Keyboard.releaseAll ();
}

void init_NKRO () {
  Keyboard.begin (); delay (3000);
}

void NKRO () {
  init_NKRO ();
  while (1) {
    scan ();
    send_NKRO ();
  }
}


/* End of application words */
/* ******************************************** */
/* Now build the dictionary */


/* empty words don't cause an error */
NAMED(_nop, " ");
void NOP() { }

/* Forward declaration required here */
NAMED(_words, "words");
void WORDS();

/* table of names and function addresses in flash */
const entry dictionary[] = {
  {_nop, NOP},
  {_words, WORDS},
  {_dup, DUP},
  {_drop, DROP},
  {_back, BACK},
  {_swap, SWAP},
  {_over, OVER},
  {_add, ADD},
  {_and, AND},
  {_or, OR},
  {_xor, XOR},
  {_invert, INVERT},
  {_negate, NEGATE},
  {_dotS, dotS},
  {_dotShex, dotShex},
  {_dot, dot},
  {_dotHEX, dotHEX},
  {_delay, MS},
  {_high, high},
  {_low, low},
  {_in, in},
  {_input, input},
  {_output, output},
  {_input_pullup, input_pullup},
  {_wiggle, wiggle},
  {_dumpr, rdumps},
  {_init, INIT},
  {_key, KEY},
  {_readraw, read_raw_keys},
  {_readAB, read_AB},
  {_readall, read_all},
  {_scan, scan},
  {_test, test}
};

/* Number of words in the dictionary */
const int entries = sizeof dictionary / sizeof dictionary[0];

/* Display all words in dictionary */
void WORDS() {
  for (int i = entries - 1; i >= 0; i--) {
    strcpy_P(namebuf, dictionary[i].name);
    Serial.print(namebuf);
    Serial.print(F(" "));
    if ((i % 5) == 0) Serial.println();
  }
}

/* Find a word in the dictionary, returning its position */
int locate() {
  for (int i = entries - 1; i >= 0; i--) {
    strcpy_P(namebuf, dictionary[i].name);
    if(strcmp(tib, namebuf) == 0) return i;
  }
  return 0;
}

/* Is the word in tib a number? */
int isNumber() {
  char *endptr;
  strtol(tib, &endptr, 0);
  if (endptr == tib) return 0;
  if (*endptr != '\0') return 0;
  return 1;
}

/* Convert number in tib */
int number() {
  char *endptr;
  return (int) strtol(tib, &endptr, 0);
}

char ch;

void ok() {
  if (ch == '\r') Serial.println(F("ok"));
}

/* Incrementally read command line from serial port */
byte reading() {
  if (!Serial.available()) return 1;
  ch = Serial.read();
  if (ch == '\n') return 1;
  if (ch == '\r') return 0;
  if (ch == ' ') return 0;
  if (pos < maxtib) {
    tib[pos++] = ch;
    tib[pos] = 0;
  }
  return 1;
}

/* Block on reading the command line from serial port */
/* then echo each word */
void readword() {
  pos = 0;
  tib[0] = 0;
  while (reading());
  Serial.print(tib);
  Serial.print(F(" "));
}

/* Run a word via its name */
void runword() {
  int place = locate();
  if (place != 0) {
    dictionary[place].function();
    ok();
    return;
  }
  if (isNumber()) {
    PUSH(number());
    ok();
    return;
  }
  Serial.println(F("?"));
}

void init_interpreter () {
  Serial.begin(9600); delay (3000);
  while(!Serial);
  Serial.println(F("Forth-like interpreter:"));
  WORDS(); 
  Serial.println();
}

void interpret () {
  init_interpreter ();
  while (1) {
    readword ();
    runword ();
  }
}


/* Arduino main loop */

void setup() {
  INIT();
  if (digitalRead (PROTOCOL)) {
    interpret ();
  } else {
    NKRO ();
  }
}

// not using "loop", but Arduino IDE wants it to exist
void loop() {
  while (true);
}

