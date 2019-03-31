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
#define DATA(m, a) {memory.data [m] = a;}

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
  name  32b word,  a 32 bit int, made up of byte count and three letters
  link  32b word, point to next word in list, 0 says end of list
  code  32b word, a pointer to some actual C compiled code,
        all native code is in this field
  data  32b word, at least, a list to execute or a data field of some kind


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
  T = memory.data [S + 1];
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

void _LIT (void) {
  _DUP (); 
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
      _DUP ();
      T = -1;
      return;
    }
    T *= base;
    t -= '0';
    if (t > 9) t -= 37;
    T += t;
  }
  if (tib [0] == '-') T = -T;
  _DUP ();
  T = 0;
}

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
  W = (S0 - 1);
  while (W > (S)) {
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
//  _DROP ();
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
  LINK(81, 77)
  CODE(82, _NEST)
  DATA(83, inits)
#  define abort 83
  // again
  DATA(84, branch)
  DATA(85, 89)
  // quit
  NAME(86, 0, 4, 'q', 'u', 'i')
  LINK(87, 77)
  CODE(88, _NEST)
  DATA(89, initr)
  // begin quit loop
  DATA(90, parse)
  DATA(91, wword)
  DATA(92, find)
  DATA(93, qdup)
  DATA(94, zbranch)
  DATA(95, 103) // to number
  DATA(96, execute)
  DATA(97, depth)
  DATA(98, zeroless)
  DATA(99, zbranch)
  DATA(100, 114) // to ok
  DATA(101, branch)
  DATA(102, 106)
  DATA(103, number)
  DATA(104, zbranch)
  DATA(105, 114) // to ok
  DATA(106, showtib)
  DATA(107, lit)
  DATA(108, '?')
  DATA(109, emit)
  DATA(110, cr)
  DATA(111, inits)
  DATA(112, branch)
  DATA(113, 89)
  DATA(114, ok)
  DATA(115, branch)
  DATA(116, 90) // continue quit loop

  // .
  NAME(117, 0, 1, '.', 0, 0)
  LINK(118, 86)
  CODE(119, _DOT)
#  define dot 119
  // .s
  NAME(120, 0, 2, '.', 's', 0)
  LINK(121, 117)
  CODE(122, _DDOTS)
#  define ddots 122
  // words
  NAME(123, 0, 5, 'w', 'o', 'r')
  LINK(124, 120)
  CODE(125, _WORDS)
#  define words 125
  // space
  NAME(126, 0, 5, 's', 'p', 'a')
  LINK(127, 123)
  CODE(128, _SPACE)
#  define space 128
  // h.
  NAME(129, 0, 2, 'h', '.', 0)
  LINK(130, 126)
  CODE(131, _HDOT)
#  define hdot 131
  // +
  NAME(132, 0, 1, '+', 0, 0)
  LINK(133, 129)
  CODE(134, _PLUS)
  // -
  NAME(135, 0, 1, '-', 0, 0)
  LINK(136, 132)
  CODE(137, _MINUS)
  // and
  NAME(138, 0, 3, 'a', 'n', 'd')
  LINK(139, 135)
  CODE(140, _aND)
  // or
  NAME(141, 0, 2, 'o', 'r', 0)
  LINK(142, 138)
  CODE(143, _OR)
  // xor
  NAME(144, 0, 3, 'x', 'o', 'r')
  LINK(145, 141)
  CODE(146, _XOR)
  // invert
  NAME(147, 0, 6, 'i', 'n', 'v')
  LINK(148, 144)
  CODE(149, _INVERT)
  // abs
  NAME(150, 0, 3, 'a', 'b', 's')
  LINK(151, 147)
  CODE(152, _ABS)
  // negate
  NAME(153, 0, 6, 'n', 'e', 'g')
  LINK(154, 150)
  CODE(155, _NEGATE)
  // 2*
  NAME(156, 0, 2, '2', '*', 0)
  LINK(157, 153)
  CODE(158, _TWOSTAR)
  // 2/
  NAME(159, 0, 2, '2', '/', 0)
  LINK(160, 156)
  CODE(161, _TWOSLASH)

  D = 159; // latest word
  H = 162; // top of dictionary

  // test
  DATA(200, parse)
  DATA(201, wword)
  DATA(202, find)
  DATA(203, dot)
  DATA(204, number)
  DATA(205, dot)
  DATA(206, dot)
//  DATA(201, wword)
//  DATA(202, find)
//  DATA(203, execute)
  DATA(207, ddots)
  DATA(208, showtib)
  DATA(209, branch)
  DATA(210, 200)

  I = abort; // instruction pointer = abort
//  I = 200; //  test
  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex");
  //  _WORDS ();
  //  _DEPTH ();
  //  _DDOTS ();
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.program [W] ();
  //  delay (300);
}



