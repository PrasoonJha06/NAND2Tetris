Project03 is about memory. Computer remembers information using a chip called DFF (Data Flip Flop) which is provided by the authors.

First, we begin by constructing a 1-bit register, i.e., a chip that can store 1 bit of information and update it as per will of the user. This is then extended to 16 bits which is pretty straightforward.

RAM is a chip which contains mnay registers stacked up. What makes it more complicated than register is that now we have to find a register before reading it or writing on it. This is achieved using combinational chips built in Project01, hence RAM is able to find a register instantaneously (in the same time-cycle).

RAMs are stacked upon each other to make larger RAMs. We end up with a RAM with 16K (2^14 to be precise) registers in this project.

The last chip to be implemented in this project is PC (Program Counter) which is a special type of register. A simple register stores data and updates it. PC in addition to that can also increment the data and reset it. This is done in accordance with a conditional statement given by the authors.
