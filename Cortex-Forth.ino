/*
  Forth virtual machine

  This code is in the public domain.

*/

#define RAM_SIZE 0x1000
#define S0 0x1000
#define R0 0x0f00 
#define NAME(m, f, c, x, y, z) {memory.data [m] = f + c + (x << 8) + (y << 16) + (z << 24);}
#define LINK(m, a) {memory.data [m] = a;}
#define CODE(m, a) {memory.program [m] = a;}
#define LIST(m, a) {memory.data [m] = a;}

// global variables
union Memory {
  int data [RAM_SIZE];
  void (*program []) (void);
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

void _QDUP (void) {
  if (T) _DUP ();
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
  T = memory.data [S - 1];
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
  _DROP ();
  T = (T + W);
}

void _aND (void) {
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
  memory.data [--S] = T;
  T = memory.data [I++];
}

void _BRANCH (void) {
  I = memory.data [I];
}

void _0BRANCH (void) {
  if (T == 0) {
    I = memory.data [I];
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

void _NEST (void) {
  memory.data [--R] = I;
  I = (W + 1);
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
  W = tib.length ();
  tib [W - 1] = 0;
  Serial.print (tib);
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
  char t;
  _DUP ();
  T = (tib.length () - 1);
  W = T;
  t = tib [0];
  T |= (t << 8);
  if (W > 1) {
    t = tib [1];
    T |= (t << 16);
  }
  if (W < 2) {
    t = tib [2];
     T |= (t << 24);
  }
}

void _NUMBER (void) {
  char t;
  _DUP ();
  T = 0;
  for (int i = 0; i < (tib.length () -1); i++) {
    if (i == 0) {
      if (tib [i] == '-') continue;
    }
    t = tib [i];
    if (!isDigit (t)) {
      if (tib [0] == '-') T = -T;
      memory.data [--S] = -1;
      return;
    }
    T *= base;
    t -= '0';
    if (t > 9) t -= 37;
    T += t;
  }
  if (tib [0] == '-') T = -T;
  memory.data [--S] = 0;
}

void _EXECUTE (void) {
  memory.data [--R] = I;
  I = (T + 2);
}

void _FIND (void) {
  W = T;
  T = D;
  while (T != 0) {
    T = (memory.data [T]);
    if (T == W) {
      return;
    }
  }
}

// do, loop
// docol, doconst, dovar

// inner interpreter

void setup () {

  S = S0; // initialize data stack
  R = R0; // initialize return stack

// initialize dictionary

// unlinked primitives
  CODE(1, _LIT)
  CODE(2, _BRANCH)
  CODE(3, _0BRANCH)
//  CODE(4, _DO)
//  CODE(5, _LOOP)
  CODE(6, _INITR)
  CODE(7, _INITS)
  CODE(8, _SHOWTIB)
  CODE(9, _OK)
// room to expand here

// words with dictionary links

// trailing space kludge
  NAME(20, 0, 0, 10, 0, 0)
  LINK(21, 0)
  CODE(22, _NOP)
// exit
  NAME(23, 0, 4, 'e', 'x', 'i')
  LINK(24, 20)
  CODE(25, _EXIT)
// key
  NAME(26, 0, 3, 'k', 'e', 'y')
  LINK(27, 23)
  CODE(28, _KEY)
// emit
  NAME(29, 0, 4, 'e', 'm', 'i')
  LINK(30, 26)
  CODE(31, _EMIT)
// cr
  NAME(32, 0, 2, 'c', 'r', 0) 
  LINK(33, 29)
  CODE(34, _CR)
// parse
  NAME(35, 0, 5, 'p', 'a', 'r')
  LINK(36, 32)
  CODE(37, _PARSE)
// word
  NAME(38, 0, 4, 'w', 'o', 'r')
  LINK(39, 35)
  CODE(40, _WORD)
// dup
  NAME(41, 0, 3, 'd', 'u', 'p')
  LINK(42, 38)
  CODE(43, _DUP)
// drop
  NAME(44, 0, 4, 'd', 'r', 'o')
  LINK(45, 41)
  CODE(46, _DROP)
// swap
  NAME(47, 0, 4, 's', 'w', 'a')
  LINK(48, 44)
  CODE(49, _SWAP)
// over
  NAME(50, 0, 4, 'o', 'v', 'e')
  LINK(51, 47)
  CODE(52, _OVER)
// @
  NAME(53, 0, 1, '@', 0, 0)
  LINK(54, 50)
  CODE(55, _FETCH)
// !
  NAME(56, 0, 1, '!', 0, 0)
  LINK(57, 53)
  CODE(58, _STORE)
// ,
  NAME(59, 0, 1, ',', 0, 0)
  LINK(60, 56)
  CODE(61, _COMMA)
// find
  NAME(62, 0, 4, 'f', 'i', 'n')
  LINK(63, 59)
  CODE(64, _FIND)
// execute
  NAME(65, 0, 7, 'e', 'x', 'e')
  LINK(66, 62)
  CODE(67, _EXECUTE)
// ?dup
  NAME(68, 0, 3, '?', 'd', 'u')
  LINK(69, 65)
  CODE(70, _QDUP)
// number
  NAME(71, 0, 6, 'n', 'u', 'm')
  LINK(72, 68)
  CODE(73, _NUMBER)

// quit
  NAME(74, 0, 4, 'q', 'u', 'i')
  LINK(75, 181)
  CODE(76, _NEST)    // docol
  // begin begin
  LIST(77, 6)         // initr
  // begin
  LIST(78, 27)        // parse
  LIST(79, 30)        // word
  LIST(80, 64)        // find
  LIST(81, 70)        // ?dup
  // while (if)
  LIST(82, 3)         // 0branch
  LIST(83, 88)
  LIST(84, 67)        // execute
  LIST(85, 9)         // ok
  // repeat
  LIST(86, 2)         // branch
  LIST(87, 78)
  // (then)
  LIST(88, 73)        // number
  LIST(89, 1)         // 0branch
  LIST(90, 85)        //  to ok
  LIST(91, 7)         // showtib
  LIST(92, 1)         // lit
  LIST(93, '?')
  LIST(94, 31)        // emit
  LIST(95, 34)        // cr
  LIST(96, 6)         // inits
  // again
  LIST(97, 1)         // branch
  LIST(98, 77)
// abort 
  NAME(99, 0, 5, 'a', 'b', 'o')
  LINK(100, 45)
  CODE(101, _NEST)
  LIST(102, 5)         // inits
  // again
  LIST(103, 1)         // branch
  LIST(104, 77)
/*
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
  CODE(59, _aND)
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
*/

// test
  LIST(200, 1) // lit
  LIST(201, 0x41) // A
  LIST(202, 31) // emit
  LIST(203, 34)  // cr
  LIST(204, 2)  // branch
  LIST(205, 200)

  D = 99; // latest word
  H = 105; // top of dictionary

  I = 200; // instruction pointer = abort

  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex");
//  _words ();
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.program [W] ();
  delay (500);
}

