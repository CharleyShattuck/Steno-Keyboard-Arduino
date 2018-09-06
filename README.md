Steno-Keyboard-Arduino
======================

Arduino code for a homemade steno keyboard.

steno_keyboard.ino: The original, separate pin for each key. Arduino Mega.

mux_steno.ino: Second try, multiplexed. Arduino Micro, now modified to use Adafruit Itsy Bitsy and allow pin 8 to ground to choose NKRO keyboard protocol, otherwise TX Bolt.

Interpreter.ino: Unrelated project. A Forth style interpreter.
