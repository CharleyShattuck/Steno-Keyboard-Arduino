/*
  Forth virtual machine
  Indirect threaded

  This code is in the public domain.

*/

#define ram-size 0x10000
#define S0 0x10000
#define R0 0xff00 
#define cell 4

byte memory[0x10000]; // 64k RAM
int S = S0; // data stack pointer
int R = R0; // return stack pointer
int I = 0; // instruction pointer
int U = 0; // user pointer

void push (int stack, int value) {
  memory [stack] = value;
  stack -= cell;
}

int pop (int stack) {
  stack += cell;
  return (memory [stack]);
}

void next () {

}

// yield to next task
// the round robin includes both high level Forth tasks
//   and low level C tasks, like I/O
void pause () {
  push (R, I);
  push (R, S);
// save R in user area,
// follow round robin until it finds a task that's awake
// load new U
// load new R
  S = pop (R);
  I = pop (R);
  next;
}

// each task runs quit
void quit () {
  S = S0;
// execute next word, from I
  pause ();
}

// the setup function runs once when you press reset or power the board
// This will setup stacks and other pointers
void setup() {
// setup all the tasks in the round robin
// load U and I to start multitasker
}

// the loop function runs over and over again forever
// This will be the Forth round robin multitasker
void loop() {
  while (1);

}
