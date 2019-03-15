/*
  Forth virtual machine
  Indirect threaded

  This code is in the public domain.

*/

#define ram-size 0x10000
#define S0 0x1000
#define R0 0x0fff 

// global variables
int memory[0x1000]; // 64k RAM
int S = S0; // data stack pointer
int R = R0; // return stack pointer
int I = 0; // instruction pointer
int W = 0; // working register

// function prototypes for the primitives
void _dummy (void);
void _lit (void);     // 0
void _exit (void);    // 2
void _branch (void);  // 3
void _0branch (void); // 4
void _key (void);     // 5
void _emit (void);    // 6
void _dup (void);
void _drop (void);
void _swap (void);
void _plus (void);

// primitive function array
void (*primitive []) (void) = {
  _lit, // 0
  _exit, _branch, _0branch, _key, // 4
  _emit, _dup, _drop, _swap,      // 8
  _plus, _dummy, _dummy, _dummy,  // 12
  _dummy, _dummy, _dummy, _dummy, // 16
  _dummy, _dummy, _dummy, _dummy, // 20
  _dummy, _dummy, _dummy, _dummy, // 24
  _dummy, _dummy, _dummy, _dummy, // 28
  _dummy, _dummy, _dummy, _dummy  // 32
};

//  primitives
void _dummy (void) {
  Serial.print ("undefined");  
}

void _lit (void) {memory [--S] = memory [I++];}

void _exit (void) {I = memory [R++];}

void _branch (void) {I = memory [I];}

void _0branch (void) {

}

void _key (void) {

}

void _emit (void) {
  char c = memory [S++];
  Serial.write (c);
}

void _dup (void) {
  int T = memory [S--];
  memory [S] = T;
}

void _drop (void) {S += 1;}

void _swap (void) {
  int T = memory [S++];
  int N = memory [S];
  memory [S--] = T;
  memory [S] = N;
}

void _plus (void) {
  int T = memory [S++];
  T = T + memory [S];
  memory [S] = T;
}

void _1plus (void) {
  int T = (memory [S] + 1);
  memory [S] = T;
}

// the setup function runs once when you press reset or power the board
// This will setup stacks and other pointers, initial machine state
void setup() {
  S = S0;
  R = R0;
  memory [0] = ~0x00; // lit
  memory [1] =  0x41; // 'A'
  memory [2] = ~0x06; // dup
  memory [3] = ~0x05; // emit
  memory [4] = ~0x00; // lit
  memory [5] =  0x01; //  1
  memory [6] = ~0x09; // +
  memory [7] = ~0x02; // branch
  memory [8] =  0x02; //  address
  Serial.begin (9600);
  delay (1000);
  I = 0;
}

// the loop function runs over and over again forever
void loop() {
  W = memory [I++];
  if (W < 0) {
    primitive [~W] (); // execute primitive
  } else {
    memory [--R] = I; // nest
    I = W; // into a high level word
  }
  delay (100);
}

