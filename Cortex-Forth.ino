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
  if (W > 2) {
    t = tib [2];
    T |= (t << 24);
  }
}

void _NUMBER (void) {
  char t;
  _DUP ();
  T = 0;
  for (int i = 0; i < (tib.length () - 1); i++) {
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

// void _EXECUTE (void) {
//   memory.data [--R] = I;
//   I = (T + 2);
// }
void _EXECUTE (void) {
  W = (T + 2);
  _DROP ();
  memory.program [W] ();
}

void _FIND (void) {
  int X = T;
  T = D;
  while (T != 0) {
    W = (memory.data [T]);
    if (W == X) {
      return;
    }
    T = memory.data [T + 1];
  }
}

void _DOT (void) {
  Serial.print (T);
  Serial.write (' ');
  _DROP ();
}

void _HDOT (void) {
  Serial.print (T, HEX);
  Serial.write (' ');
  _DROP ();
}

void _DDOTS (void) {
  if (S == S0) {
    Serial.print ("empty ");
    return;
  }
  _DUP ();
  W = S0;
  while (W > S) {
    Serial.print (memory.data [--W]);
    Serial.write (' ');
  }
  _DROP ();
}

void _SPACE () {
  Serial.write (' ');
}

void _ZEROEQUAL () {
  if (T == 0) {
    T = -1;
    return;
  }
  T = 0;
}

void _ZEROLESS () {
  if (T < 0) {
    T = -1;
    return;
  }
  T = 0;
}

void _WORDS (void) {
  int C = 0;
  int X = 0;
  int Y = 0;
  W = D;
  while (W) {
    Y = memory.data [W];
    C = (Y & 0xff);
    X = ((Y >> 8) & 0xff);
    _DUP (); T = X; _EMIT ();
    X = ((Y >> 16) & 0xff);
    if (X != 0) {
      _DUP (); T = X; _EMIT ();
    }
    X = ((Y >> 24) & 0xff);
    if (X != 0) {
      _DUP (); T = X; _EMIT ();
    }
    C -= 4;
    while (!(C < 0)) {
      Serial.print ("_");
      C -= 1;
    }
    _SPACE ();
    W = memory.data [++W];
  }
  _CR ();
  _DROP ();
}

void _DEPTH (void) {
  W = S0 - S;
  _DUP ();
  T = W;
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
#  define lit 1
  CODE(2, _BRANCH)
#  define branch 2
  CODE(3, _0BRANCH)
#  define zbranch 3
  //  CODE(4, _DO)
  //  CODE(5, _LOOP)
  CODE(6, _INITR)
#  define initr 6
  CODE(7, _INITS)
#  define inits 7
  CODE(8, _SHOWTIB)
#  define showtib 8
  CODE(9, _OK)
#  define ok 9
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
#  define emit 31
  // cr
  NAME(32, 0, 2, 'c', 'r', 0)
  LINK(33, 29)
  CODE(34, _CR)
#  define cr 34
  // parse
  NAME(35, 0, 5, 'p', 'a', 'r')
  LINK(36, 32)
  CODE(37, _PARSE)
#  define parse 37
  // word
  NAME(38, 0, 4, 'w', 'o', 'r')
  LINK(39, 35)
  CODE(40, _WORD)
#  define wword 40
  // dup
  NAME(41, 0, 3, 'd', 'u', 'p')
  LINK(42, 38)
  CODE(43, _DUP)
#  define dup 43
  // drop
  NAME(44, 0, 4, 'd', 'r', 'o')
  LINK(45, 41)
  CODE(46, _DROP)
#  define drop 46
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
#  define find 64
  // execute
  NAME(65, 0, 7, 'e', 'x', 'e')
  LINK(66, 62)
  CODE(67, _EXECUTE)
#  define execute 67
  // ?dup
  NAME(68, 0, 3, '?', 'd', 'u')
  LINK(69, 65)
  CODE(70, _QDUP)
#  define qdup 70
  // number
  NAME(71, 0, 6, 'n', 'u', 'm')
  LINK(72, 68)
  CODE(73, _NUMBER)
#  define number 73
  // depth
  NAME(74, 0, 5, 'd', 'e', 'p')
  LINK(75, 71)
  CODE(76, _DEPTH)
#  define depth 76
  // 0<
  NAME(77, 0, 2, '0', '<', 0)
  LINK(78, 74)
  CODE(79, _ZEROLESS)
#  define zeroless 79

  // abort
  NAME(80, 0, 5, 'a', 'b', 'o')
  LINK(81, 80)
  CODE(82, _NEST)
  LIST(83, inits)
#  define abort 83
  // again
  LIST(84, branch)
  LIST(85, 89)
  // quit
  NAME(86, 0, 4, 'q', 'u', 'i')
  LINK(87, 77)
  CODE(88, _NEST)
  LIST(89, initr)
  // begin quit loop
  LIST(90, parse)
  LIST(91, wword)
  LIST(92, find)
  LIST(93, qdup)
  LIST(94, zbranch)
  LIST(95, 101) // to number
  LIST(96, execute)
  LIST(97, depth)
  LIST(98, zeroless)
  LIST(99, zbranch)
  LIST(100, 112) // to ok
  LIST(101, number)
  LIST(102, zbranch)
  LIST(103, 112) // to ok
  LIST(104, showtib)
  LIST(105, lit)
  LIST(106, '?')
  LIST(107, emit)
  LIST(108, cr)
  LIST(109, inits)
  LIST(110, branch)
  LIST(111, abort)
  LIST(112, ok)
  LIST(113, branch)
  LIST(114, 90) // continue quit loop


  // .
  NAME(115, 0, 1, '.', 0, 0)
  LINK(116, 86)
  CODE(117, _DOT)
#  define dot 117
  // .s
  NAME(118, 0, 2, '.', 's', 0)
  LINK(119, 115)
  CODE(120, _DDOTS)
#  define ddots 120
  // words
  NAME(121, 0, 5, 'w', 'o', 'r')
  LINK(122, 118)
  CODE(123, _WORDS)
#  define words 123
  // space
  NAME(124, 0, 5, 's', 'p', 'a')
  LINK(125, 121)
  CODE(126, _SPACE)
#  define space 126
  // h.
  NAME(127, 0, 2, 'h', '.', 0)
  LINK(128, 124)
  CODE(129, _HDOT)
#  define hdot 129

  // test
  LIST(200, parse)
  LIST(201, wword)
  LIST(202, find)
  LIST(203, execute)
//  LIST(203, ddots)
//  LIST(204, showtib)
  LIST(204, branch)
  LIST(205, 200)

  D = 127; // latest word
  H = 130; // top of dictionary

  I = abort; // instruction pointer = abreort
//  I = 200; //  test
  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex");
  //  _WORDS ();
  //  _DEPTH ();
  _DDOTS ();
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.program [W] ();
  //  delay (300);
}

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



