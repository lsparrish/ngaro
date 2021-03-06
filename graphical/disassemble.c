/******************************************************
 * Ngaro
 *
 *|F|
 *|F| FILE: disassemble.c
 *|F|
 *
 * Written by Charles Childers, released into the public
 * domain
 ******************************************************/

#include <stdio.h>

#include "functions.h"
#include "vm.h"

extern VM_STATE vm;



/******************************************************
 *|F| void display_instruction()
 *
 * Attempt at writing some debugging functionality. This
 * is intended to display the current instruction and
 * the basic stack information as it exists when this
 * instruction is reached.
 *
 * Output takes a form like:
 *   AAAAA:   BBBBB                 <C> DDDDD ---- <E> FFFFF
 *
 * Where:
 *   AAAAA   IP Address
 *   BBBBB   Instruction
 *   C       Depth of data stack
 *   DDDDD   Data stack contents
 *   E       Depth of address stack
 *   FFFFF   Address stack contents
 ******************************************************/
void display_instruction()
{
  int i;
  char *instructions[] = { "nop",   "lit",   "dup",  "drop", "swap",  "push",         \
                           "pop",   "call",  "jump", "ret",  ">jump", "<jump",        \
                           "!jump", "=jump", "@",    "!",    "+",     "-",     "*",   \
                           "/mod",  "and",   "or",   "xor",  "<<",    ">>",    "0;",  \
                           "1+",    "1-",    "in",   "out",  "wait" };

  i = 0;

  /* Display the IP */
  fprintf(stderr, "%6i:   ", vm.ip);

  /* And the instruction, finding a name from the lookup table */
  if(vm.image[vm.ip] <= NUM_OPS)
    fprintf(stderr, "%5s", instructions[vm.image[vm.ip]]);
  else
    fprintf(stderr, "??? ");

  /* Some instructions take a following value. Display this if necessary. */
  if (vm.image[vm.ip] == VM_LIT || vm.image[vm.ip] == VM_CALL || vm.image[vm.ip] == VM_JUMP || \
      vm.image[vm.ip] == VM_LT_JUMP || vm.image[vm.ip] == VM_GT_JUMP || vm.image[vm.ip] == VM_EQ_JUMP || \
      vm.image[vm.ip] == VM_NE_JUMP)
  {
    fprintf(stderr, " %6i", vm.image[vm.ip+1]);
    i = 1;
  }

  /* Padding if following data isn't required. */
  if (i == 0)
    fprintf(stderr, "       ");

  fprintf(stderr, "\t\t");

  /* Display the data stack */
  fprintf(stderr, "<%i> ", vm.sp);
  for (i = vm.sp; i > 0; i--)
    fprintf(stderr, "%i ", vm.data[(vm.sp-i)+1]);

  /* And the address stack */
  fprintf(stderr, " ----  <%i> ", vm.rsp);
  for (i = vm.rsp; i > 0; i--)
    fprintf(stderr, "%i ", vm.address[(vm.rsp-i)+1]);

  /* Whew, all done! */
  fprintf(stderr, "\n");
}
