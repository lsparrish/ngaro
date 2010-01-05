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


/* track how many times each opcode is run */
#define LAST_OP (NUM_OPS)
#define ILL_OP (NUM_OPS + 1)
#define TRACKED_OPS (NUM_OPS + 2)
unsigned int op_count[TRACKED_OPS];

/* track number of instructions between (potentially)
   branching ones, jumps calls and returns.  32 is
   ridiculously large actually.  Somewhere around 6-8
   there is a sharp knee. */
#define MAX_SLL (32)

/* +2: 0 1 ... if 3 is max, I am also using 4. */
/* to represent "more than 3".  And  3+2 == 5 */
#define TRACKED_SLLS (MAX_SLL + 2)
unsigned int sll_count[TRACKED_SLLS];
unsigned int straight_line_length = -1;

/* track how many times each literal value is used */
#define MIN_LIT (-1024)
#define MAX_LIT (8192)

/* +3: 1 for 0, 1 for below range, 1 for above range */
#define TRACKED_LITS (MAX_LIT - MIN_LIT + 3)

/* OFF_LITS + (MIN_LIT - 1) == 0 */
#define OFF_LITS (1 - MIN_LIT)
/* I don't mind 32 element arrays we don't use, but 9k
 * is too much.  So use a pointer and malloc. */
unsigned int *lit_count = 0;

/* track how many times each jump distance is used */
#define MIN_DIST (-64)
#define MAX_DIST (2564)
#define TRACKED_DISTS (MAX_DIST - MIN_DIST + 3)
#define OFF_DISTS (1 - MIN_DIST)
unsigned int dist_count[TRACKED_DISTS];

/* track how deep the stacks are */
#define MIN_SP (-3)
#define MAX_SP (100)
#define TRACKED_SPS (MAX_SP - MIN_SP + 3)
#define OFF_SPS (1 - MIN_SP)
unsigned int dsp_count[TRACKED_SPS];
unsigned int rsp_count[TRACKED_SPS];

/* track call destinations */
unsigned int tracked_calls = 0;
unsigned int *call_count = 0;
unsigned int call_stats_please = 0; /* no call statistics by default */


void init_stats(FILE **opstats, char *opstat_path, int call_tracking_requested)
{
  int a;
  long int last_header_pos = -1;
  char *line_buffer;
  size_t line_buf_size = 1024;

  if ((! opstats) || (! opstat_path)) return;

  if (call_tracking_requested)
  {
    call_stats_please = 1;
    if (! call_count)
    {
      tracked_calls = 10; /* it will grow */
      call_count = (unsigned int *)malloc(tracked_calls * sizeof(*call_count));
    }
    if (! call_count)
    {
      fprintf(stderr,
              "Sorry, can't allocate the structure to count call targets.\n"
              "Not collecting any stats  this run.\n");
      return;
    }
  }

  if (! lit_count)
    lit_count = (unsigned int *)malloc(TRACKED_LITS * sizeof(*lit_count));
  if (! lit_count)
  {
    fprintf(stderr,
            "Sorry, can't allocate the structure to count lits.\n"
            "Not collecting any stats  this run.\n");
    return;
  }

  if (! (*opstats))
    *opstats = fopen(opstat_path, "w+");
  if (! (*opstats))
  {
    fprintf(stderr,
            "Sorry, can't open %s to save op code statistics.\n"
            "Not collecting any stats  this run.\n", opstat_path);
    free(lit_count);
    lit_count = 0;
    return;
  }
  for (a = 0; a < TRACKED_OPS; ++a)     op_count[a] = 0;
  for (a = 0; a < TRACKED_SPS; ++a)    rsp_count[a] = dsp_count[a] = 0;
  for (a = 0; a < TRACKED_SLLS; ++a)   sll_count[a] = 0;
  for (a = 0; a < TRACKED_LITS; ++a)   lit_count[a] = 0;
  for (a = 0; a < TRACKED_DISTS; ++a) dist_count[a] = 0;
  for (a = 0; a < tracked_calls; ++a) call_count[a] = 0;
}

void collect_stats(VM *vm)
{
  /* track frequency of various op codes */
  int opcode = vm->image[vm->ip];
  if ((opcode < 0) || (LAST_OP < opcode))
    opcode = ILL_OP;
  op_count[opcode] += 1;

  /* track distribution of straight line code lengths */
  switch (opcode)
  {
    case VM_GT_JUMP:  case VM_LT_JUMP:  case VM_NE_JUMP:
    case VM_EQ_JUMP:  case VM_JUMP:
         /* track jump distances */
         {
             int dist = vm->image[vm->ip + 1];
             dist -= vm->ip;
             if      (dist < MIN_DIST) dist = MIN_DIST - 1;
             else if (MAX_DIST < dist) dist = MAX_DIST + 1;
             dist_count[dist + OFF_DISTS] += 1;
         }
         /* FALL THROUGH! */
    case VM_CALL:     case VM_RETURN:   case VM_ZERO_EXIT:
         if (0 <= straight_line_length)
         {
           if (MAX_SLL < straight_line_length)
             straight_line_length = MAX_SLL + 1;
           sll_count[straight_line_length] += 1;
         }
         straight_line_length = 0;
         break;

    default:
         straight_line_length += 1;
         break;
  }

  /* track distribution of literals */
  if (VM_LIT == opcode)
  {
    int lit = vm->image[vm->ip + 1];
    if      (lit < MIN_LIT) lit = MIN_LIT - 1;
    else if (MAX_LIT < lit) lit = MAX_LIT + 1;
    lit_count[lit + OFF_LITS] += 1;
  }

  if (call_count)
  {
    /* track distribution of stack depths */
    int dsp = vm->sp;
    if      (dsp < MIN_SP) dsp = MIN_SP - 1;
    else if (MAX_SP < dsp) dsp = MAX_SP + 1;
    dsp_count[dsp + OFF_SPS] += 1;
    int rsp = vm->rsp;
    if      (rsp < MIN_SP) rsp = MIN_SP - 1;
    else if (MAX_SP < rsp) rsp = MAX_SP + 1;
    rsp_count[rsp + OFF_SPS] += 1;

    /* track distribution of call targets */
    if (VM_CALL == opcode)
    {
      int target = vm->image[vm->ip + 1];
      if (tracked_calls < target)
      {
        int newsize = target + 10;
        int i;
        call_count = (unsigned int *)realloc(call_count, newsize*sizeof(*call_count));
        if (! call_count)
        {
          fprintf(stderr,
                  "Sorry, can't grow the structure to count call targets.\n"
                  "aborting this run.\n");
          exit(1);
        }
        for (i = tracked_calls; i < newsize; ++i) call_count[i] = 0;
        tracked_calls = newsize;
      }
      call_count[target] += 1;
    }
  }

  /* $TODO$ read existing stats file and accumulate */

  /* $TODO$ ? track distribution of call lengths.
            are most calls also local?  how local?? */

  /* $TODO$ ? track total time spent executing
            different opcodes.  No matter how popular NOP
            is, it doesn't matter if it only uses 1% of
            runtime -- optimize something else. */
}

void report_stats(FILE *opstats)
{
  int a;
  char *opname[TRACKED_OPS] = {
    "VM_NOP",       "VM_LIT",       "VM_DUP",       "VM_DROP",
    "VM_SWAP",      "VM_PUSH",      "VM_POP",       "VM_CALL",
    "VM_JUMP",      "VM_RETURN",    "VM_GT_JUMP",   "VM_LT_JUMP",
    "VM_NE_JUMP",   "VM_EQ_JUMP",   "VM_FETCH",     "VM_STORE",
    "VM_ADD",       "VM_SUB",       "VM_MUL",       "VM_DIVMOD",
    "VM_AND",       "VM_OR",        "VM_XOR",       "VM_SHL",
    "VM_SHR",       "VM_ZERO_EXIT", "VM_INC",       "VM_DEC",
    "VM_IN",        "VM_OUT",       "VM_WAIT",
    "VM_ILLEGAL" };
  /* The max 32 bit unsigned int is 4,294,967,295 so */
  /* I am using %10u and %10d below.  6 places to change */
  /* if xx_count[] isn't a 32 bit int */

  /* report frequency of the opcodes. */
  fprintf(opstats, "   times run | code | op_name\n");
  for (a = 0; a < TRACKED_OPS; ++a)
  {
    fprintf(opstats, "  %10u |  %2x  | %s\n", op_count[a], a, opname[a]);
  }
  fprintf(opstats, "\n");

  /* report freq. of the sizes of the straight line code blocks. */
  fprintf(opstats, "  times seen | straight line code length\n");
  for (a = 0; a < TRACKED_SLLS; ++a)
  {
    if (sll_count[a])
    {
      if (MAX_SLL + 1 == a)
        fprintf(opstats, "  %10d | > %3d\n", sll_count[a], (a - 1));
      else
        fprintf(opstats, "  %10d |   %3d\n", sll_count[a], a);
    }
  }
  fprintf(opstats, "\n");

  /* report freq. of usage of the various literals. */
  fprintf(opstats, "  times used | literal value\n");
  for (a = 0; a < TRACKED_LITS; ++a)
  {
    if (lit_count[a])
    {
      if (0 == a)
        fprintf(opstats, "  %10d | < %4d\n", lit_count[a], ((a - OFF_LITS) + 1));
      else if (TRACKED_LITS - 1 == a)
        fprintf(opstats, "  %10d | > %4d\n", lit_count[a], ((a - OFF_LITS) - 1));
      else
        fprintf(opstats, "  %10d |   %4d\n", lit_count[a], (a - OFF_LITS));
    }
  }
  fprintf(opstats, "\n");

  /* report freq. of usage of the various jump distances. */
  fprintf(opstats, "  times used | jump distance\n");
  for (a = 0; a < TRACKED_DISTS; ++a)
  {
    if (dist_count[a])
    {
      if (0 == a)
        fprintf(opstats, "  %10d | < %4d\n", dist_count[a], ((a - OFF_DISTS) + 1));
      else if (TRACKED_DISTS - 1 == a)
        fprintf(opstats, "  %10d | > %4d\n", dist_count[a], ((a - OFF_DISTS) - 1));
      else
        fprintf(opstats, "  %10d |   %4d\n", dist_count[a], (a - OFF_DISTS));
    }
  }
  fprintf(opstats, "\n");

  if (call_count)
  {
    /* report distribution of stack depths. */
    fprintf(opstats, "  times seen | data stack depth\n");
    for (a = 0; a < TRACKED_SPS; ++a)
    {
      if (dsp_count[a])
      {
        if (0 == a)
          fprintf(opstats, "  %10d | < %4d\n", dsp_count[a], ((a - OFF_SPS) + 1));
        else if (TRACKED_SPS - 1 == a)
          fprintf(opstats, "  %10d | > %4d\n", dsp_count[a], ((a - OFF_SPS) - 1));
        else
          fprintf(opstats, "  %10d |   %4d\n", dsp_count[a], (a - OFF_SPS));
      }
    }
    fprintf(opstats, "\n");
    fprintf(opstats, "  times seen | return/address stack depth\n");
    for (a = 0; a < TRACKED_SPS; ++a)
    {
      if (rsp_count[a])
      {
        if (0 == a)
          fprintf(opstats, "  %10d | < %4d\n", rsp_count[a], ((a - OFF_SPS) + 1));
        else if (TRACKED_SPS - 1 == a)
          fprintf(opstats, "  %10d | > %4d\n", rsp_count[a], ((a - OFF_SPS) - 1));
        else
          fprintf(opstats, "  %10d |   %4d\n", rsp_count[a], (a - OFF_SPS));
      }
    }
    fprintf(opstats, "\n");

    /* report freq. of the various call targets. */
    fprintf(opstats, "times called | decimal address\n");
    for (a = 0; a < tracked_calls; ++a)
    {
      if (call_count[a])
      {
        fprintf(opstats, "  %10d | %5d\n", call_count[a], a);
      }
    }
    fprintf(opstats, "\n");
    free(call_count);
    call_count = 0;
    tracked_calls = 0;
  }

  /* clean up */
  free(lit_count);
  lit_count = 0;
  fclose(opstats);
}

/******************************************************
 * Main entry point into the VM
 ******************************************************/
int main(int argc, char **argv)
{
  int a, i, trace, endian;
  char *opstat_path = 0;
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
      i++; opstat_path = argv[i];
      init_stats(&opstats, opstat_path, call_stats_please);
    }
    else if (strcmp(argv[i], "--callstats") == 0)
    {
      call_stats_please = 1;
      if (opstat_path)
      {
        init_stats(&opstats, opstat_path, call_stats_please);
      }
    }
    else if (strcmp(argv[i], "--shrink") == 0)
    {
      vm->shrink = 1;
    }
    else if ((strcmp(argv[i], "--help") == 0) ||
             (strcmp(argv[i], "-help") == 0)  ||
             (strcmp(argv[i], "/help") == 0)  ||
             (strcmp(argv[i], "/?") == 0)     ||
             (strcmp(argv[i], "/h") == 0)     ||
             (strcmp(argv[i], "-h") == 0))
    {
      fprintf(stderr, "%s [options] [imagename]\n", argv[0]);
      fprintf(stderr, "Valid options are:\n");
      fprintf(stderr, "   --about          Display some information about Ngaro\n");
      fprintf(stderr, "   --trace          Execution trace\n");
      fprintf(stderr, "   --endian         Load an image with a different endianness\n");
      fprintf(stderr, "   --shrink         Shrink the image to the current heap size when saving\n");
      fprintf(stderr, "   --with [file]    Treat [file] as an input source\n");
      fprintf(stderr, "   --opstats [file] Write statistics about VM operations to [file]\n");
      fprintf(stderr, "      --callstats      Include how many times each address is called (slow)\n");
      fprintf(stderr, "                       Also includes distribution of stack depths.\n");
      exit(0);
    }
    else if ((strcmp(argv[i], "--about") == 0) ||
             (strcmp(argv[i], "--version") == 0))
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
    if (trace == 0)
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        collect_stats(vm);
        vm_process(vm);
      }
    }
    else
    {
      for (vm->ip = 0; vm->ip < IMAGE_SIZE; vm->ip++)
      {
        collect_stats(vm);
        display_instruction(vm);
        vm_process(vm);
      }
    }
    report_stats(opstats);
  }

  /* Once done, cleanup */
  dev_cleanup();
  return 0;
}
