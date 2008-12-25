/******************************************************
 * Ngaro
 *
 *|F|
 *|F| FILE: ngaro.c
 *|F|
 *
 * Written by Charles Childers, released into the public
 * domain
 ******************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "vm.h"

extern VM_STATE vm;


/******************************************************
 *|F| int main(int argc, char **argv)
 ******************************************************/
int main(int argc, char **argv)
{
  int a, i, trace, endian;

  printf("Video @ %i\n", VIDEO_BASE);

  trace = 0;
  endian = 0;

  strcpy(vm.filename, "retroImage");

  init_vm();
  init_devices();

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
    else if (strcmp(argv[i], "--help") == 0)
    {
      fprintf(stderr, "%s [options] [imagename]\n", argv[0]);
      fprintf(stderr, "Valid options are:\n");
      fprintf(stderr, "   --trace    Execution trace\n");
      fprintf(stderr, "   --endian   Load an image with a different endianness\n");
      exit(0);
    }
    else
    {
      strcpy(vm.filename, argv[i]);
    }
  }


  /* Load the image */
  a = vm_load_image(vm.filename);

  if (a == -1)
    initial_image();

  /* Swap endian if --endian was passed */
  if (endian == 1)
    swapEndian();


  /* Process the image */
  if (trace == 0)
  {
    for (vm.ip = 0; vm.ip < IMAGE_SIZE; vm.ip++)
    {
      vm_process(vm.image[vm.ip]);
      update_display(0);
    }
  }
  else
  {
    for (vm.ip = 0; vm.ip < IMAGE_SIZE; vm.ip++)
    {
      display_instruction();
      vm_process(vm.image[vm.ip]);
      update_display(0);
    }
  }

  /* Once done, cleanup */
  cleanup_devices();
  return 0;
}
