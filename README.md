trivium
=======

Yet another Forth VM

It seems that every year ends with me toying around with the smallest
set of instructions needed for a Forth VM. Rogozhin's 4 state universal
Turing machine has 22 instructions. Stack machines cheat by combining
some operations into a single instruction, so they need Stack (push
data stack, pop data stack, push return stack, pop return stack),
Arithmetic (subtract), and Control (unconditional and conditional jumps).
Add instructions to read and write memory, and you have 9 instructions
(11 if you want input and output).

Not an efficient VM, but it's complete, right?

With hints from

    http://homepage.cs.uiowa.edu/~jones/arch/cisc/
    http://en.wikipedia.org/wiki/Random-access_stored-program_machine
    http://git.annexia.org/?p=jonesforth.git;a=blob;f=jonesforth.S

