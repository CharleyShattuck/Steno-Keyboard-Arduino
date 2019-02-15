Steno-Keyboard-Arduino
======================

Arduino code for a homemade steno keyboard.

steno_keyboard.ino: The original, separate pin for each key. Arduino Mega.

mux_steno.ino: Second try, multiplexed. Arduino Micro, now modified to use Adafruit Itsy Bitsy and allow pin 8 to ground to choose NKRO keyboard protocol, otherwise TX Bolt. TinyMod2 uses pin 7 for that purpose.

A new addition is a tiny version of Jackdaw, an orthographic chording system for the TinyMod. Still being developed. Not every character is available yet.

Interpreter.ino: Unrelated project. A Forth style interpreter.
