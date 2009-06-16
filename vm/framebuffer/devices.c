/******************************************************
 * Ngaro
 *
 *|F|
 *|F| FILE: devices.c
 *|F|
 *
 * Written by Charles Childers, released into the public
 * domain
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "vm.h"

#include "SDL.h"
#include "SDL_keysym.h"

/* From vm.c */
extern VM_STATE vm;


typedef struct {
  SDL_Surface *screen;
  SDL_Surface *font;
  int mouse_x;
  int mouse_y;
} DEVICES;

DEVICES io;


/******************************************************
 *|F| void draw_character(int x, int y, int character)
 ******************************************************/
void drawcharacter(int x, int y, int character)
{
  int i, j, screenofs, charofs;

  /* The Ngaro font does not have the first 33    */
  /* characters, so if the requested character is */
  /* less than this, ignore the request           */
  if (character <= 32)
    return;

  character -= 33;

  /* Render the character to the framebuffer*/
  for (i = 0; i < io.font->w; i++)
  {
    screenofs = x + (y + i) * io.screen->pitch;
    charofs = (i + character * io.font->w) * io.font->pitch;
    for (j = 0; j < io.font->w; j++)
    {
//      if ((int)((char *)io.font->pixels)[charofs] != 0)
      vm.image[screenofs+VIDEO_BASE] = (int)((char *)io.font->pixels)[charofs];
      screenofs++;
      charofs++;
    }
  }
}



/******************************************************
 *|F| int handle_devices(void *unused)
 ******************************************************/
int handle_devices(void *unused)
{
  SDL_Event event;
  int x, y, c;

  while (vm.ports[0] == 0)
  {
    if (SDL_PollEvent(&event) != 0)
    {
      switch(event.type)
      {
        case SDL_KEYDOWN:
             vm.ports[1] = (int)event.key.keysym.unicode;
             if (event.key.keysym.sym == SDLK_BACKSPACE)
               vm.ports[1] = 8;
             vm.ports[0] = 1;
             break;
        case SDL_KEYUP:
             break;
        case SDL_QUIT:
             vm.ip = IMAGE_SIZE;
             vm.ports[0] = 1;
             break;
        case SDL_MOUSEMOTION:
             io.mouse_x = event.motion.x;
             io.mouse_y = event.motion.y;
             break;
      }
    }
    if (vm.ports[2] == 1)
    {
      c = TOS; DROP
      y = TOS; DROP
      x = TOS; DROP
      if (c >= 0)
        drawcharacter(x, y, c);
      else
        clear_display();
      vm.ports[2] = 0;
      vm.ports[0] = 1;
    }
    if (vm.ports[4] == 1)
    {
      vm_save_image(vm.filename);
      vm.ports[4] = 0;
      vm.ports[0] = 1;
    }

    /* Capabilities */
    if (vm.ports[5] == -1)
    {
      vm.ports[5] = IMAGE_SIZE;
      vm.ports[0] = 1;
    }
    if (vm.ports[5] == -2)
    {
      vm.ports[5] = VIDEO_BASE;
      vm.ports[0] = 1;
    }
    if (vm.ports[5] == -3)
    {
      vm.ports[5] = VIDEO_WIDTH;
      vm.ports[0] = 1;
    }
    if (vm.ports[5] == -4)
    {
      vm.ports[5] = VIDEO_HEIGHT;
      vm.ports[0] = 1;
    }
    if (vm.ports[5] == -5)
    {
      vm.ports[5] = vm.sp;
      vm.ports[0] = 1;
    }
    if (vm.ports[5] == -6)
    {
      vm.ports[5] = vm.rsp;
      vm.ports[0] = 1;
    }

    /* Mouse Events */
    if (vm.ports[12] == -1)
    {
      vm.sp++; vm.data[vm.sp] = io.mouse_x;
      vm.sp++; vm.data[vm.sp] = io.mouse_y;
      vm.ports[12] = 0;
      vm.ports[0] = 1;
    }
  }
  return 0;
}



/******************************************************
 *|F| void init_devices()
 ******************************************************/
void init_devices()
{
  SDL_Surface *temp;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }

  /* This causes each key value returned to be unique. */
  SDL_EnableUNICODE(1);

  /* We use an 8-bit display mode. */
  io.screen = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 8, SDL_SWSURFACE);
  SDL_WM_SetCaption("Ngaro", "Ngaro");

  /* Try to load the font. We look in the current directory and in */
  /* /usr/share/ngaro. If we can't find it, report an error and exit. */
  temp = SDL_LoadBMP("font.bmp");
  if (temp == 0)
    temp = SDL_LoadBMP("/usr/share/ngaro/font.bmp");
  if (temp == 0)
  {
    fprintf(stderr, "FATAL: Unable to locate font.bmp!\n");
    exit(1);
  }

  io.font = SDL_ConvertSurface(temp, io.screen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);

  io.mouse_x = 0;
  io.mouse_y = 0;
}



/******************************************************
 *|F| void cleanup_devices()
 ******************************************************/
void cleanup_devices()
{
}


/******************************************************
 *|F| void drawpixel(int x, int color)
 ******************************************************/
void drawpixel(int x, int color)
{
  unsigned char *ptr = (unsigned char*)io.screen->pixels;
  ptr[x] = (char)color;
}



/******************************************************
 *|F| int update_display(void *unused)
 ******************************************************/
int update_display(void *unused)
{
  int i;
  if (vm.ports[3] == 0)
  {
    for (i = VIDEO_BASE; i < (VIDEO_BASE+(VIDEO_WIDTH*VIDEO_HEIGHT)); i++)
       drawpixel(i-VIDEO_BASE, vm.image[i]);
    SDL_Flip(io.screen);
    vm.ports[3] = 1;
  }
  return 0;
}


/******************************************************
 *|F| void clear_display()
 ******************************************************/
void clear_display()
{
  int i;

  for (i = VIDEO_BASE; i < (VIDEO_BASE+(VIDEO_WIDTH*VIDEO_HEIGHT)); i++)
    vm.image[i] = 0;

  vm.ports[3] = 0;
}
