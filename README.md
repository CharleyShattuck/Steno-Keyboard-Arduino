Steno-Keyboard-Arduino
======================

Arduino code for a homemade steno keyboard.

steno_keyboard.ino: The original, separate pin for each key. Arduino Mega.

mux_steno.ino: Second try, multiplexed. Arduino Micro, now modified to use Adafruit Itsy Bitsy and allow pin 8 to ground to choose NKRO keyboard protocol, otherwise TX Bolt. TinyMod2 uses pin 7 for that purpose.

A new addition is a tiny version of Jackdaw, an orthographic chording system for the TinyMod. Still being developed.

Interpreter.ino: Unrelated project. A Forth style interpreter.

Also M0-Forth is a Forth in C for an Arm Arduino, in development.

M0-Forth is now an abandoned first try. Cortex-Forth is simpler and more efficient. Also, I think, easier to explain. But at this point I've stopped working on that one too.
