Chapter 7 introduces the idea of Virtual Machine and Two Tier Compilation.

As per it, a high level language instead of getting translated into low level assembly code directly is first translated into a hardware independent code that runs on an imaginary computer called Virtual Machine. This VM code can then be translated to assembly code of actual computers.

The task of Project07 is to write a translator that translates this VM code into its corresponding Hack Assembly code. Albeit the translator developed in this project is incomplete. It can handle push, pop, arithmetic, logical and relational commands. 

Chapter 8 expands the VM translator by adding branching statements and functions.

Enter the name of a VM file or a folder containing VM files in source to translate it into Hack Assembly language.

```bash
./VMTranslator <source>
```
