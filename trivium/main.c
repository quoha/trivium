//
//  main.c
//  trivium
//
//  Created by Michael Henderson on 12/24/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
// It seems that every year ends with me toying around with the smallest set of instructions needed for a Forth VM. Rogozhin's 4 state universal Turing machine has 22 instructions. Stack machines cheat by combining some operations into a single instruction, so they need Stack (push data stack, pop data stack, push return stack, pop return stack), Arithmetic (subtract), and Control (unconditional and conditional jumps). Add instructions to read and write memory, and you have 9 instructions (11 if you want input and output). Not an efficient VM, but it's complete, right?
//  With hints from
//      http://homepage.cs.uiowa.edu/~jones/arch/cisc/
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vm     vm;
typedef struct vmrslt vmrslt;

void   vm_dump(vm *v);
vm    *vm_new(void);
vmrslt vm_run(vm *v);

#define VM_CORESIZE  64
#define VM_STACKSIZE 16

#define VMRT_OK     0
#define VMRT_PANIC -1

#define VMOP_NOOP 0x00
#define VMOP_DUP  0x01
#define VMOP_ONE  0x02
#define VMOP_ZERO 0x03
#define VMOP_LOAD 0x04
#define VMOP_POP  0x05
#define VMOP_SUB  0x06
#define VMOP_JPOS 0x07
#define VMOP_HALT 0x0F

struct vm {
    unsigned int  pc;  // program counter
    unsigned int  pds; // pointer to top of data stack
    char mem[VM_CORESIZE]; // core memory
    char ds[VM_STACKSIZE]; // data stack
};

struct vmrslt {
    int         status;
    const char *err;
};

void vm_dump(vm *v) {
    int idx;
    printf(".dump: -------\n");
    printf(".....: pc %8d", v->pc);
    for (idx = 0; idx < VM_CORESIZE; idx++) {
        if ((idx % 16) == 0) {
            printf("\n.....: core  ");
        }
        if (idx == v->pc) {
            printf("[%02x]", v->mem[idx]);
        } else {
            printf(" %02x ", v->mem[idx]);
        }
    }
    printf("\n.....: stack ");
    for (idx = 0; idx < VM_STACKSIZE; idx++) {
        if (v->pds == idx) {
            printf("[%02x]", v->ds[idx]);
        } else {
            printf(" %02x ", v->ds[idx]);
        }
    }
    printf("\n");
}

vm *vm_new(void) {
    vm *v = malloc(sizeof(*v));
    if (v) {
        memset(v, 0, sizeof(*v));
        v->pc  = 0;
        v->pds = 0;
    }
    return v;
}

vmrslt vm_run(vm *v) {
    vmrslt r = { 0, 0 };

    for (;;) {
        // fetch instruction
        if (v->pc >= VM_CORESIZE) {
            r.status = VMRT_PANIC;
            r. err = "panic: program counter out of range";
            return r;
        }
        unsigned char op = v->mem[v->pc++];
        switch (op) {
            case VMOP_DUP:
                // duplicate top of stack
                do {
                    char tmp = v->ds[v->pds];
                    v->pds++;
                    if (v->pds == VM_STACKSIZE) {
                        v->pds = 0;
                    }
                    v->ds[v->pds] = tmp;
                } while (0);
                continue;
            case VMOP_HALT:
                // stop running
                return r;
            case VMOP_JPOS:
                // pop target from stack
                // pop test from stack
                // if test is positive then set program counter to target
                do {
                    unsigned int addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_STACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    char test = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_STACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (test >= 0) {
                        if (addr >= VM_CORESIZE) {
                            r.status = VMRT_PANIC;
                            r. err = "panic: pop address out of range";
                            return r;
                        }
                        v->pc = addr;
                    }
                } while (0);
                continue;
            case VMOP_LOAD:
                // pop address from stack
                // push mem[thatAddress] to stack
                do {
                    char addr = v->ds[v->pds];
                    if (addr >= VM_CORESIZE) {
                        r.status = VMRT_PANIC;
                        r. err = "panic: load address out of range";
                        return r;
                    }
                    v->ds[v->pds] = v->mem[addr];
                } while (0);
                continue;
            case VMOP_NOOP:
                // do nothing
                continue;
            case VMOP_ONE:
                // shift the top of the stack left one bit,
                // force the least significant bit to 1
                v->ds[v->pds] = (v->ds[v->pds] << 1) + 1;
                continue;
            case VMOP_POP:
                // pop value from stack
                // pop address from stack
                // store value in mem[address]
                do {
                    char val = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_STACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    unsigned int addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_STACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (addr >= VM_CORESIZE) {
                        r.status = VMRT_PANIC;
                        r. err = "panic: pop address out of range";
                        return r;
                    }
                    v->mem[addr] = val;
                } while (0);
                continue;
            case VMOP_SUB:
                // pop s1 from stack
                // pop s2 from stack
                // push s2 - s1 on to stack
                do {
                    char s1 = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_STACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    v->ds[v->pds] = v->ds[v->pds] - s1;
                } while (0);
                continue;
            case VMOP_ZERO:
                // shift the top of the stack left one bit,
                // force the least significant bit to 0
                v->ds[v->pds] = (v->ds[v->pds] << 1) + 0;
                continue;
        }
        r.status = VMRT_PANIC;
        r. err = "panic: invalid instruction";
        return r;
    }
    return r;
}

int main(int argc, const char * argv[]) {
    vm *v = vm_new();
    vmrslt vr = vm_run(v);
    printf(".info: vr.status %d %s\n", vr.status, vr.err ? vr.err : "");
    vm_dump(v);
    return 0;
}
