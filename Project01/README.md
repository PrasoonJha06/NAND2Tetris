First Project of NAND2Tetris is about building basic logic gates using a builtin Nand gate.

NOT and AND can easily be derived from NAND. OR can be derived from NOT and AND using De Morgan's law.

The next three gates: XOR (Exclusive Or), MUX (Multiplexer) and DMUX (Demultiplexer) are derived from the three fundamental gates (NOT, AND, OR). How to combine them can be discovered using Boolean synthesis or just observing their truth tables. 

Next multi-bit versions (16 bit to be precise) of the aforementioned chips are constructed. These are pretty straightforward. 16 1-bit chips work independently on on every bit to give us the result.

Next are the multi-way versions of some of the chips. These are the same chips for an extended no of inputs.
