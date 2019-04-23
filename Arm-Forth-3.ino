/*
  Forth virtual machine

  This code is in the public domain.

*/

#define RAM_SIZE 0x1200
#define S0 0x1000
#define _TIB 0x4004 // (4 * S0) + 4
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
//  Serial.write (T); Serial.flush ();
}

void _EMIT (void) {
  char c = T;
  Serial.write (c); Serial.flush ();
  _DROP ();
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

void _AAND (void) {
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

/*
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
*/

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

void _DEPTH (void) {
  W = S0 - S;
  _DUP ();
  T = W;
}

void _HERE (void) {
  _DUP ();
  T = H;
}

void _ALLOT (void) {
  H += T;
  _DROP ();
}

void _DOVAR (void) {
  _DUP ();
  T = (W + 1);
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

void _EQUAL (void) {
  W = T;
  _DROP ();
  if (T == W) {
    T = -1;
  } else {
    T = 0;
  }
}

void _DOT (void) {
  Serial.print (T, HEX);
  _DROP ();
  Serial.write (' ');
}

void _PLUSSTORE (void) {
  W = T;
  _DROP ();
  T = (memory.data [W] + T);
  memory.data [W] = T;
  _DROP ();
}

void _COUNT (void) {
  W = (memory.data [T] & 0xff);
  T *= 4;
  T += 1;
  _DUP ();
  T = W;
}

void _ONEPLUS (void) {
  T += 1;
}

void _ONEMINUS (void) {
  T -= 1;
}

void _DEBUG (void) {
  _DUP ();
  T = R;
  _DOT ();
  Serial.print (" ");
  _DUP ();
  T = S;
  _DOT ();
  Serial.println (" ");
}

void _CR (void) {
  Serial.write (13);
  Serial.write (10);
}

void _SPACE (void) {
  Serial.write (32);
}

void _BL (void) {
  _DUP ();
  T = 32;
}

void _DASHTRAILING (void) {
  int A = 0;
  do {
    _OVER (); _OVER (); _PLUS (); _ONEMINUS ();
    _CFETCH (); A = T; _DROP ();
    if (A == 32) _ONEMINUS ();
  } while (A == 32) ;
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
//  CODE(8, _OK)
// .#  define ok 8
  // room to expand here

  // trailing space kludge (is this needed now?)
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
#  define key 28
  // emit ( c - )
  NAME(29, 0, 4, 'e', 'm', 'i')
  LINK(30, 26)
  CODE(31, _EMIT)
#  define emit 31
  // dup ( n - n n)
  NAME(32, 0, 3, 'd', 'u', 'p')
  LINK(33, 29)
  CODE(34, _DUP)
#  define dup 34
  // drop ( n - )
  NAME(35, 0, 4, 'd', 'r', 'o')
  LINK(36, 32)
  CODE(37, _DROP)
#  define drop 37
  // swap ( n1 n2 - n2 n1)
  NAME(38, 0, 4, 's', 'w', 'a')
  LINK(39, 35)
  CODE(40, _SWAP)
#  define swap 40
  // over ( n1 n2 - n1 n2 n1)
  NAME(41, 0, 4, 'o', 'v', 'e')
  LINK(42, 38)
  CODE(43, _OVER)
#  define over 43
  // @ ( a - n)
  NAME(44, 0, 1, '@', 0, 0)
  LINK(45, 43)
  CODE(46, _FETCH)
#  define fetch 46
  // ! ( n a - )
  NAME(47, 0, 1, '!', 0, 0)
  LINK(48, 44)
  CODE(49, _STORE)
#  define store 49
  // , ( n - )
  NAME(50, 0, 1, ',', 0, 0)
  LINK(51, 47)
  CODE(52, _COMMA)
#  define comma 52
  // find ( n - a)
  NAME(53, 0, 4, 'f', 'i', 'n')
  LINK(54, 50)
  CODE(55, _FIND)
#  define find 55
  // execute ( a)
  NAME(56, 0, 7, 'e', 'x', 'e')
  LINK(57, 53)
  CODE(58, _EXECUTE)
#  define execute 58
  // ?dup ( n - 0 | n n)
  NAME(59, 0, 3, '?', 'd', 'u')
  LINK(60, 56)
  CODE(61, _QDUP)
#  define qdup 61
  // 0< ( n - f)
  NAME(62, 0, 2, '0', '<', 0)
  LINK(63, 59)
  CODE(64, _ZEROLESS)
#  define zeroless 64
  // + ( n1 n2 - n3)
  NAME(65, 0, 1, '+', 0, 0)
  LINK(66, 62)
  CODE(67, _PLUS)
#  define plus 67
  // - ( n1 n2 - n3)
  NAME(68, 0, 1, '-', 0, 0)
  LINK(69, 65)
  CODE(70, _MINUS)
#  define minus 70
  // and (n1 n2 - n3)
  NAME(71, 0, 3, 'a', 'n', 'd')
  LINK(72, 68)
  CODE(73, _AAND)
#  define aand 73
  // or ( n1 n2 - n3)
  NAME(74, 0, 2, 'o', 'r', 0)
  LINK(75, 71)
  CODE(76, _OR)
#  define oor 76
  // xor ( n1 n2 - n3)
  NAME(77, 0, 3, 'x', 'o', 'r')
  LINK(78, 74)
  CODE(79, _XOR)
#  define xxor 79
  // invert ( n1 - n2)
  NAME(80, 0, 6, 'i', 'n', 'v')
  LINK(81, 77)
  CODE(82, _INVERT)
#  define invert 82
  // abs ( n1 - n2)
  NAME(83, 0, 3, 'a', 'b', 's')
  LINK(84, 80)
  CODE(85, _ABS)
#  define aabs 85
  // negate ( n1 - n2)
  NAME(86, 0, 6, 'n', 'e', 'g')
  LINK(87, 83)
  CODE(88, _NEGATE)
#  define negate 88
  // 2* ( n1 - n2)
  NAME(89, 0, 2, '2', '*', 0)
  LINK(90, 86)
  CODE(91, _TWOSTAR)
#  define twostar 91
  // 2/ ( n1 - n2)
  NAME(92, 0, 2, '2', '/', 0)
  LINK(93, 89)
  CODE(94, _TWOSLASH)
#  define twoslash 92
  // here 
  NAME(95, 0, 4, 'h', 'e', 'r')
  LINK(96, 92)
  CODE(97, _HERE)
#  define here 97
  // allot 
  NAME(98, 0, 5, 'a', 'l', 'l')
  LINK(99, 95)
  CODE(100, _ALLOT)
#  define allot 100
  // [ 
  NAME(101, IMMED, 1, '[', 0, 0)
  LINK(102, 98)
  CODE(103, _LBRAC)
#  define lbrac 103
  // ]
  NAME(104, 0, 1, ']', 0, 0)
  LINK(105, 101)
  CODE(106, _RBRAC)
#  define rbrac 106
  // ;
  NAME(107, IMMED, 1, ';', 0, 0)
  LINK(108, 104)
  CODE(109, _SEMI)
  // i 
  NAME(110, 0, 1, 'i', 0, 0)
  LINK(111, 107)
  CODE(112, _I)
#  define _i 112
  // do
  NAME(113, IMMED, 2, 'd', 'o', 0)
  LINK(114, 110)
  CODE(115, _CDO)
  // loop 
  NAME(116, IMMED, 4, 'l', 'o', 'o')
  LINK(117, 113)
  CODE(118, _CLOOP)
  // begin 
  NAME(119, IMMED, 5, 'b', 'e', 'g')
  LINK(120, 116)
  CODE(121, _CBEGIN)
  // until 
  NAME(122, IMMED, 5, 'u', 'n', 't')
  LINK(123, 119)
  CODE(124, _CUNTIL)
  // if
  NAME(125, IMMED, 2, 'i', 'f', 0)
  LINK(126, 122)
  CODE(127, _CIF)
  // then
  NAME(128, IMMED, 4, 't', 'h', 'e')
  LINK(129, 125)
  CODE(130, _CTHEN)
  // else
  NAME(131, IMMED, 4, 'e', 'l', 's')
  LINK(132, 128)
  CODE(133, _CELSE)
  // again
  NAME(134, IMMED, 5, 'a', 'g', 'a')
  LINK(135, 131)
  CODE(136, _CAGAIN)
  // while
  NAME(137, IMMED, 5, 'w', 'h', 'i')
  LINK(138, 134)
  CODE(139, _CWHILE)  
  // repeat
  NAME(140, IMMED, 6, 'r', 'e', 'p')
  LINK(141, 137)
  CODE(142, _CREPEAT)
  // literal 
  NAME(143, IMMED, 7, 'l', 'i', 't')
  LINK(144, 140)
  CODE(145, _CLITERAL)
// byte addressing words  
  // c@ ( b - c) 
  NAME(146, 0, 2, 'c', '@', 0)
  LINK(147, 143)
  CODE(148, _CFETCH)
#  define cfetch 148
  // c! ( c b - ) 
  NAME(149, 0, 2, 'c', '!', 0)
  LINK(150, 146)
  CODE(151, _CSTORE)
#  define cstore 151
  // type ( b c - ) 
  NAME(152, 0, 4, 't', 'y', 'p')
  LINK(153, 149)
  CODE(154, _NEST)
#  define type 154
  DATA(155, over)
  DATA(156, plus)
  DATA(157, swap)
  DATA(158, ddo)
  DATA(159, _i)
  DATA(160, cfetch)
  DATA(161, emit)
  DATA(162, lloop)
  DATA(163, 159)
  DATA(164, exit)
  // =
  NAME(165, 0, 1, '=', 0, 0)
  LINK(166, 152)
  CODE(167, _EQUAL)
#  define equal 167
  // cr
  NAME(168, 0, 2, 'c', 'r', 0)
  LINK(169, 165)
  CODE(170, _CR)
#  define cr 170
  // space 
  NAME(171, 0, 5, 's', 'p', 'a')
  LINK(172, 168)
  CODE(173, _SPACE)
#  define space 173
  // -trailing ( b c - )
  NAME(174, 0, 9, '-', 't', 'r')
  LINK(175, 171)
  CODE(176, _DASHTRAILING)
#  define dashtrailing 176
  // #tib 
  NAME(177, 0, 4, '#', 't', 'i')
  LINK(178, 174)
  CODE(179, _DOVAR)
#  define ntib 179
  DATA(180, 0)
  // .  // temporary, for initial debug
  NAME(181, 0, 1, '.', 0, 0)
  LINK(182, 177)
  CODE(183, _DOT)
#  define dot 183
  // tib
  NAME(184, 0, 3, 't', 'i', 'b')
  LINK(185, 181)
  CODE(186, _DOCONST)
# define tib 186
  DATA(187, _TIB) 
  // +! 
  NAME(188, 0, 2, '+', '!', 0)
  LINK(189, 184)
  CODE(190, _PLUSSTORE)
# define plusstore 190
  // >in ( - b) 
  NAME(191, 0, 3, '>', 'i', 'n')
  LINK(192, 188)
  CODE(193, _DOVAR)
#  define toin 193 
  DATA(194, 0)
  // count ( a - b c)
  NAME(195, 0, 5, 'c', 'o', 'u')
  LINK(196, 191)
  CODE(197, _COUNT)
# define count 197

  // query 
  NAME(200, 0, 5, 'q', 'u', 'e')
  LINK(201, 195)
  CODE(202, _NEST)
#  define query 202
  DATA(203, lit)
  DATA(204, 0)
  DATA(205, ntib)
  DATA(206, store)
  DATA(207, key) // begin
  DATA(208, dup)
  DATA(209, lit)
  DATA(210, 13)
  DATA(211, minus)
  DATA(212, zbranch) // while
  DATA(213, 240)
  DATA(214, dup)
  DATA(215, emit)
  DATA(216, dup)
  DATA(217, lit)
  DATA(218, 127) // backspace
  DATA(219, equal)
  DATA(220, zbranch) // if
  DATA(221, 229)
  DATA(222, drop)
  DATA(223, lit)
  DATA(224, -1)
  DATA(225, ntib)
  DATA(226, plusstore)
  DATA(227, branch) // else
  DATA(228, 238)
  DATA(229, tib)
  DATA(230, ntib)
  DATA(231, fetch)
  DATA(232, plus)
  DATA(233, cstore)
  DATA(234, lit)
  DATA(235, 1)
  DATA(236, ntib)
  DATA(237, plusstore)
  DATA(238, branch) // repeat
  DATA(239, 207)
  DATA(240, drop)
  DATA(241, lit)
  DATA(242, 0)
  DATA(243, toin)
  DATA(244, store)
  DATA(245, tib)
  DATA(246, ntib)
  DATA(247, fetch)
  DATA(248, dashtrailing)
  DATA(249, ntib)
  DATA(250, store)
  DATA(251, drop)
  DATA(252, exit)

  // char ( - c)
  NAME(253, 0, 4, 'c', 'h', 'a')
  LINK(254, 200)
  CODE(255, _NEST)
# define cchar 255
  DATA(256, toin)
  DATA(257, fetch)
  DATA(258, ntib)
  DATA(259, fetch)
  DATA(260, equal)
  DATA(261, zbranch) // if
  DATA(262, 267)
  DATA(263, lit)
  DATA(264, 32)
  DATA(265, branch) // else
  DATA(266, 271)
  DATA(267, tib)
  DATA(268, toin)
  DATA(269, fetch)
  DATA(270, plus)
  DATA(271, cfetch) // then
  DATA(272, lit)
  DATA(273, 1)
  DATA(274, toin)
  DATA(275, plusstore)
  DATA(276, exit)
  // trim ( c - c)
  NAME(277, 0, 4, 't', 'r', 'i')
  LINK(278, 253)
  CODE(279, _NEST)
#  define ttrim 279
  DATA(280, cchar) // begin
  DATA(281, over)
  DATA(282, minus)
  DATA(283, toin)
  DATA(284, fetch)
  DATA(285, ntib)
  DATA(286, fetch)
  DATA(287, equal)
  DATA(288, oor)
  DATA(289, zbranch) // if
  DATA(290, 296)
  DATA(291, lit)
  DATA(292, -1)
  DATA(293, toin)
  DATA(294, plusstore)
  DATA(295, exit)
  DATA(296, branch) // again
  DATA(297, 280)
  // 1+
  NAME(299, 0, 2, '1', '+', 0)
  LINK(300, 277)
  CODE(301, _ONEPLUS)
#  define oneplus 301
  // 1-
  NAME(302, 0, 2, '1', '-', 0)
  LINK(303, 299)
  CODE(304, _ONEMINUS)
#  define oneminus 304
  // bl
  NAME(305, 0, 2, 'b', 'l', 0)
  LINK(306, 302)
  CODE(307, _BL)
#  define bl 307
//  DATA(308, 32)
  // #chars ( c - c n)
  NAME(309, 0, 5, '#', 'c', 'h')
  LINK(310, 309)
  CODE(311, _NEST)
#  define nchars 311
  DATA(312, toin)
  DATA(313, fetch)
  DATA(314, swap)
  DATA(315, cchar) // begin 
  DATA(316, over)
  DATA(317, equal)
  DATA(318, toin)
  DATA(319, fetch)
  DATA(320, ntib)
  DATA(321, fetch)
  DATA(322, oneplus)
  DATA(323, equal)
  DATA(324, oor)
  DATA(325, zbranch) // if
  DATA(326, 337)
  DATA(327, swap)
  DATA(328, toin)
  DATA(329, fetch)
  DATA(330, over)
  DATA(331, minus)
  DATA(332, oneminus)
  DATA(333, swap)
  DATA(334, toin)
  DATA(335, store)
  DATA(336, exit)
  DATA(337, branch) // again then
  DATA(338, 315)
  // word ( c - a)
  NAME(339, 0, 4, 'w', 'o', 'r')
  LINK(340, 309)
  CODE(341, _NEST)
#  define wword 341
  DATA(342, ttrim)
  DATA(343, nchars)
  DATA(344, here)
  DATA(345, store)
  DATA(346, here)
  DATA(347, count)
  DATA(348, over)
  DATA(349, plus)
  DATA(350, swap)
  DATA(351, ddo)
  DATA(352, cchar)
  DATA(353, _i)
  DATA(354, cstore)
  DATA(355, lloop)
  DATA(356, 352)
  DATA(357, here)
  DATA(358, exit)
  // debug
  NAME(359, 0, 5, 'd', 'e', 'b')
  LINK(360, 339)
  CODE(361, _DEBUG)
#  define debug 361


  D = 359; // latest word
  H = 362; // top of dictionary


  DATA(407, query) // begin
  DATA(408, cr)
//  DATA(402, tib)
//  DATA(403, ntib)
//  DATA(404, fetch)
//  DATA(405, dashtrailing)
//  DATA(406, ntib)
//  DATA(407, store)
//  DATA(408, drop)
  DATA(409, toin) // begin
  DATA(410, fetch)
  DATA(411, ntib)
  DATA(412, fetch)
  DATA(413, minus)
  DATA(414, zeroless)
  DATA(415, zbranch) // while
  DATA(416, 424)
  DATA(417, bl)
  DATA(418, wword)
  DATA(419, count)
  DATA(420, type)
  DATA(421, cr)
  DATA(422, branch) // repeat
  DATA(423, 409)
  DATA(424, cr)
  DATA(425, branch) // again
  DATA(426, 407)

  I = 407; // test
//  I = abort; // instruction pointer = abort
  Serial.begin (9600);
  while (!Serial);
  Serial.println ("myForth Arm Cortex"); Serial.flush ();
}

// the loop function runs over and over again forever
void loop() {
  W = memory.data [I++];
  memory.program [W] ();
//  delay (100);
}
