Chapter 5 is the last chapter of NAND2Tetris where one has to make chips. The Hack Computer is made up of three chips: CPU, Data Memory and ROM32K. ROM32K has been provided by the authors, the rest 2 are to be built.

Even in CPU, they have provided the architecture and thank god they did for I would not have been able to figure it out on my own. The task was to determine how this CPU
works on the basis of every instruction that is loaded into it. For an evening, I just stared at the architecture. It was only the next day that it made sense to me.

The trick to solving the CPU is to go back to the C-instruction tables in chapter 4 and observe for what bit, what is happening.

Then there is the Data Memory that is built by stacking RAM16K, Screen memory map and Keyboard memory map on top of each other. This was somewhat similar to building
RAMs in Project03.

Computer chip is the simplest for what to connect where was already provided by the authors. The hard tasks were the previous 2, this was just the reward for all that
previous hard work.
