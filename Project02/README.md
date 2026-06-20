The second project of NAND2Tetris consists of making chips that do basic
arithmetic calculations in binary.

HalfAdder is used to add two bits.
Its 2 outputs are just basic logic operations.

By observing the truth table of full adder, it an be deduced that it is 
built using two half adders and an Or gate.

Add16 uses previous adders to add an entire 16 bit bus.

Inc16 is a special case of Add16 where b is always 1.

ALU uses conditional statements to generate many different arithmetic 
operations using adders and previously built logic gates.
Conditional statements are realised using Multiplexers.
