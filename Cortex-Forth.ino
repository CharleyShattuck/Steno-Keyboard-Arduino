/*
  Forth virtual machine

  This code is in the public domain.

*/

#define RAM_SIZE 0x1200
#define S0 0x1000
#define R0 0x0f00
#define NAME(m, f, c, x, y, z) {memory.data [m] = f + c + (x << 8) + (y << 16) + (z << 24);}
#define LINK(m, a) {memory.data [m] = a;}
#define CODE(m, a) {memory.program [m] = a;}
#define DATA(m, a) {memory.data [m] = a;}
#define IMMED 0x80

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
boolean state = false; // compiling or not

/*  A word in the dictionary has these fields:
  name  32b word,  a 32 bit int, made up of byte count and three letters
  link  32b word, point to next word in list, 0 says end of list
  code  32b word, a pointer to some actual C compiled code,
        all native code is in this field
  data  32b word, at least, a list to execute or a data field of some kind


*/


// primitive definitions

void _LBRAC (void) {
  state = false; // interpreting
}

void _RBRAC (void) {
  state = true; // compiling
}

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
//  Serial.write (T);
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
    if (t == ' ') {
      t = Serial.read ();
//      Serial.write (t);
    }
  } while (t == ' ');
  do {
    while (!Serial.available ());
    t = Serial.read ();
//    Serial.write (t);
    tib = tib + t;
  } while (t > ' ');
  Serial.print (tib);
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
      if (state == true) {
        _DUP ();
        T = 1; // forward reference to lit
        _COMMA (); // lit
        _COMMA (); // the number
      }
  _DUP ();
  T = 0;
}

void _EXECUTE (void) {
  if (state == true) {
    if (((memory.data [T]) & 0x80) == 0) {
      T += 2;
      _COMMA ();
      return;
    }
  }
  W = (T + 2);
  _DROP ();
  memory.program [W] ();
}

void _FIND (void) {
  int X = T;
  T = D;
  while (T != 0) {
    W = (memory.data [T]);
    if ((W & 0xffffff7f) == X) {
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

void _DOTWORD () {
  int Y = memory.data [W];
  int X = (Y & 0xff);
  Serial.write ('[');
  Serial.print (X);
  Serial.write (' ');
  X = ((Y >> 8) & 0xff);
  _DUP (); T = X; _EMIT ();
  X = ((Y >> 16) & 0xff);
  if (X != 0) { _DUP (); T = X; _EMIT (); }
  X = ((Y >> 24) & 0xff);
  if (X != 0) { _DUP (); T = X; _EMIT (); }
  Serial.print ("] "); 
}

void _WORDS (void) {
  int i = 0;
  W = D;
  do {
    _DOTWORD ();
    W = memory.data [++W];
    i += 1;
    if ((i % 8) == 0) _CR ();
  } while (memory.data [W + 1]);
}

void _DEPTH (void) {
  W = S0 - S;
  _DUP ();
  T = W;
}

void _DUMP (void) {
  int a = T;
  _DROP ();
  for (int i = 0; i < a; i++) {
    W = T;
    Serial.print (memory.data [T++], HEX);
    Serial.write (' ');
    _DOTWORD ();
  }
}

void _HERE (void) {
  _DUP ();
  T = H;
}

void _ALLOT (void) {
  H += T;
  _DROP ();
}

void _HEAD (void) {
  _PARSE ();
  _WORD ();
  _COMMA ();
  _DUP ();
  T = D;
  _COMMA ();
  D = H - 2;
}

void _DOVAR (void) {
  _DUP ();
  T = (W + 1);
}

void _CREATE (void) {
  _HEAD ();
  _DUP ();
  _DUP ();
  memory.program [S] = _DOVAR;
  _DROP ();
  _COMMA ();
}

void _COLON (void) {
  _HEAD ();
  _DUP ();
  _DUP ();
  memory.program [S] = _NEST;
  _DROP ();
  _COMMA ();
  _RBRAC ();
}

void _SEMI (void) {
  _DUP ();
  T = 25; // forward reference to exit 
  _COMMA (); // compile exit
  _LBRAC (); // stop compiling
}

void _DOCONST (void) {
  _DUP ();
  T = memory.data [W + 1];
}

void _CONSTANT (void) {
  _HEAD ();
  _DUP ();
  _DUP ();
  memory.program [S] = _DOCONST;
  _DROP ();
  _COMMA ();
  _COMMA ();
}

void _VARIABLE (void) {
  _CREATE ();
  H += 1;
}

void _QUESTION (void) {
  _FETCH ();
  _DOT ();
}

void _R (void) {
  _DUP ();
  T = R;
}

void _DO (void) {
  memory.data [--R] = T;
  _DROP ();
  memory.data [--R] = T;
  _DROP ();
}

void _LOOP (void) {
  int X = memory.data [R++];
  W = (memory.data [R++] + 1);
  if (W == X) {
    I += 1;
    return;
  }
  memory.data [--R] = (W);
  memory.data [--R] = X;
  I = memory.data [I];
}

void _I (void) {
  _DUP ();
  T = memory.data [R + 1];
}

void _CDO (void) {
  _DUP ();
  T = 4; // forward reference to ddo
  _COMMA ();
  _DUP ();
  T = H;
}

void _CLOOP (void) {
  _DUP ();
  T = 5; // forward reference to lloop
  _COMMA ();
  _COMMA (); // address left on stack by do
}

void _CBEGIN (void) {
  _DUP ();
  T = H;
}

void _CUNTIL (void) {
  _DUP ();
  T = 3; // forward reference to Obranch
  _COMMA ();
  _COMMA (); // address left on stack by begin
}

void _CAGAIN (void) {
  _DUP ();
  T = 2; // forward reference to branch
  _COMMA ();
  _COMMA (); // address left on stack by begin
}

void _CIF (void) {
  _DUP ();
  T = 3; // forward reference to 0branch
  _COMMA ();
  _DUP ();
  T = H; // address that needs patching later
  _DUP ();
  T = 0;
  _COMMA (); // dummy in address field
}

void _CWHILE (void) {
  _CIF ();
  _SWAP ();
}

void _CTHEN (void) {
  _DUP ();
  T = H;
  _SWAP ();
  _STORE ();
}

void _CREPEAT (void) {
  _CAGAIN ();
  _CTHEN ();
}

void _CELSE (void) {
  _DUP ();
  T = 2; // forward reference to branch
  _COMMA ();
  _DUP ();
  T = H; // address that needs patching later
  _DUP ();
  T = 0;
  _COMMA (); // dummy in address field
  _SWAP ();
  _CTHEN ();
}

void _FORGET (void) {
  _PARSE ();
  _WORD ();
  _FIND ();
  D = memory.data [T + 1];
  H = T;
  _DROP ();
}

void _TICK (void) {
  _PARSE ();
  _WORD ();
  _FIND ();
}

void _CLITERAL (void) {
  _DUP ();
  T = 1; // forward reference to lit
  _COMMA ();
  _COMMA (); // the number that was already on the stack
}

void _CFETCH (void) {
  W = (T % 4);
  T = memory.data [T / 4];
  T = (T >> (W * 8) & 0xff);
} 

void _CSTORE (void) {
  int X = (T / 4);
  W = (T % 4);
  _DROP ();
  T = (T << (W * 8));
  T = (T | (memory.data [X] & ~(0xff << (W * 8))));
  memory.data [X] = T;
  _DROP ();
} 



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
  CODE(4, _DO)
#  define ddo 4
  CODE(5, _LOOP)
#  define lloop 5
  CODE(6, _INITR)
#  define initr 6
  CODE(7, _INITS)
#  define inits 7
  CODE(8, _SHOWTIB)
#  define showtib 8
  CODE(9, _OK)
#  define ok 9
  // room to expand here

  // trailing space kludge
  NAME(20, 0, 0, 10, 0, 0)
  LINK(21, 0)
  CODE(22, _NOP)
  // exit
  NAME(23, 0, 4, 'e', 'x', 'i')
  LINK(24, 20)
  CODE(25, _EXIT)
#  define exit 25
  // key ( - c)
  NAME(26, 0, 3, 'k', 'e', 'y')
  LINK(27, 23)
  CODE(28, _KEY)
  // emit ( c - )
  NAME(29, 0, 4, 'e', 'm', 'i')
  LINK(30, 26)
  CODE(31, _EMIT)
#  define emit 31
  // cr
  NAME(32, 0, 2, 'c', 'r', 0)
  LINK(33, 29)
  CODE(34, _CR)
#  define cr 34
  // parse // leaves string in tib
  NAME(35, 0, 5, 'p', 'a', 'r')
  LINK(36, 32)
  CODE(37, _PARSE)
#  define parse 37
  // word ( - n) gets string from tib
  NAME(38, 0, 4, 'w', 'o', 'r')
  LINK(39, 35)
  CODE(40, _WORD)
#  define wword 40
  // dup ( n - n n)
  NAME(41, 0, 3, 'd', 'u', 'p')
  LINK(42, 38)
  CODE(43, _DUP)
#  define dup 43
  // drop ( n - )
  NAME(44, 0, 4, 'd', 'r', 'o')
  LINK(45, 41)
  CODE(46, _DROP)
#  define drop 46
  // swap ( n1 n2 - n2 n1)
  NAME(47, 0, 4, 's', 'w', 'a')
  LINK(48, 44)
  CODE(49, _SWAP)
#  define swap 49
  // over ( n1 n2 - n1 n2 n1)
  NAME(50, 0, 4, 'o', 'v', 'e')
  LINK(51, 47)
  CODE(52, _OVER)
#  define over 52
  // @ ( a - n)
  NAME(53, 0, 1, '@', 0, 0)
  LINK(54, 50)
  CODE(55, _FETCH)
  // ! ( n a - )
  NAME(56, 0, 1, '!', 0, 0)
  LINK(57, 53)
  CODE(58, _STORE)
  // , ( n - )
  NAME(59, 0, 1, ',', 0, 0)
  LINK(60, 56)
  CODE(61, _COMMA)
  // find ( n - a)
  NAME(62, 0, 4, 'f', 'i', 'n')
  LINK(63, 59)
  CODE(64, _FIND)
#  define find 64
  // execute ( a)
  NAME(65, 0, 7, 'e', 'x', 'e')
  LINK(66, 62)
  CODE(67, _EXECUTE)
#  define execute 67
  // ?dup ( n - 0 | n n)
  NAME(68, 0, 3, '?', 'd', 'u')
  LINK(69, 65)
  CODE(70, _QDUP)
#  define qdup 70
  // number ( - n -f) gets string from tib
  NAME(71, 0, 6, 'n', 'u', 'm')
  LINK(72, 68)
  CODE(73, _NUMBER)
#  define number 73
  // depth ( - n)
  NAME(74, 0, 5, 'd', 'e', 'p')
  LINK(75, 71)
  CODE(76, _DEPTH)
#  define depth 76
  // 0< ( n - f)
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

  // . ( n - )
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
  // h. ( n - )
  NAME(129, 0, 2, 'h', '.', 0)
  LINK(130, 126)
  CODE(131, _HDOT)
#  define hdot 131
  // + ( n1 n2 - n3)
  NAME(132, 0, 1, '+', 0, 0)
  LINK(133, 129)
  CODE(134, _PLUS)
#  define plus 134
  // - ( n1 n2 - n3)
  NAME(135, 0, 1, '-', 0, 0)
  LINK(136, 132)
  CODE(137, _MINUS)
  // and (n1 n2 - n3)
  NAME(138, 0, 3, 'a', 'n', 'd')
  LINK(139, 135)
  CODE(140, _aND)
  // or ( n1 n2 - n3)
  NAME(141, 0, 2, 'o', 'r', 0)
  LINK(142, 138)
  CODE(143, _OR)
  // xor ( n1 n2 - n3)
  NAME(144, 0, 3, 'x', 'o', 'r')
  LINK(145, 141)
  CODE(146, _XOR)
  // invert ( n1 - n2)
  NAME(147, 0, 6, 'i', 'n', 'v')
  LINK(148, 144)
  CODE(149, _INVERT)
  // abs ( n1 - n2)
  NAME(150, 0, 3, 'a', 'b', 's')
  LINK(151, 147)
  CODE(152, _ABS)
  // negate ( n1 - n2)
  NAME(153, 0, 6, 'n', 'e', 'g')
  LINK(154, 150)
  CODE(155, _NEGATE)
  // 2* ( n1 - n2)
  NAME(156, 0, 2, '2', '*', 0)
  LINK(157, 153)
  CODE(158, _TWOSTAR)
  // 2/ ( n1 - n2)
  NAME(159, 0, 2, '2', '/', 0)
  LINK(160, 156)
  CODE(161, _TWOSLASH)
  // dump ( a n - a+n) 
  NAME(162, 0, 4, 'd', 'u', 'm')
  LINK(163, 159)
  CODE(164, _DUMP)
  // create
  NAME(165, 0, 6, 'c', 'r', 'e')
  LINK(166, 162)
  CODE(167, _CREATE)
  // here 
  NAME(168, 0, 4, 'h', 'e', 'r')
  LINK(169, 165)
  CODE(170, _HERE)
  // allot 
  NAME(171, 0, 5, 'a', 'l', 'l')
  LINK(172, 168)
  CODE(173, _ALLOT)
  // variable 
  NAME(174, 0, 8, 'v', 'a', 'r')
  LINK(175, 171)
  CODE(176, _VARIABLE)
  // ?
  NAME(177, 0, 1, '?', 0, 0)
  LINK(178, 174)
  CODE(179, _QUESTION)
  // constant
  NAME(180, 0, 8, 'c', 'o', 'n')
  LINK(181, 177)
  CODE(182, _CONSTANT)
  // R
  NAME(183, 0, 1, 'R', 0, 0)
  LINK(184, 180)
  CODE(185, _R)
  // [ 
  NAME(186, IMMED, 1, '[', 0, 0)
  LINK(187, 183)
  CODE(188, _LBRAC)
  // ]
  NAME(189, 0, 1, ']', 0, 0)
  LINK(190, 186)
  CODE(191, _RBRAC)
  // :
  NAME(192, 0, 1, ':', 0, 0)
  LINK(193, 189)
  CODE(194, _COLON)
  // ;
  NAME(195, IMMED, 1, ';', 0, 0)
  LINK(196, 192)
  CODE(197, _SEMI)
  // i 
  NAME(198, 0, 1, 'i', 0, 0)
  LINK(199, 195)
  CODE(200, _I)
#  define _i 200
  // do
  NAME(201, IMMED, 2, 'd', 'o', 0)
  LINK(202, 198)
  CODE(203, _CDO)
  // loop 
  NAME(204, IMMED, 4, 'l', 'o', 'o')
  LINK(205, 201)
  CODE(206, _CLOOP)
  // begin 
  NAME(207, IMMED, 5, 'b', 'e', 'g')
  LINK(208, 204)
  CODE(209, _CBEGIN)
  // until 
  NAME(210, IMMED, 5, 'u', 'n', 't')
  LINK(211, 207)
  CODE(212, _CUNTIL)
  // if
  NAME(213, IMMED, 2, 'i', 'f', 0)
  LINK(214, 210)
  CODE(215, _CIF)
  // then
  NAME(216, IMMED, 4, 't', 'h', 'e')
  LINK(217, 213)
  CODE(218, _CTHEN)
  // else
  NAME(219, IMMED, 4, 'e', 'l', 's')
  LINK(220, 216)
  CODE(221, _CELSE)
  // forget
  NAME(222, 0, 6, 'f', 'o', 'r')
  LINK(223, 219)
  CODE(224, _FORGET)
  // '
  NAME(225, 0, 1, '\'', 0, 0)
  LINK(226, 222)
  CODE(227, _TICK)
  // again
  NAME(228, IMMED, 5, 'a', 'g', 'a')
  LINK(229, 225)
  CODE(230, _CAGAIN)
  // while
  NAME(231, IMMED, 5, 'w', 'h', 'i')
  LINK(232, 228)
  CODE(233, _CWHILE)  
  // repeat
  NAME(234, IMMED, 6, 'r', 'e', 'p')
  LINK(235, 231)
  CODE(236, _CREPEAT)
  // literal 
  NAME(237, IMMED, 7, 'l', 'i', 't')
  LINK(238, 234)
  CODE(239, _CLITERAL)
  // c@ ( b - c) 
  NAME(240, 0, 2, 'c', '@', 0)
  LINK(241, 237)
  CODE(242, _CFETCH)
#  define cfetch 242
  // c! ( c b - ) 
  NAME(243, 0, 2, 'c', '!', 0)
  LINK(244, 240)
  CODE(245, _CSTORE)
  // type ( b c - ) 
  NAME(246, 0, 4, 't', 'y', 'p')
  LINK(247, 243)
  CODE(248, _NEST)
  DATA(249, over)
  DATA(250, plus)
  DATA(251, swap)
  DATA(252, ddo)
  DATA(253, _i)
  DATA(254, cfetch)
  DATA(255, emit)
  DATA(256, lloop)
  DATA(257, 253)
  DATA(258, exit)

 
  // test
  DATA(300, lit)
  DATA(301, 10) // i
  DATA(302, lit)
  DATA(303, 0) // i
  DATA(304, ddo)
  DATA(305, 200) // i
  DATA(306, dot)
  DATA(307, lloop)
  DATA(308, 305)
  DATA(309, 185) // R
  DATA(310, dot)
  DATA(311, ddots)
  DATA(312, cr)
  DATA(313, branch)
  DATA(314, 300)



  D = 246; // latest word
  H = 259; // top of dictionary

//  I = 300; // test
  I = abort; // instruction pointer = abort
  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex");
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.program [W] ();
//  delay (300);
}



