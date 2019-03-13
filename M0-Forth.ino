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
void _docode (void);
void _docolon (void);
void _doconst (void);
void _dup (void);
void _drop (void);
void _swap (void);

// primitive function array
void (*primitive []) (void) = {
  _docode, _dup, _drop, _swap, _dummy, _dummy, _dummy, _dummy,
  _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy, _dummy,
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

void _docode (void) {
  W = memory [I++];
  primitive [W] ();
}

void _dup (void) {
  Serial.print (" dup ");
}

void _drop (void) {
  Serial.print (" drop ");
}

void _swap (void) {
  Serial.print (" swap ");
}

// the setup function runs once when you press reset or power the board
// This will setup stacks and other pointers, initial machine state
void setup() {
  S = S0;
  R = R0;
  Serial.begin (9600);
}

// the loop function runs over and over again forever
void loop() {
  delay (5000);
// test
  W = 1; primitive [W] ();
  W = 2; primitive [W] ();
  W = 3; primitive [W] ();
}
