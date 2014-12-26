//
//  main.c
//  trivium
//
//  Created by Michael Henderson on 12/24/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vm     vm;
typedef struct vmrslt vmrslt;

void   vm_dump(vm *v);
vm    *vm_new(void);
vmrslt vm_run(vm *v);

#define VM_CORESIZE   64
#define VM_DSTACKSIZE 16
#define VM_RSTACKSIZE 16

#define VMRT_OK     0
#define VMRT_PANIC -1

// there are three categories for instructions: Control, Stack, and Arithmetic
#define VMOP_NOOP   0x00
#define VMOP_JMP    0x01
#define VMOP_JEQ    0x02
#define VMOP_PUSH   0x03
#define VMOP_POP    0x04
#define VMOP_RPUSH  0x05
#define VMOP_RPOP   0x06
#define VMOP_ADD    0x07
#define VMOP_SUB    0x08
#define VMOP_LOAD   0x09
#define VMOP_STOR   0x0a
#define VMOP_HALT   0xff

struct vm {
    unsigned int  pc;  // program counter
    unsigned int  pds; // pointer to top of data stack
    unsigned int  prs; // pointer to top of return stack
    char acc; // accumulator
    unsigned char mem[VM_CORESIZE]; // core memory
    unsigned char ds[VM_DSTACKSIZE]; // data stack
    unsigned char rs[VM_RSTACKSIZE]; // return stack
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
    for (idx = 0; idx < VM_DSTACKSIZE; idx++) {
        if (v->pds == idx) {
            printf("[%02x]", v->ds[idx]);
        } else {
            printf(" %02x ", v->ds[idx]);
        }
    }
    printf("\n.....: rstk  ");
    for (idx = 0; idx < VM_RSTACKSIZE; idx++) {
        if (v->prs == idx) {
            printf("[%02x]", v->rs[idx]);
        } else {
            printf(" %02x ", v->rs[idx]);
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
            case VMOP_ADD:
                // pop s1 from stack, pop s2 from stack, push s2 + s1 on to stack
                do {
                    char s1 = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    v->ds[v->pds] = v->ds[v->pds] + s1;
                } while (0);
                continue;
            case VMOP_HALT:
                // stop running
                return r;
            case VMOP_JEQ:
                // pop target from stack
                // if acc is zero then set program counter to target
                do {
                    unsigned int addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (v->acc == 0) {
                        if (addr >= VM_CORESIZE) {
                            r.status = VMRT_PANIC;
                            r. err = "panic: jeq address out of range";
                            return r;
                        }
                        v->pc = addr;
                    }
                } while (0);
                continue;
            case VMOP_JMP:
                // pop target from stack
                do {
                    unsigned int addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (addr >= VM_CORESIZE) {
                        r.status = VMRT_PANIC;
                        r. err = "panic: jump address out of range";
                        return r;
                    }
                    v->pc = addr;
                } while (0);
                continue;
            case VMOP_LOAD:
                // pop address from stack and save mem[thatAddress] to acc
                do {
                    char addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (addr >= VM_CORESIZE) {
                        r.status = VMRT_PANIC;
                        r. err = "panic: load address out of range";
                        return r;
                    }
                    v->acc = v->mem[addr];
                } while (0);
                continue;
            case VMOP_NOOP:
                // do nothing
                continue;
            case VMOP_POP:
                // pop value from top of stack and save in acc
                v->acc = v->ds[v->pds];
                if (v->pds == 0) {
                    v->pds = VM_DSTACKSIZE - 1;
                } else {
                    v->pds--;
                }
                continue;
            case VMOP_PUSH:
                // push acc to top of stack
                v->pds++;
                if (v->pds == VM_DSTACKSIZE) {
                    v->pds = 0;
                }
                v->ds[v->pds] = v->acc;
                continue;
            case VMOP_RPOP:
                // pop top value from stack and save in acc
                v->acc = v->rs[v->prs];
                if (v->prs == 0) {
                    v->prs = VM_RSTACKSIZE - 1;
                } else {
                    v->prs--;
                }
                continue;
            case VMOP_RPUSH:
                // push acc to top of stack
                v->prs++;
                if (v->prs == VM_RSTACKSIZE) {
                    v->prs = 0;
                }
                v->rs[v->prs] = v->acc;
                continue;
            case VMOP_STOR:
                // pop address from stack and store acc in mem[thatAddress]
                do {
                    char addr = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    if (addr >= VM_CORESIZE) {
                        r.status = VMRT_PANIC;
                        r. err = "panic: stor address out of range";
                        return r;
                    }
                    v->mem[addr] = v->acc;
                } while (0);
                continue;
            case VMOP_SUB:
                // pop s1 from stack, pop s2 from stack, push s2 - s1 on to stack
                do {
                    char s1 = v->ds[v->pds];
                    if (v->pds == 0) {
                        v->pds = VM_DSTACKSIZE - 1;
                    } else {
                        v->pds--;
                    }
                    v->ds[v->pds] = v->ds[v->pds] - s1;
                } while (0);
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
