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
void _const (void);
void _code (void);
void _enter (void);
void _lit (void);
void _exit (void);
void _key (void);
void _emit (void);
void _dup (void);
void _drop (void);
void _swap (void);
void _1plus (void);

// primitive function array
void (*primitive []) (void) = {
  _lit, _code, _enter, _exit, _key, _emit, _dup, _drop,
  _swap, _1plus, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
};

// functions
void _dummy (void) {
  Serial.print ("undefined");  
}

void _code (void) {
  W = memory [I++];
  primitive [W] ();
}

void _lit (void) {
  int T = memory [I++];
  memory [--S] = T;
}

void _enter (void) {

}

void _exit (void) {

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

void _drop (void) {
  S += 1;
}

void _swap (void) {
  int T = memory [S++];
  int N = memory [S];
  memory [S--] = T;
  memory [S] = N;
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
  Serial.begin (9600);
// test
  memory [--S] = 0x41; // push 'A' onto the stack
}

// the loop function runs over and over again forever
void loop() {
// test
  delay (1000);
  W = 6; primitive [W] (); // execute "dup"
  W = 5; primitive [W] (); // execute "emit"
  W = 9; primitive [W] (); // execute "1+"
}
