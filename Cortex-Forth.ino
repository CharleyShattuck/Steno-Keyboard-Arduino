/*
  Forth virtual machine

  This code is in the public domain.

*/

#define RAM_SIZE 0x1000
#define S0 0x1000
#define R0 0x0f00 
#define M(a, b) {memory [a] = b;}
#define NAME(m, f, c, x, y, z) {memory [m] = f + c + (x << 8) + (y << 16) + (z << 24);}
#define LINK(m, a) {memory [m] = a;}
#define CODE(m, a) {memory [m] = a;}
#define LIST(m, a) {memory [m] = a;}

// global variables
union Memory {
  int data [RAM_SIZE];
  void (*code []) (void);
} memory;

String tib = "";
int S = S0; // data stack pointer
int R = R0; // return stack pointer
int I = 0; // instruction pointer
int W = 0; // working register
int T = 0; // top of stack
int H = 0; // dictionary pointer, HERE
int D = 0; // dictionary list entry point
int base = 10;

/*  A word in the dictionary has these fields:
  link  32b  point to next word in list, 0 says end of list
  name  32b  a 32 bit int, made up of byte count and three letters
  code  32b  a pointer to some actually C compiled code,
        all native code is in this field
  data  32b at least  a list to execute or a data field of some kind


*/


// primitive definitions


void _NOP (void) {
  return;
}

void _EXIT (void) {
  I = memory.data [R++];
}

void _DROP (void) {
  T = memory.data [S++];
}

void _DUP (void) {
  memory.data [--S] = T;
}

void _KEY (void) {
  _DUP ();
  while (!Serial.available ());
  T = Serial.read ();
}

void _EMIT (void) {
  char c = T;
  Serial.write (c);
  _DROP ();
}

void _CR (void) {
  Serial.println (" ");
}

void _OK (void) {
  if (tib [tib.length () - 1] == 10) Serial.println (" Ok");
}

void _SWAP (void) {
  W = memory.data [S];
  memory.data [S] = T;
  T = W;
}

void _OVER (void) {
  _DUP ();
  T = memory [S - 1];
}

void _FETCH (void) {
  T = memory.data [T];
}

void _STORE (void) {
  W = T,
  _DROP ();
  memory.data [W] = T;
  _DROP ();
}

void _COMMA (void) {
  memory.data [H++] = T;
  _DROP ();
}

void _MINUS (void) {
  W = T;
  _DROP ();
  T = (T - W);
}

void _PLUS (void) {
  W = T;
  T = (T + W);
}

void _AND (void) {
  W = T;
  _DROP ();
  T = (T & W);
}

void _OR (void) {
  W = T;
  _DROP ();
  T = (T | W);
}

void _XOR (void) {
  W = T;
  _DROP ();
  T = (T ^ W);
}

void _INVERT (void) {
  T = ~T;
}

void _ABS (void) {
  T = abs (T);
}

void _NEGATE (void) {
  T = -T;
}

void _TWOSLASH (void) {
  T = (T >> 1);
}

void _TWOSTAR (void) {
  T = (T << 1);
}

// unlinked primitives

void _LIT (void) {
  memory.data [--S] = T
  T = memory.data [I++];
}

void _ BRANCH (void) {
  I = memory.data [I];
}

void _0BRANCH (void) {
  if (T == 0) {
    I = memory [I];
    _DROP (); 
    return;
  }
  I += 1;
  _DROP ();
}

void _INITR (void) {
  R = R0;
}

void _INITS (void) {
  S = S0;
}

void _DOCOL (void) {

}

void _CREATE (void) {

}

void _CONSTANT (void) {

}

void _VARIABLE (void) {

}

void _DO (void) {

}

void _LOOP (void) {

}

void _SHOWTIB (void) {

}

// trim leading spaces
void _PARSE (void) {
  char t;
  tib = "";
  do {
    while (!Serial.available ());
    t = Serial.peek ();
    if (t == ' ') t = Serial.read ();
  } while (t == ' ');
  do {
    while (!Serial.available ());
    t = Serial.read ();
    tib = tib + t;
  } while (t > ' ');
}

void _WORD (void) {

}

void _NUMBER (void) {

}

void _EXECUTE (void) {

}


// do, loop
// docol, doconst, dovar

// inner interpreter

void setup () {

  S = S0; // initialize data stack
  R = R0; // initialize return stack

// initialize dictionary

// unlinked primitives
  CODE(0, _LIT)
  CODE(1, _BRANCH)
  CODE(2, _0BRANCH)
  CODE(3, _DO)
  CODE(4, _LOOP)
  CODE(5, _INITR
  CODE(6, _INITS
  CODE(7, _SHOWTIB)
  CODE(8, _OK)

// trailing space kludge
  NAME(10, 0, 0, 10, 0, 0)
  LINK(11, 0)
  CODE(12, _NOP)
// exit
  NAME(13, 0, 4, 'e', 'x', 'i')
  LINK(14, 10)
  CODE(15, _EXIT)
// key
  NAME(16, 0, 3, 'k', 'e', 'y')
  LINK(17, 13)
  CODE(18, _KEY)
// emit
  NAME(19, 0, 4, 'e', 'm', 'i')
  LINK(20, 16)
  CODE(21, _EMIT)
// cr
  NAME(22, 0, 2, 'c', 'r', 0) 
  LINK(23, 19)
  CODE(24, _CR)
// parse
  NAME(25, 0, 5, 'p', 'a', 'r')
  LINK(26, 22)
  CODE(27, _PARSE)
// word
  NAME(28, 0, 4, 'w', 'o', 'r')
  LINK(29, 25)
  CODE(30, _WORD)

// dup
  NAME(24, 0, 3, 'd', 'u', 'p')
  LINK(25, 18)
  CODE(26, _DUP)
// drop
  NAME(27, 0, 4, 'd', 'r', 'o')
  LINK(28, 21)
  CODE(29, _DROP)
// swap
  NAME(30, 0, 4, 's', 'w', 'a')
  LINK(31, 25)
  CODE(32, _SWAP)
// over
  NAME(33, 0, 4, 'o', 'v', 'e')
  LINK(34, 29)
  CODE(35, _OVER)
// @
  NAME(36, 0, 1, '@', 0, 0)
  LINK(37, 33)
  CODE(38, _FETCH)
// !
  NAME(39, 0, 1, '!', 0, 0)
  LINK(40, 37)
  CODE(41, _STORE)
// ,
  NAME(42, 0, 1, ',', 0, 0)
  LINK(43, 41)
  CODE(44, _COMMA)
// find
  NAME(137, 0, 4, 'f', 'i', 'n')
  LINK(138, 133)
  CODE(139, _FIND)
// execute
  NAME(141, 0, 7, 'e', 'x', 'e')
  LINK(142, 137)
  CODE(143, _EXECUTE


// ?dup
  NAME(149, 0, 3, '?', 'd', 'u')
  LINK(150, 145)
  CODE(151, _QDUP)
// number
  NAME(149, 0, 6, 'n', 'u', 'm')
  LINK(150, 145)
  CODE(151, _NUMBER)

// quit
  NAME(45, 0, 4, 'q', 'u', 'i')
  LINK(46, 181)
  CODE(47, _DOCOL)    // docol
  // begin begin
  LIST(48, 9)         // initr
  // begin
  LIST(49, 27)        // parse
  LIST(50, 30)        // word
  LIST(51, _FIND)     // find
  LIST(52, _QDUP)     // ?dup
  // while (if)
  LIST(53, 2)         // 0branch
  LIST(54, 198)
  LIST(55, _EXECUTE)  // execute
  LIST(56, 8)         // ok
  // repeat
  LIST(57, 1)         // branch
  LIST(58, 188)
  // (then)
  LIST(59, _NUMBER)   // number
  LIST(60, 2)         // 0branch
  LIST(61, 195)
  LIST(62, 7)         // showtib
  LIST(63, _LIT)      // lit
  LIST(64, '?')
  LIST(65, _EMIT)     // emit
  LIST(66, _CR)       // cr
  LIST(67, 6)         // inits
  // again
  LIST(68, 1)         // branch
  LIST(69, 187)
// abort 
  NAME(70, 0, 5, 'a', 'b', 'o')
  LINK(71, 185)
  CODE(72, _DOCOL)
  LIST(72, 5)         // inits
  // again
  LIST(73, 1)         // branch
  LIST(69, 187)

// +
  NAME(49, 0, 1, '+', 0, 0)
  LINK(50, 45)
  CODE(51, _PLUS)
// -
  NAME(53, 0, 1, '-', 0, 0)
  LINK(54, 49)
  CODE(55, _MINUS)
// and
  NAME(57, 0, 3, 'a', 'n', 'd')
  LINK(58, 53)
  CODE(59, _AND)
// or 
  NAME(61, 0, 2, 'o', 'r', 0)
  LINK(62, 57)
  CODE(63, _OR)
// xor 
  NAME(65, 0, 3, 'x', 'o', 'r')
  LINK(66, 61)
  CODE(67, _XOR)
// invert 
  NAME(69, 0, 6, 'i', 'n', 'v')
  LINK(70, 65)
  CODE(71, _INVERT)
// abs
  NAME(73, 0, 3, 'a', 'b', 's')
  LINK(74, 69)
  CODE(75, _ABS)
// negate
  NAME(77, 0, 6, 'n', 'e', 'g')
  LINK(78, 73)
  CODE(79, _NEGATE)
// 2*
  NAME(81, 0, 2, '2', '*', 0)
  LINK(82, 77)
  CODE(83, _TWOSTAR)
// 2/
  NAME(85, 0, 2, '2', '/', 0)
  LINK(86, 81)
  CODE(87, _TWOSLASH)

  D = 0; // latest word
  H = 0; // top of dictionary

  I = 0; // instruction pointer = abort

  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex");
//  _words ();
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.code [W] ();
//  delay (100);
}

