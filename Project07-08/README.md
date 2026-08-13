Chapter 7 introduces the idea of Virtual Machine and Two Stage Compilation.

As per it, a high level language instead of getting translated into low level assembly code directly is first translated into a hardware independent code that runs on an imaginary computer called Virtual Machine. This VM code can then be translated to assembly code of actual computers.

The task of Project07 is to write a translator that translates this VM code into its corresponding Hack Assembly code. Albeit the translator developed in this project is incomplete. It can handle push, pop, arithmetic, logical and relational commands. The rest of the features of this VM language are to be implemented in the next project.
