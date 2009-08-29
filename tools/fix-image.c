/******************************************************
 * fix-image.c
 *
 * This is used to convert an image to 32-bit format.
 * As part of the build system, it detects the host
 * native word size (used by Toka), and only does the
 * conversion if the native word size is not 32-bits.
 ******************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>


long input[5000000];
int  output[5000000];

int load_image64(char *image)
{
  FILE *fp;
  int x;

  if ((fp = fopen(image, "rb")) == NULL)
  {
    fprintf(stderr, "Sorry, but I couldn't open %s\n", image);
    exit(-1);
  }

  x = fread(input, sizeof(long), 5000000, fp);
  fclose(fp);

  return x;
}

int save_image(char *image, int cells)
{
  FILE *fp;
  int x;

  if ((fp = fopen(image, "w")) == NULL)
  {
    fprintf(stderr, "Sorry, but I couldn't open %s\n", image);
    exit(-1);
  }

  x = fwrite(output, sizeof(int), cells, fp);
  fclose(fp);
  return x;
}

void convert64to32(char *s)
{
  int i, cells;
  if (sizeof(int) == sizeof(long))
  {
    printf(" No\n\n");
    exit(0);
  }
  printf(" Yes\n");

  fprintf(stderr, "Loading...\n");
  cells = load_image64(s);

  fprintf(stderr, "Converting...\n");
  for (i = 0; i < 5000000; i++)
     output[i] = (int)input[i];

  fprintf(stderr, "Saving...\n\n");
  save_image(s, cells);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "%s imagename\n", argv[0]);
    exit(-1);
  }

  printf("Checking to see if image needs to be converted to 32-bit...");
  convert64to32(argv[1]);

  return 0;
}
