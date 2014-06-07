#include <avr/pgmspace.h>

/* Tiny interpreter,
   similar to myforth's Standalone Interpreter
   This example code is in the public domain */

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
#define NAMED(x, y) PROGMEM const char x[]=y

/* Terminal Input Buffer for interpreter */
const byte maxtib = 16;
char tib[maxtib];
/* buffer required for strings read from flash */
char namebuf[maxtib];
byte pos;

/* push n to top of data stack */
void push(int n) {
  p=(p+1)&STKMASK;
  TOS=n;
}

/* return top of stack */
int pop() {
  int n=TOS;
  p=(p-1)&STKMASK;
  return n;
}

/* Global delay timer */
int spd = 15;

/* top of stack becomes current spd */
NAMED(_speed, "speed");
void speed() { spd=pop();}

/* discard top of stack */
NAMED(_drop, "drop");
void drop() { pop();}

/* recover dropped stack item */
NAMED(_back, "back");
void back() { for(int i=1; i<STKSIZE; i++) drop();}

/* copy top of stack */
NAMED(_dup, "dup");
void dup() { push(TOS);}

/* exchange top two stack items */
NAMED(_swap, "swap");
void swap() {
  int a;
  int b;
  a = pop();
  b = pop();
  push(a);
  push(b);
}

/* copy second on stack to top */
NAMED(_over, "over");
void over() {
  int a;
  int b;
  a = pop();
  b = pop();
  push(b);
  push(a);
  push(b);
}
/* add top two items */
NAMED(_add, "+");
void add() {
  int a = pop();
  TOS=a+TOS;
}

/* bitwise and top two items */
NAMED(_and, "and");
void and_() {
  int a = pop();
  TOS=a&TOS;
}

/* inclusive or top two items */
NAMED(_or, "or");
void or_() {
  int a = pop();
  TOS=a|TOS;
}

/* exclusive or top two items */
NAMED(_xor, "xor");
void xor_() {
  int a = pop();
  TOS=a^TOS;
}

/* invert all bits in top of stack */
NAMED(_invert, "invert");
void invert() { TOS=~(TOS);}

/* negate top of stack */
NAMED(_negate, "negate");
void negate() { TOS=-(TOS);}

/* destructively display top of stack, decimal */
NAMED(_dot, ".");
void dot() {
  Serial.print(pop());
  Serial.print(" ");
}

/* destructively display top of stack, hex */
NAMED(_dotHEX, ".h");
void dotHEX() {
  Serial.print(0xffff & pop(), HEX);
  Serial.print(" ");
}

/* display whole stack, hex */
NAMED(_dotShex, ".sh");
void dotShex() {
  for(int i=0; i<STKSIZE; i++) dotHEX();
} 

/* display whole stack, decimal */
NAMED(_dotS, ".s");
void dotS() { for(int i=0; i<STKSIZE; i++) dot();}

/* delay TOS # of milliseconds */
NAMED(_delay, "delay");
void del() { delay(pop());}

/* Toggle pin at TOS and delay(spd), repeat... */
NAMED(_wiggle, "wiggle");
void wiggle() {
  int a=pop();
  pinMode(a, OUTPUT);
  for(int i=0; i<20; i++) {
    digitalWrite(a, HIGH);
    delay(100);
    digitalWrite(a, LOW);
    delay(100);
  }
}

/* TOS is pin number, set it HIGH */
NAMED(_high, "high");
void high() { digitalWrite(pop(), HIGH);}

/* set TOS pin LOW */
NAMED(_low, "low");
void low() { digitalWrite(pop(), LOW);}

/* read TOS pin */
NAMED(_in, "in");
void in() { TOS=digitalRead(TOS);}

/* make TOS pin an input */
NAMED(_input, "input");
void input() { pinMode(pop(), INPUT);}

/* make TOS pin an output */
NAMED(_output, "output");
void output() { pinMode(pop(), OUTPUT);}

/* make TOS pin an input with weak pullup */
NAMED(_input_pullup, "input_pullup");
void input_pullup() { pinMode(pop(), INPUT_PULLUP);}

/* read flash byte at TOS address */
NAMED(_fetchp, "@p");
void fetchp() { TOS=pgm_read_byte_near(TOS);}

/* dump 16 bytes of flash in hex with ascii on the side */
void dumpFLASH() {
  char buffer[5]="";
  int p=pop();
  sprintf(buffer, "%4x", p);
  Serial.print(buffer);
  Serial.print("  ");
  for(int i=0; i<16; i++) {
    sprintf(buffer, " %2x", pgm_read_byte_near(p++));
    Serial.print(buffer);
  }
  p=p-16;
  Serial.print("   ");
  for(int i=0; i<16; i++) {
    buffer[0]=pgm_read_byte_near(p++);
    if(buffer[0]>0x7f || buffer[0]<' ') buffer[0]='.';
    buffer[1]='\0';
    Serial.print(buffer);
  }
  push(p);
}

/* dump 256 bytes of flash */
NAMED(_dumps, "d");
void dumps() {
  for(int i=0; i<16; i++) {
    Serial.println();
    dumpFLASH();
  }
}

/* dump 16 bytes of RAM in hex with ascii on the side */
void dumpRAM() {
  char buffer[5]="";
  char *ram;
  int p=pop();
  ram = (char*)p;
  sprintf(buffer, "%4x", p);
  Serial.print(buffer);
  Serial.print("   ");
  for(int i=0; i<16; i++) {
    char c=*ram++;
    sprintf(buffer, " %2x", (c&0xff));
    Serial.print(buffer);
  }
  ram = (char*)p;
  Serial.print("   ");
  for(int i=0; i<16; i++) {
    buffer[0]=*ram++;
    if(buffer[0]>0x7f || buffer[0]<' ') buffer[0]='.';
    buffer[1]='\0';
    Serial.print(buffer);
  }
  push(p+16);
}

/* dump 256 bytes of RAM */
NAMED(_dumpr, "r");
void rdumps() {
  for(int i=0; i<16; i++) {
    Serial.println();
    dumpRAM();
  }
}

/* End of Forth interpreter words */
/* ******************************************** */
/* Beginning of application words */




/* End of application words */
/* ******************************************** */
/* Now build the dictionary */

/* empty words don't cause an error */
NAMED(_nop, " ");
void nop() { }

/* Forward declaration required here */
NAMED(_words, "words");
void words();

/* table of names and function addresses in flash */
const entry dictionary[] = {
  {_nop, nop},
  {_words, words},
  {_dup, dup},
  {_drop, drop},
  {_back, back},
  {_swap, swap},
  {_over, over},
  {_add, add},
  {_and, and_},
  {_or, or_},
  {_xor, xor_},
  {_invert, invert},
  {_negate, negate},
  {_dotS, dotS},
  {_dotShex, dotShex},
  {_dot, dot},
  {_dotHEX, dotHEX},
  {_delay, del},
  {_high, high},
  {_low, low},
  {_in, in},
  {_input, input},
  {_output, output},
  {_input_pullup, input_pullup},
  {_wiggle, wiggle},
  {_fetchp, fetchp},
  {_dumps, dumps},
  {_dumpr, rdumps},
  {_speed, speed}
};

/* Number of words in the dictionary */
const int entries = sizeof dictionary / sizeof dictionary[0];

/* Display all words in dictionary */
void words() {
  for(int i=entries-1; i>=0; i--) {
    strcpy_P(namebuf, dictionary[i].name);
    Serial.print(namebuf);
    Serial.print(" ");
   }
}

/* Find a word in the dictionary, returning its position */
int locate() {
  for(int i=entries; i>=0; i--) {
    strcpy_P(namebuf, dictionary[i].name);
    if(!strcmp(tib, namebuf)) return i;
  }
  return 0;
}

/* Is the word in tib a number? */
int isNumber() {
  char *endptr;
  strtol(tib, &endptr, 0);
  if(endptr == tib) return 0;
  if(*endptr != '\0') return 0;
  return 1;
}

/* Convert number in tib */
int number() {
  char *endptr;
  return (int) strtol(tib, &endptr, 0);
}

char ch;

void ok() { if(ch == '\r') Serial.println("ok");}

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
  tib[0]=0;
  while(reading());
  Serial.print(tib);
  Serial.print(" ");
}

/* Run a word via its name */
void runword() {
  int place = locate();
  if(place != 0) {
    dictionary[place].function();
    ok();
    return;
  }
  if(isNumber()) {
    push(number());
    ok();
    return;
  }
  Serial.println("?");
}

/* Arduino main loop */

void setup() {
  Serial.begin(115200);
  Serial.println("Tiny Interpreter:");
  words();
  Serial.println();
}

void loop() {
  readword();
  runword();
}
