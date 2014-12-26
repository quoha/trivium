trivium
=======

Yet another Forth VM

It seems that every year ends with me toying around with the smallest
set of instructions needed for a Forth VM. Rogozhin's 4 state universal
Turing machine has 22 instructions. Stack machines cheat by combining
some operations into a single instruction, so they need Stack (push
data stack, pop data stack, push return stack, pop return stack),
Arithmetic (add, subtract), and Control (unconditional and conditional jumps).
Add instructions to read and write memory, and you have 10 instructions
(12 if you want input and output).

Not an efficient VM, but it's complete, right?

## Registers

1. ACC -
    Accumulator. Memory flows through this register.
2. DSP -
    Data stack pointer. Points to value on the top of the data stack.
3. RSP -
    Return stack pointer. Points to value on the top of the return stack.
4. PC -
    Program counter. Points to next instruction to execute.

## Control

1. JMP - Perform an unconditional jump.
    Pop S1 from data stack.
    Set PC to S1.
2. JEQ - Perform a conditional jump.
    Pop S1 from data stack.
    If ACC is ZERO, set PC to S1.

## Stack

1. POP -
    Pop S1 from data stack.
    Set ACC to S1.
2. PUSH -
    Push ACC on to data stack.
3. RPOP -
    Pop S1 from return stack.
    Set ACC to S1.
4. RPUSH -
    Push ACC on to return stack.

## Arithmetic

1. ADD -
    Pop S1 from data stack.
    Pop S2 from data stack.
    Push S2 + S1 onto data stack.
2. SUB -
    Pop S1 from data stack.
    Pop S2 from data stack.
    Push S2 - S1 onto data stack.

## Memory

1. LOAD - Load a value from memory into the accumulator
    Pop S1 from data stack.
    Copy ACC to MEMORY[S1].
2. STOR - Store the accumulator into memory
    Pop S1 from data stack.
    Copy MEMORY[S1] to ACC.

## Convenience

1. NOOP
    Do nothing (used for aligning values).
2. HALT
    Stop the VM.

## Sources

With hints from

    http://homepage.cs.uiowa.edu/~jones/arch/cisc/
    http://en.wikipedia.org/wiki/Random-access_stored-program_machine
    http://git.annexia.org/?p=jonesforth.git;a=blob;f=jonesforth.S

