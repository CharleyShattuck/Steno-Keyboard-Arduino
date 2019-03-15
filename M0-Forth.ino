/*
  Forth virtual machine

  This code is in the public domain.

*/

#define RAM_SIZE 0x1000
#define S0 0x1000
#define R0 0x0fff 

// global variables
int memory[RAM_SIZE];
int S = S0; // data stack pointer
int R = R0; // return stack pointer
int I = 0; // instruction pointer
int W = 0; // working register
int T = 0; // top of stack

// function prototypes for the primitives
void _lit (void);
void _exit (void);
void _branch (void);
void _0branch (void);
void _key (void);
void _emit (void);
void _dup (void);
void _drop (void);
void _swap (void);
void _fetch (void);
void _store (void);
void _plus (void);
// void _minus (void);
// void _or (void);
// void _xor (void);
// void _and (void);
// void _invert (void);
// void _zeroequal (void);
// void _zeroless (void);
// void _times (void);
// void _divide (void);
// void _mod (void);
// void _slashmod (void);
// void _starslash (void);

// primitive function array
void (*primitive []) (void) = {
  _lit,
#define _LIT ~0
  _exit,
#define _EXIT ~1
  _branch,
#define _BRANCH ~2
  _0branch,
#define _0BRANCH ~3
  _key,
#define _KEY ~4
  _emit,
#define _EMIT ~5
  _dup,
#define _DUP ~6
  _drop,
#define _DROP ~7
  _swap,
#define _SWAP ~8
  _plus,
#define _PLUS ~9
  _fetch,
#define _FETCH ~10
  _store,
#define _STORE ~11
};

//  primitives

void _lit (void) {
  memory [--S] = memory [I++];
}

void _exit (void) {
  I = memory [R++];
}

void _branch (void) {
  I = memory [I];
}

void _0branch (void) {
  T = memory [S++];
  if (T == 0) {
    I = memory [I];
    return;
  }
  I += 1;
}

void _key (void) {

}

void _emit (void) {
  char c = memory [S++];
  Serial.write (c);
}

void _dup (void) {
  T = memory [S--];
  memory [S] = T;
}

void _drop (void) {
  S += 1;
}

void _swap (void) {
  T = memory [S++];
  W = memory [S];
  memory [S--] = T;
  memory [S] = W;
}

void _plus (void) {
  T = memory [S++];
  T = T + memory [S];
  memory [S] = T;
}

void _fetch (void) {
  W = memory [S];
  T = memory [W];
  memory [S] = T;
}

void _store (void) {
  T = memory [S++];
  W = memory [S++];
  memory [T] = W;
}

// the setup function runs once when you press reset or power the board
// This will setup stacks and other pointers, initial machine state
void setup() {
  S = S0;
  R = R0;
  I = 0;
  memory [0] = _LIT; // lit
  memory [1] =  0x41; // 'A'
  memory [2] = _DUP; // dup
  memory [3] = _EMIT; // emit
  memory [4] = _LIT; // lit
  memory [5] =  0x01; //  1
  memory [6] = _PLUS; // +
  memory [7] = _BRANCH; // branch
  memory [8] =  0x02; //  address
  Serial.begin (9600);
  delay (1000);
}

// the loop function runs over and over again forever
void loop() {
  W = memory [I++];
  if (W < 0) {  // primitives are inverted
    primitive [~W] (); // execute primitive
  } else { // high level words are just addresses
    memory [--R] = I; // nest
    I = W; // into a high level word
  }
  delay (100);
}

