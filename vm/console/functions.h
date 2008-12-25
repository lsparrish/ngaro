#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

/* FILE: devices.c */
int dev_getch();
void dev_refresh();
void dev_include(char *s);
void dev_putch(int c);
void dev_init(int level);
void dev_cleanup();

/* FILE: disassemble.c */
void display_instruction();

/* FILE: endian.c */
void swapEndian();

/* FILE: initial_image.c */
void initial_image();

/* FILE: loader.c */
int vm_load_image(char *image);
int vm_save_image(char *image);

/* FILE: ngaro.c */
int main(int argc, char **argv);

/* FILE: video.c */
void drawpixel(int x, int color);
int update_display(void *unused);

/* FILE: vm.c */
void init_vm();
void vm_process();
void init_devices();
void cleanup_devices();

#endif
