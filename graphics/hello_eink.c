#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/kd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <string.h>

#include <asm/types.h>
typedef __u8 u8;

#include "linux/fb.h"
#include "linux/einkfb.h"
#include "linux/mxcfb.h"

/* 
  this program is based on:

    https://github.com/CrazyCoder/coolreader-kindle-qt/blob/master/drivers/QKindleFb/qkindlefb.cpp

  linestep():
    Returns the length of each scanline of the framebuffer in bytes.

  base():
    Returns a pointer to the beginning of the framebuffer.

  from:
    http://qt-project.org/doc/qt-4.8/qscreen.html#linestepx

*/

void show_info(struct fb_fix_screeninfo *pfinfo, struct fb_var_screeninfo *pvinfo) {

  struct fb_var_screeninfo vinfo = *pvinfo;
  struct fb_fix_screeninfo finfo = *pfinfo;
  
  printf("finfo: ") ;
  printf("   id=%s", (char *)&finfo.id) ;
  printf("   smem_start=%08x", (int)finfo.smem_start) ;
  printf("   smem_len=%d", finfo.smem_len) ;
  printf("   type=%d", finfo.type) ;
  printf("   type_aux=%d", finfo.type_aux) ;
  printf("   visual=%d", finfo.visual) ;
  printf("   xpanstep=%d", finfo.xpanstep) ;
  printf("   ypanstep=%d", finfo.ypanstep) ;
  printf("   ywrapstep=%d", finfo.ywrapstep) ;
  printf("   line_length=%d", finfo.line_length) ;
  printf("   mmio_start=%08x", (int)finfo.mmio_start) ;
  printf("   mmio_len=%d", finfo.mmio_len) ;
  printf("   accel=%d", finfo.accel) ;
  
  printf("vinfo: ") ;
  printf("   xres=%d", vinfo.xres) ;
  printf("   yres=%d", vinfo.yres) ;
  printf("   xres_virt=%d", vinfo.xres_virtual) ;
  printf("   yres_virt=%d", vinfo.yres_virtual) ;
  printf("   xoffset=%d", vinfo.xoffset) ;
  printf("   yoffset=%d", vinfo.yoffset) ;
  
  printf("   bitsperpixel=%d", vinfo.bits_per_pixel) ;
  printf("   grayscale=%d", vinfo.grayscale) ;
  
  printf("   nonstd=%d", vinfo.nonstd) ;
  printf("   activate=%d", vinfo.activate) ;
  
  printf("   height=%d", vinfo.height) ;
  printf("   width=%d", vinfo.width) ;
  
  printf("   flags=%d", vinfo.accel_flags) ;
  
  /* Timing: All values in pixclocks, except pixclock (of course) */
  printf("   pixclk=%d", vinfo.pixclock) ;
  printf("   lmargin=%d", vinfo.left_margin) ;
  printf("   rmargin=%d", vinfo.right_margin) ;
  printf("   umargin=%d", vinfo.upper_margin) ;
  printf("   bmargin=%d", vinfo.lower_margin) ;
  printf("   hsync_len=%d", vinfo.hsync_len) ;
  printf("   vsync_len=%d", vinfo.vsync_len) ;
  printf("   sync=%d", vinfo.sync) ;
  printf("   vmode=%d", vinfo.vmode) ;
  printf("   rotate=%d", vinfo.rotate) ;
  
  printf("Red %d %d %d",vinfo.red.offset,vinfo.red.length,
         vinfo.red.msb_right);
  printf("Green %d %d %d",vinfo.green.offset,vinfo.green.length,
         vinfo.green.msb_right);
  printf("Blue %d %d %d",vinfo.blue.offset,vinfo.blue.length,
         vinfo.blue.msb_right);
  printf("Transparent %d %d %d",vinfo.transp.offset,vinfo.transp.length,
         vinfo.transp.msb_right);
}

// not sure what the argument does
void blank(int fd, int on) {
  ioctl(fd, FBIOBLANK, on ? 1 : 0);
}

void invert_rect(int fd, int x0, int y0, int x1, int y1) {
  update_area_t ua;
  ua.x1 = x0;
  ua.y1 = y0;
  ua.x2 = x1 + 1;
  ua.y2 = y1 + 1;
  ua.which_fx = fx_invert ;
  ua.buffer = NULL;
  
  ioctl(fd, FBIO_EINK_UPDATE_DISPLAY_AREA, &ua);
}



void usage(FILE* f) {
  fprintf(f, "Usage: hello_ink framebuffer_device\n");
  fprintf(f, "\n");
  fprintf(f, "  example: hello_ink /dev/fd0\n");
  fprintf(f, "\n");
}


int main(int argc, char** argv) {
  int fd;
  struct fb_fix_screeninfo finfo;
  struct fb_var_screeninfo vinfo;
  int kindle_driver; // 1 if kindle diver. 0 if generic driver
 
  if(argc != 2) {
    usage(stderr);
    return 1;
  }
  fd = open(argv[1], O_RDWR);
  if(fd == -1) {
    fprintf(stderr, "Error: Failed to open framebuffer device\n");
    return 1;
  }
  
  memset(&vinfo, 0, sizeof(vinfo));
  memset(&finfo, 0, sizeof(finfo));

  // Get fixed screen information
  if(ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
    perror("ioctl FBIOGET_FSCREENINFO");
    return 1;
  }

  kindle_driver = strcmp(finfo.id, "eink_fb") ? 0 : 1;

  // Get variable screen information 
  if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
    perror("ioctl FBIOGET_VSCREENINFO");
    return 1;
  }

  show_info(&finfo, &vinfo) ;
  
  blank(fd, 0);

  invert_rect(fd, 10, 10, 100, 100);

  return 0;
}
