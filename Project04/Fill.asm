// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

(START_DARK)

    // addr = SCREEN
    @SCREEN 
    D=A 
    @addr 
    M=D 

(DARK)

    // if RAM[KBD] == 0 goto START_LIGHT
    @KBD 
    D=M 
    @START_LIGHT 
    D;JEQ

    // RAM[addr] = -1
    @addr 
    A=M 
    M=-1

    // addr = addr + 1
    @addr
    M=M+1

    // goto DARK
    @DARK 
    0;JMP 

(START_LIGHT) 

    // addr = SCREEN
    @SCREEN 
    D=A 
    @addr 
    M=D 

(LIGHT) 

    // if RAM[KBD] != 0 goto START_DARK
    @KBD
    D=M
    @START_DARK
    D;JNE

    // RAM[addr] = 0
    @addr 
    A=M 
    M=0 

    // addr = addr + 1
    @addr 
    M=M+1 

    // goto LIGHT 
    @LIGHT 
    0;JMP 