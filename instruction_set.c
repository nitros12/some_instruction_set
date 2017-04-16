/*

each instruction is of 1 to 3 bytes long.

first two bits decide on length of instruction (00, 01, 10)

1 byte: [0 0] [0 0 0 0 0 0] (single opcodes only)
                 opcode

2 byte: [0 1] [0 0 0 0 0 0] [0 0 0 0 0 0 0 0]
                 opcode         operand 1

3 byte: [1 0] [0 0 0 0 0 0] [0 0 0 0 0 0 0 0] [0 0 0 0 0 0 0 0]
                 opcode         operand 1         operand 2

*/

#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>

char cpu_popstack(struct Cpu *cpu) {
  return cpu->memory[++cpu->regs.esp]; // move up, get value
}

void cpu_pushstack(struct Cpu *cpu, char val) {
  cpu->memory[cpu->regs.esp--] = val; // set value, move down
}

void cpu_splitargs(uint16_t args, char *arg1, char *arg2) {
  *arg1 = (char) ((args >> 8) & 0xFF);
  *arg2 = (char) (args & 0xFF);
}

struct PackedInstr decode(char **stream) {

  char opcode = 0;
  uint16_t operands = 0;

  opcode = **stream & 0x3F; // remove upper two bits
  int numargs = (**stream & 0xC0) >> 6;
  switch (numargs) { // extract top 2 bits then downshift
    case 2:
      operands |= ((uint16_t) *++*stream) << 8; // get first arg, place at top
    case 1:
      operands |= (uint16_t) *++*stream; // get (first or second arg), place at bottom (if this is a single arg instruction, arg is in the lower byte)
      break;
    default:
      goto ERROR;
  }

  (*stream)++; // increment pointer (for next use)

  struct PackedInstr p;
  p.args = operands;

  switch (numargs) {
    case 0:
      p.i = noArgs(opcode);
      break;
    case 1:
      p.i = oneArg(opcode);
      break;
    case 2:
      p.i = twoArgs(opcode);
      break;
    default:
      goto ERROR;
  }

  return p;

  ERROR:
    exit(1);
}

instruction noArgs(char opcode) {
  switch (opcode) {
    case 0:
      return nop;
    case 1:
      return ret;
    case 2:
      return call;
    default:
      goto ERROR;
  }

  ERROR:
    printf("[ERROR] Invalid 0 args instruction found, ID: %d\n", (int) opcode);
    exit(1);
}

instruction oneArg(char opcode) {
  switch (opcode) {
    case 0:
      return sjmp;
    case 1:
      return psh;
    case 2:
      return pop;
    default:
      goto ERROR;
  }

  ERROR:
    printf("[ERROR] Invalid 1 arg instruction found, ID: %d\n", (int) opcode);
    exit(1);
}

instruction twoArgs(char opcode) {
  switch (opcode) {
    case 0:
      return tst;
    case 1:
      return mov;
    default:
      goto ERROR;
  }

  ERROR:
    printf("[ERROR] Invalid 2 args instruction found, ID: %d\n", (int) opcode);
    exit(1);
}

void halt(OPERATION_I){
  cpu->flags.halt = false;
}

void nop(OPERATION_I) {
  // yo it's a no op
}

void ret(OPERATION_I) {
  size_t upper = (size_t) cpu_popstack(cpu); // do this because memory is bytes and we need a 16 bit pointer
  size_t lower = (size_t) cpu_popstack(cpu);
  cpu->instructions = (char *) ((upper << 8) + lower);
}

void call(OPERATION_I) {
  size_t address_upper = (size_t) cpu_popstack(cpu); // get location to jump to
  size_t address_lower = (size_t) cpu_popstack(cpu);

  size_t current_address = (size_t) cpu->instructions; // get return address
  size_t return_lower = current_address & 0xff;
  size_t return_upper = (current_address >> 8) & 0xff;

  cpu_pushstack(cpu, (char) return_upper);
  cpu_pushstack(cpu, (char) return_lower);

  cpu->instructions = (char *) ((address_upper << 8) + address_lower); // set instruction pointer
}

void sjmp(OPERATION_I) {
 // TODO: this
}
