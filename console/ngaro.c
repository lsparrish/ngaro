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


/******************************************************
 * Main entry point into the VM
 ******************************************************/
int main(int argc, char **argv)
{
  int a, i, trace, endian;

  VM *vm = malloc(sizeof(VM));

  trace = 0;
  endian = 0;

  strcpy(vm->filename, "retroImage");

  init_vm(vm);
  dev_init(INPUT);

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
    else if (strcmp(argv[i], "--help") == 0)
    {
      fprintf(stderr, "%s [options] [imagename]\n", argv[0]);
      fprintf(stderr, "Valid options are:\n");
      fprintf(stderr, "   --about        Display some information about Ngaro\n");
      fprintf(stderr, "   --trace        Execution trace\n");
      fprintf(stderr, "   --endian       Load an image with a different endianness\n");
      fprintf(stderr, "   --with [file]  Treat [file] as an input source\n");
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

  /* Once done, cleanup */
  dev_cleanup();
  return 0;
}
