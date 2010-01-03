/******************************************************
 * Ngaro
 * Copyright (C) 2008, 2009, Charles Childers
 ******************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "vm.h"


int opcount[NUM_OPS+2];

/******************************************************
 * Main entry point into the VM
 ******************************************************/
int main(int argc, char **argv)
{
  int a, i, trace, endian;
  char *opstat_file = 0;
  FILE *opstats = 0;

  VM *vm = malloc(sizeof(VM));

  trace = 0;
  endian = 0;

  strcpy(vm->filename, "retroImage");

  init_vm(vm);
  dev_init(INPUT);

  vm->shrink = 0;

  /* Parse the command line arguments */
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--trace") == 0)
    {
      trace = 1;
    }
    else if (strcmp(argv[i], "--endian") == 0)
    {
      endian = 1;
    }
    else if (strcmp(argv[i], "--with") == 0)
    {
      i++; dev_include(argv[i]);
    }
    else if (strcmp(argv[i], "--opstats") == 0)
    {
      i++; opstat_file = argv[i];
    }
    else if (strcmp(argv[i], "--shrink") == 0)
    {
      vm->shrink = 1;
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      fprintf(stderr, "%s [options] [imagename]\n", argv[0]);
      fprintf(stderr, "Valid options are:\n");
      fprintf(stderr, "   --about          Display some information about Ngaro\n");
      fprintf(stderr, "   --trace          Execution trace\n");
      fprintf(stderr, "   --endian         Load an image with a different endianness\n");
      fprintf(stderr, "   --shrink         Shrink the image to the current heap size when saving\n");
      fprintf(stderr, "   --with [file]    Treat [file] as an input source\n");
      fprintf(stderr, "   --opstats [file] Write statistics about VM opcode to [file]\n");
      exit(0);
    }
    else if (strcmp(argv[i], "--about") == 0)
    {
      fprintf(stderr, "Retro Language  [VM: C, console]\n\n");
      exit(0);
    }
    else
    {
      strcpy(vm->filename, argv[i]);
    }
  }

  dev_init(OUTPUT);

  a = vm_load_image(vm, vm->filename);
  if (a == -1)
  {
    dev_cleanup();
    printf("Sorry, unable to find %s\n", vm->filename);
    exit(1);
  }

  /* Swap endian if --endian was passed */
  if (endian == 1)
    swapEndian(vm);

  /* Process the image */

  if (opstat_file)
  {
    opstats = fopen(opstat_file, "w");
    if (!opstats)
    {
      fprintf(stderr, "Sorry, can't open %s to save op code statistics.\n", opstat_file);
    }
  }
  if (opstats == 0)
  {
    if (trace == 0)
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        vm_process(vm);
      }
    }
    else
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        display_instruction(vm);
        vm_process(vm);
      }
    }
  }
  else
  {
    int opcode = 999;
    for (a = 0; a < NUM_OPS + 2; ++a) opcount[a] = 0;
    if (trace == 0)
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        opcode = vm->image[vm->ip];
        opcount[(opcode <= NUM_OPS)?(opcode):(NUM_OPS+1)] += 1;
        vm_process(vm);
      }
    }
    else
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        opcode = vm->image[vm->ip];
        opcount[(opcode <= NUM_OPS) ? (opcode) : (NUM_OPS + 1)] += 1;
        display_instruction(vm);
        vm_process(vm);
      }
    }
    {
      char *opname[NUM_OPS+2] = {
        "VM_NOP",       "VM_LIT",       "VM_DUP",       "VM_DROP",
        "VM_SWAP",      "VM_PUSH",      "VM_POP",       "VM_CALL",
        "VM_JUMP",      "VM_RETURN",    "VM_GT_JUMP",   "VM_LT_JUMP",
        "VM_NE_JUMP",   "VM_EQ_JUMP",   "VM_FETCH",     "VM_STORE",
        "VM_ADD",       "VM_SUB",       "VM_MUL",       "VM_DIVMOD",
        "VM_AND",       "VM_OR",        "VM_XOR",       "VM_SHL",
        "VM_SHR",       "VM_ZERO_EXIT", "VM_INC",       "VM_DEC",
        "VM_IN",        "VM_OUT",       "VM_WAIT",
        "VM_ILLEGAL" };
      fprintf(opstats, "   times run | code | op_name\n");
      for (a = 0; a < NUM_OPS + 2; ++a)
      {
        fprintf(opstats, "%12d |  %2x  | %s\n", opcount[a], a, opname[a]);
      }
      fprintf(opstats, "\n");
      fclose(opstats);
    }
  }

  /* Once done, cleanup */
  dev_cleanup();
  return 0;
}
