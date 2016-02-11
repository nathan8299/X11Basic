/* framebuffer.c      Grafik-Routinen fuer Framebuffer-Systeme (c) Markus Hoffmann    */


/* This file is part of X11BASIC, the basic interpreter for Unix/X
 * ======================================================================
 * X11BASIC is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */



#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sysexits.h>

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include "defs.h"
#include "window.h"
#include "framebuffer.h"

extern struct fb_var_screeninfo vinfo;
extern struct fb_fix_screeninfo finfo;

int fbfd = -1;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

G_CONTEXT screen;

const unsigned char mousealpha[16*16]={
  0,0,0,0,0,0,0,127,127,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
127,127,127,127,127,  0,127,127,127,127,127,127,127,127,127,127,
127,255,255,255,  0,  0,255,255,255,255,255,255,255,255,255,127,
127,255,255,255,  0,  0,255,255,255,255,255,255,255,255,255,127,
127,127,127,127,127,255,127,127,127,127,127,127,127,127,127,127,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,255,127,0,0,0,0,0,0,
  0,0,0,0,0,0,0,127,127,127,0,0,0,0,0,0};

const unsigned short mousepat[16*16]={
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,BLACK,WHITE,WHITE,BLACK,0,0,0,0,0,0,
BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,BLACK,WHITE,WHITE,127,127,127,127,127,127,127,
WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,
WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,
127,127,127,127,127,255,127,127,127,127,127,127,127,127,127,127,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0,
  0,0,0,0,0,0,0,BLACK,WHITE,BLACK,0,0,0,0,0,0};


unsigned short vmousepat[16*16];
unsigned char vmousealpha[16*16];


void Fb_Open() {
  fbfd=open(FB_DEVICENAME, O_RDWR);
  if (!fbfd) {
    printf("ERROR: could not open framebufferdevice.\n");
    exit(EX_OSFILE);
  }
#if DEBUG
  printf("Framebuffer device now opened.\n");
#endif
  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("ERROR: Could not get fixed screen information.\n");
    exit(EX_IOERR);
  }
  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("ERROR: Could not get variable screen information.\n");
    exit(EX_IOERR);
  }
  screen.bpp=vinfo.bits_per_pixel;
  screen.clip_w=screen.width=vinfo.xres;
  screen.clip_h=screen.height=vinfo.yres;
  screen.clip_x=screen.clip_y=0;
  screen.fcolor=YELLOW;
  screen.bcolor=BLACK;
  screen.mouse_ox=8;
  screen.mouse_oy=8;
  screen.mouse_x=vinfo.xres/2;
  screen.mouse_y=vinfo.yres/2;
  screen.mousemask=mousealpha;
  screen.mousepat=mousepat;
  screen.alpha=255;
  // Figure out the size of the screen in bytes
  screen.size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
  screen.scanline=vinfo.xres*vinfo.bits_per_pixel/8;
#if DEBUG
  printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
#endif

  // Map the device to memory
  screen.pixels = (char *)mmap(0, screen.size, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if((int)screen.pixels==-1) {
    printf("ERROR: Could not map framebuffer device to memory.\n");
    exit(EX_IOERR);
  }
  /* Now set the padding to zero, otherwise it can happen that nothing is visible
    if the pading was set to the second page...*/

  vinfo.xoffset=0;
  vinfo.yoffset=0;
  ioctl(fbfd,FBIOPAN_DISPLAY,&vinfo);
  FB_show_mouse();
}

void Fb_Close() {
  if(fbfd > 0) {
    munmap(screen.pixels, screen.size);
    close(fbfd);
  }
  fbfd = -1;
}

/* This is a low-level Function, need to be fast, but does noch check 
   clipping */

inline void FB_PutPixel_noclip(int x, int y, unsigned short color) {
  unsigned short *ptr  = (unsigned short*)(screen.pixels+x*2+y*screen.scanline);
  *ptr = color;
}
inline void FB_PutPixel_noclip_alpha(int x, int y, unsigned short color, unsigned char alpha) {
  unsigned short *ptr  = (unsigned short*)(screen.pixels+x*2+y*screen.scanline);
  *ptr = mix_color(color,*ptr,alpha);
}

inline void FB_PutPixel(int x, int y, unsigned short color) {
  if(x<screen.clip_x) return;
  if(y<screen.clip_y) return;
  if(x>=screen.clip_x+screen.clip_w) return;
  if(y>=screen.clip_y+screen.clip_h) return;
  
  unsigned short *ptr  = (unsigned short*)(screen.pixels+x*2+y*screen.scanline);
  if(screen.alpha==255)  *ptr = color;
  else *ptr = mix_color(color,*ptr,screen.alpha);
}



inline void Fb_Scroll(int target_y, int source_y, int height) {
  memmove(screen.pixels+target_y*screen.scanline,
          screen.pixels+source_y*screen.scanline,height*screen.scanline);
}

void FB_set_color(unsigned short color) {
  screen.fcolor=color;
}
void FB_set_bcolor(unsigned short color) {
  screen.bcolor=color;
}
void FB_set_alpha(unsigned char alpha) {
  screen.alpha=alpha;
}
void FB_set_textmode(unsigned int mode) {
  screen.textmode=mode;
}

void FB_set_clip(G_CONTEXT *screen,int x,int y,int w,int h) {
  screen->clip_x=x;
  screen->clip_y=y;
  screen->clip_w=w;
  screen->clip_h=h;
}
void FB_clip_off(G_CONTEXT *screen) {
  screen->clip_x=0;
  screen->clip_y=0;
  screen->clip_w=screen->width;
  screen->clip_h=screen->height;
}

void FB_plot(int x, int y) {
  FB_PutPixel(x,y,screen.fcolor);
}
unsigned short FB_point(int x, int y) {
  if(x<0 || y<0 || x>=screen.width || y>=screen.height) return;
  unsigned short *ptr  = (unsigned short*)(screen.pixels+x*2+y*screen.scanline);
  return(*ptr);
}
void DrawHorizontalLine(int X, int Y, int width, unsigned short color) {
  register int w = width; 

  if (Y<screen.clip_y) return;
  if (Y>=screen.clip_y+screen.clip_h) return;

  if (X<screen.clip_x)      // clip left margin
    { w-=(screen.clip_x-X); X=screen.clip_x; }
  if (w>screen.clip_x+screen.clip_w-X)    // clip right margin
    w=screen.clip_x+screen.clip_w-X;

  if(screen.alpha==255) {
    while(w-->0) FB_PutPixel_noclip(X++,Y,color);
  } else {
    while(w-->0) FB_PutPixel_noclip_alpha(X++,Y,color,screen.alpha);
  }
}
void DrawVerticalLine(int X, int Y, int height, unsigned short color) {
  register int h = height;  // in pixels

  if (X<screen.clip_x) return;
  if (X>=screen.clip_x+screen.clip_w) return;

  if (screen.clip_y>Y) { 
    h-=(screen.clip_y-Y); 
    Y=screen.clip_y; 
  }
  if(Y+h>screen.clip_y+screen.clip_h) h=screen.clip_y+screen.clip_h-X;
  if(screen.alpha==255) {
    while(h-->=0) FB_PutPixel_noclip(X,Y++,color);
  } else {
    while(h-->=0) FB_PutPixel_noclip_alpha(X,Y++,color,screen.alpha);
  }
}



void fillLine(int x1,int x2,int y,unsigned short color) {
 if(x2>=x1) DrawHorizontalLine(x1,y,x2-x1,color);
 else DrawHorizontalLine(x1,y,x1-x2,color);
}

/* Bresenham's line drawing algorithm single with */

void FB_DrawLine(int x0, int y0, int x1, int y1,unsigned short color) {
  int dy=y1-y0;
  int dx=x1-x0;
  int stepx,stepy;

  if(dy==0) {
    if(dx==0) {FB_PutPixel(x0, y0,color); return;}
    /* Swap x1, x2 if required */
    if(x0>x1) {int xtmp=x0;x0=x1;x1=xtmp;}
    DrawHorizontalLine(x0, y0,x1-x0,color); return;
  } 
  if(dy<0) {dy=-dy; stepy=-1;} else stepy=1;
  if(dx<0) {dx=-dx; stepx=-1;} else stepx=1;
  dy<<=1;	// dy is now 2*dy
  dx<<=1;	// dx is now 2*dx

  FB_PutPixel(x0, y0,color);

  if(dx>dy) {
    int fraction=dy-(dx>>1);     // same as 2*dy - dx
    while(x0!=x1) {
	if(fraction>=0) {
	  y0+=stepy;
	  fraction-=dx;	    // same as fraction -= 2*dx
	}
	x0+=stepx;
	fraction+=dy;	    // same as fraction -= 2*dy
	FB_PutPixel(x0,y0,color);
    }
  } else {
    int fraction=dx-(dy>>1);
    while(y0!=y1) {
      if(fraction>=0) {
        x0 += stepx;
        fraction -= dy;
      }
      y0 += stepy;
      fraction += dx;
      FB_PutPixel(x0, y0,color);
    }
  }
}
#include <math.h>
#define HYPOT(x,y) sqrt((double)(x)*(double)(x)+(double)(y)*(double)(y)) 

void FB_DrawThickLine(int x0, int y0, int x1, int y1,int width, unsigned short color) {
  int dy=y1-y0;
  int dx=x1-x0;
  int stepx,stepy;

  int ddx1,ddy1;
  int ddx2,ddy2;

  
  if(dx==0) {
    int i;
    /* Swap y1, y2 if required */
    if(y0>y1) {int ytmp=y0;y0=y1;y1=ytmp;}
    for (i=y0; i<=y1; i++) {
      DrawHorizontalLine(x0-width/2, i, width, color);
    }  
    return;
  }
  if(dy==0) {
    int i;
    /* Swap x1, x2 if required */
    if(x0>x1) {int xtmp=x0;x0=x1;x1=xtmp;}
    for (i=0; i<=width; i++) {
      DrawHorizontalLine(x0, y0-width/2+i,x1-x0, color);
    }  
    return;
  }


  if(dy<0) {dy=-dy; stepy=-1;} else stepy=1;
  if(dx<0) {dx=-dx; stepx=-1;} else stepx=1;

  width--;

  ddx1=(int)dy/HYPOT(dx,dy)*width/2;
  ddy1=(int)dx/HYPOT(dx,dy)*width/2;
  if(width&1) {
    ddx2=(int)(dy/HYPOT(dx,dy)*(width+1)/2);
    ddy2=(int)(dx/HYPOT(dx,dy)*(width+1)/2);
  } else {
    ddx2=ddx1;
    ddy2=ddy2;
  }
  
  dy<<=1;	// dy is now 2*dy
  dx<<=1;	// dx is now 2*dx


  FB_DrawLine(x0-ddx1*stepx,y0+ddy1*stepy,x0+ddx2*stepx,y0-ddy2*stepy,color);

  if(dx>dy) {
    int fraction=dy-(dx>>1);     // same as 2*dy - dx
    while(x0!=x1) {
	if(fraction>=0) {
	  y0+=stepy;
  FB_DrawLine(x0-ddx1*stepx,y0+ddy1*stepy,x0+ddx2*stepx,y0-ddy2*stepy,color);
	  fraction-=dx;	    // same as fraction -= 2*dx
	}
	x0+=stepx;
	fraction+=dy;	    // same as fraction -= 2*dy
  FB_DrawLine(x0-ddx1*stepx,y0+ddy1*stepy,x0+ddx2*stepx,y0-ddy2*stepy,color);

    }
  } else {
    int fraction=dx-(dy>>1);
    while(y0!=y1) {
      if(fraction>=0) {
        x0 += stepx;
  FB_DrawLine(x0-ddx1*stepx,y0+ddy1*stepy,x0+ddx2*stepx,y0-ddy2*stepy,color);

        fraction -= dy;
      }
      y0 += stepy;
      fraction += dx;
  FB_DrawLine(x0-ddx1*stepx,y0+ddy1*stepy,x0+ddx2*stepx,y0-ddy2*stepy,color);
    }
  }
}

void FB_line(int x1,int y1,int x2,int y2) {
  if(screen.linewidth>1) FB_DrawThickLine(x1,y1,x2,y2,screen.linewidth, screen.fcolor);
  else {
    if(y1==y2) {
      if(x2>=x1) DrawHorizontalLine(x1,y1,x2-x1,screen.fcolor);
      else DrawHorizontalLine(x2,y1,x1-x2,screen.fcolor);
    } else FB_DrawLine(x1,y1,x2,y2,screen.fcolor);
  }
}

void FB_lines(XPoint *points, int n, int mode) {
  int i;
  if(n>1) {
    for(i=1;i<n;i++)
      FB_line(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
  }
}
void FB_points(XPoint *points, int n, int mode) {
  int i;
  if(n) {
    for(i=0;i<n;i++) FB_plot(points[i].x,points[i].y);
  }
}


void FB_box(int x1,int y1,int x2,int y2) {
  int w,h;
  /* Swap x1, x2 if required */
  if(x1>x2) {int xtmp=x1;x1=x2;x2=xtmp;}
  /* Swap y1, y2 if required */
  if(y1>y2) {int ytmp=y1;y1=y2;y2=ytmp;}
  w=x2-x1;
  h=y2-y1;
  if(x1==x2) {
    if(y1==y2) FB_PutPixel(x1,y1,screen.fcolor);
    else {
      DrawVerticalLine(x1,y1,h,screen.fcolor);
    } 
  } else {
    if(y1==y2) DrawHorizontalLine(x1,y1,w,screen.fcolor);
    else {
      DrawHorizontalLine(x1,y1,w,screen.fcolor);
      DrawHorizontalLine(x1,y2,w,screen.fcolor);
      if(h>1) {
        DrawVerticalLine(x1,y1+1,h-1,screen.fcolor);
        DrawVerticalLine(x2,y1+1,h-1,screen.fcolor);
      }
    }
  }
}


void FillBox (int x, int y, int w, int h, unsigned short color) {
  int i;
  for (i=y; i<=y+h; i++) {
    DrawHorizontalLine(x, i, w, color);
  }  
}
void FB_pbox(int x1, int y1, int x2, int y2) {
  /* Swap x1, x2 if required */
  if(x1>x2) {int xtmp=x1;x1=x2;x2=xtmp;}
  /* Swap y1, y2 if required */
  if(y1>y2) {int ytmp=y1;y1=y2;y2=ytmp;}
  
  FillBox(x1,y1,x2-x1,y2-y1,screen.fcolor);
}


/* Clear whole screen */
void FB_Clear(G_CONTEXT *screen) {
  unsigned short *ptr=(unsigned short *)screen->pixels;
  int n=screen->size>>1;
  int i;
  for(i=0; i<n; i++) *ptr++=screen->bcolor;
}


void Fb_inverse(int x, int y,int w,int h){
  if(w<=0||h<=0) return;

  if(x<screen.clip_x||y<screen.clip_w|| x+w>screen.clip_x+screen.clip_w|| y+h>screen.clip_y+screen.clip_h) return;

  register unsigned short *ptr  = (unsigned short*)(screen.pixels+y*screen.scanline);
  ptr+=x;
  register unsigned short *endp  = ptr+h*screen.width;
  register int i;
  while (ptr<endp) {
    for(i=0;i<w;i++) *ptr++=~*ptr;
    ptr+=screen.width-w;
  }
}
unsigned short int mean_color(unsigned short int a, unsigned short int b) {
  return(
  (((((a>>11)&0x1f)+((b>>11)&0x1f))>>2)<<11)|
  (((((a>>5)&0x3f)+((b>>5)&0x3f))>>2)<<5)|
  (((a&0x1f)+(b&0x1f))>>2));
}
unsigned short int mix_color(unsigned short int a, unsigned short int b, unsigned char alpha) {
/*...*/
  if(alpha==255) return a;
  if(alpha==0) return b;
  unsigned long  R, G, B;
  R = ((b & R_MASK) + (((a & R_MASK) - (b & R_MASK)) * alpha >> 8)) & R_MASK;
  G = ((b & G_MASK) + (((a & G_MASK) - (b & G_MASK)) * alpha >> 8)) & G_MASK;
  B = ((b & B_MASK) + (((a & B_MASK) - (b & B_MASK)) * alpha >> 8)) & B_MASK;
  return(R | G | B);
}

void FB_copyarea(int x,int y,int w, int h, int tx,int ty) {
  if(x<0||y<0||w<=0||h<=0|| x+w>screen.width|| y+h>screen.height||
  tx<0||ty<0||tx+w>screen.width|| ty+h>screen.height) return;
  register unsigned short *ptr1  = (unsigned short*)(screen.pixels+y*screen.scanline);
  register unsigned short *ptr2  = (unsigned short*)(screen.pixels+ty*screen.scanline);
  ptr1+=x;
  ptr2+=tx;

  register int i;
  if(y>ty) {
    for(i=0;i<h;i++) {
       memmove(ptr2,ptr1,w*sizeof(short));
       ptr1+=screen.width;
       ptr2+=screen.width;
    }
  } else {
    for(i=h-1;i>=0;i--) {
      memmove(&ptr2[i*screen.width],&ptr1[i*screen.width],w*sizeof(short));
    }
  }
}


/* mouse routines */

void FB_show_mouse() {
  if(!screen.mouseshown) {
    FB_draw_sprite(screen.mousepat,screen.mousemask,screen.mouse_x-screen.mouse_ox,screen.mouse_y-screen.mouse_oy);
    screen.mouseshown=1;
  }
}
void FB_hide_mouse() {
  if(screen.mouseshown) {
    FB_hide_sprite(screen.mouse_x-screen.mouse_ox,screen.mouse_y-screen.mouse_oy);
    screen.mouseshown=0;
  }
}


/* Draw 16x16 Sprite */

unsigned short spritebg[16*16];  /*store the background*/

void FB_draw_sprite(unsigned short *sprite, unsigned char *alpha,int x,int y) {
  register unsigned short *ptr=(unsigned short*)(screen.pixels);
  int i,j;
  for(j=0;j<16;j++) {
    if(y+j>=screen.clip_y && y+j<screen.clip_y+screen.clip_h) {
      for(i=0;i<16;i++) {
        if(x+i>=screen.clip_x && x+i<screen.clip_x+screen.clip_w) {
          spritebg[16*j+i]=ptr[(y+j)*(screen.scanline>>1)+x+i];
	  ptr[(y+j)*(screen.scanline>>1)+x+i]=mix_color(sprite[16*j+i],
	    ptr[(y+j)*(screen.scanline>>1)+x+i],alpha[16*j+i]);
        }
      }
    }
  }
}
void FB_hide_sprite(int x,int y) {
  register unsigned short *ptr=(unsigned short*)(screen.pixels);
  int i,j;
  for(j=0;j<16;j++) {
    if(y+j>=screen.clip_y && y+j<screen.clip_y+screen.clip_h) {
      for(i=0;i<16;i++) {
        if(x+i>=screen.clip_x && x+i<screen.clip_x+screen.clip_w)
          ptr[(y+j)*(screen.scanline>>1)+x+i]=spritebg[16*j+i];
      }
    }
  }
}

int FB_get_color(int r, int g, int b) {
  return((((r>>11)&0x1f)<<11)|(((g>>10)&0x3f)<<5)|(((b>>11)&0x1f)));
}

unsigned char *fontlist816[1]={(unsigned char *)spat_a816};
unsigned char *fontlist57[1] ={(unsigned char *)asciiTable};

void Fb_BlitCharacter57(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character, int flags, int fontnr){
  unsigned char data0,data1,data2,data3,data4;

  if (x<0||y<0|| x>screen.width-CharWidth57 || y>screen.height-CharHeight57) return;
//  if ((character < Fontmin) || (character > Fontmax)) character = '.';

  const unsigned char *aptr = &((fontlist57[fontnr])[character*5]);
  data0 = *aptr++;
  data1 = *aptr++;
  data2 = *aptr++;
  data3 = *aptr++;
  data4 = *aptr;

  register unsigned short *ptr  = (unsigned short*)(screen.pixels+y*screen.scanline);
  ptr+=x;
  register unsigned short *endp  = ptr+CharHeight57*screen.width;

  if(flags&FL_REVERSE) { /* reverse */
    unsigned short t=aBackColor;
    aBackColor=aColor;
    aColor=t;
  }
#if 0
  /* Sieht nicht gut aus (unlesbar) deshalb ignorieren*/
  if(flags&FL_BOLD) {/* bold */
    data0|=data1;
    data1|=data2;
    data2|=data3;
    data3|=data4;
  }
#endif  
  if(flags&FL_UNDERLINE || flags&FL_DBLUNDERLINE) {/* underline */
    data0|=0x80;
    data1|=0x80;
    data2|=0x80;
    data3|=0x80;
    data4|=0x80;
  }
  if(flags&FL_HIDDEN) {
    data0=data1=data2=data3=data4=0;
  }
  if(flags&FL_TRANSPARENT) {
    while (ptr<endp) {
      if (data0 & 1) *ptr= aColor;  ptr++; data0 >>= 1;    
      if (data1 & 1) *ptr= aColor;  ptr++; data1 >>= 1;
      if (data2 & 1) *ptr= aColor;  ptr++; data2 >>= 1;
      if (data3 & 1) *ptr= aColor;  ptr++; data3 >>= 1;
      if (data4 & 1) *ptr= aColor;  ptr++; data4 >>= 1;
      ptr+=screen.width-CharWidth57;
    }
  } else {
    while (ptr<endp) {
      if (data0 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data0 >>= 1;    
      if (data1 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data1 >>= 1;
      if (data2 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data2 >>= 1;
      if (data3 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data3 >>= 1;
      if (data4 & 1) *ptr++ = aColor; else *ptr++ = aBackColor; data4 >>= 1;
      ptr+=screen.width-CharWidth57;
    }
  }
}


void Fb_BlitCharacter816(int x, int y, unsigned short aColor, unsigned short aBackColor, unsigned char character, int flags,int fontnr){
  char charackter[CharHeight816];
  int i,d;
  if (x<0||y<0|| x>screen.width-CharWidth816 || y>screen.height-CharHeight816) return;
  const unsigned char *aptr = &((fontlist816[fontnr])[character*CharHeight816]);
  memcpy(charackter,aptr,CharHeight816);
  register unsigned short *ptr  = (unsigned short*)(screen.pixels+y*screen.scanline);

  if(flags&FL_REVERSE) { /* reverse */
    unsigned short t=aBackColor;
    aBackColor=aColor;
    aColor=t;
  }  
  if(flags&FL_FRAMED) {/* Framed */
    charackter[0]=0xff;
    for(i=1;i<16;i++) charackter[i]|=1;
  }
  if(flags&FL_UNDERLINE) {/* underline */
    charackter[15]=0xff;
    charackter[14]=0xff;
  } else if(flags&FL_DBLUNDERLINE) {/* double underline */
    charackter[15]=0xff;
    charackter[13]=0xff;
  }
  if(flags&FL_DIM) {
    for(i=0;i<16;i++) {
      charackter[i++]&=0xaa;
      charackter[i]  &=0x55;
    }
  } else if(flags&FL_CONCREAL) {
    for(i=0;i<16;i++) {
      charackter[i++]|=0xaa;
      charackter[i]  |=0x55;
    }
  }  
  ptr+=x;
  if(flags&FL_TRANSPARENT) {/* transparent */
    for(i=0;i<16;i++) {
      d=charackter[i];
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      if (d&0x80) *ptr= aColor; ptr++; d<<=1;
      ptr+=screen.width-CharWidth816;
    }
  } else {
    for(i=0;i<16;i++) {
      d=charackter[i];
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      if (d&0x80) *ptr++ = aColor; else *ptr++ = aBackColor; d<<=1;
      ptr+=screen.width-CharWidth816;
    }
  }
  if(flags&FL_BOLD) {/* bold */
    ptr  = (unsigned short*)(screen.pixels+y*screen.scanline);
    ptr+=x+1;
    for(i=0;i<16;i++) {
      d=charackter[i];
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      if(d&0x80) *ptr= aColor; ptr++; d<<=1;
      ptr+=screen.width-CharWidth816;
    }
  }
}

void Fb_BlitText57(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str) {
  while (*str) {
    Fb_BlitCharacter57(x, y, aColor, aBackColor, *str,0,0);
    x+=CharWidth57;
    str++;
  }
}
void Fb_BlitText816(int x, int y, unsigned short aColor, unsigned short aBackColor, char *str) {
  while (*str) {
    Fb_BlitCharacter816(x, y, aColor, aBackColor, *str,0,0);
    x+=CharWidth816;
    str++;
  }
}

extern int ltextpflg,chh;

void FB_DrawString(int x, int y, char *t,int len) {
  if(len>0) {
    char buf[len+1];
    FB_hide_mouse();
    memcpy(buf,t,len);
    buf[len]=0;
    if(ltextpflg==1 || chh==16)  Fb_BlitText816(x,y,screen.fcolor, screen.bcolor,buf);
    else Fb_BlitText57(x,y,screen.fcolor,screen.bcolor,buf);
    FB_show_mouse();
  }
}



void FB_get_geometry(int *x, int *y, int *w, int *h, int *b, int *d) {
  *b=*x=*y=0;
  *w=screen.width;
  *h=screen.height;
  *d=screen.bpp;
}
/* We need these because ARM has 32 Bit alignment (and the compiler has a bug)*/
void writeint(char *p,int n) {
  p[0]=n&0xff;
  p[1]=(n&0xff00)>>8;
  p[2]=(n&0xff0000)>>16;
  p[3]=(n&0xff000000)>>24;
}
void writeshort(char *p,short n) {
  p[0]=n&0xff;
  p[1]=(n&0xff00)>>8;
}



/* This produces data which conforms to a WINDOWS .bmp file */

char *FB_get_image(int x, int y, int w,int h, int *len) {
  if(x<0||y<0||w<=0||h<=0|| x+w>screen.width|| y+h>screen.height) return(NULL);
  register unsigned short *ptr1  = (unsigned short*)(screen.pixels+y*screen.scanline);
  int size=h*w*4;
  int i,j,r,g,b,l;
  l=size+BITMAPFILEHEADERLEN+BITMAPINFOHEADERLEN;
  char *buf=malloc(l);
  char *buf3;
  char *buf2=buf+BITMAPFILEHEADERLEN+BITMAPINFOHEADERLEN;
  BITMAPFILEHEADER *header=(BITMAPFILEHEADER *)buf;
  BITMAPINFOHEADER *iheader=(BITMAPINFOHEADER *)(buf+BITMAPFILEHEADERLEN);
  header->bfType=BF_TYPE;
  writeint((char *)&buf[10],BITMAPFILEHEADERLEN+BITMAPINFOHEADERLEN);
  writeint((char *)&(iheader->biSize),BITMAPINFOHEADERLEN);
  writeint((char *)&(iheader->biWidth),w);
  writeint((char *)&(iheader->biHeight),h);
  writeshort((char *)&(iheader->biPlanes),1);
  writeshort((char *)&(iheader->biBitCount),24);
  writeint((char *)&(iheader->biCompression),BI_RGB);
  writeint((char *)&(iheader->biSizeImage),0);
  writeint((char *)&(iheader->biXPelsPerMeter),0);
  writeint((char *)&(iheader->biYPelsPerMeter),0);
  writeint((char *)&(iheader->biClrUsed),0);
  writeint((char *)&(iheader->biClrImportant),0);
  ptr1+=x;
  buf3=buf2;
  for(i=h-1;i>=0;i--) {
    for(j=0;j<w;j++) {
      r=((ptr1[j+i*screen.width]>>11)& 0x1f)<<3;
      g=((ptr1[j+i*screen.width]>>5)& 0x3f)<<2;
      b=(ptr1[j+i*screen.width] & 0x1f)<<3;
      *buf2++=b;
      *buf2++=g;
      *buf2++=r;
    }    
    buf2=(char *)(((((int)buf2-(int)buf3)+3)&0xfffffffc)+(int)buf3); /* align to 4 */
  }
  size=buf2-buf3;
  l=size+BITMAPFILEHEADERLEN+BITMAPINFOHEADERLEN;
  writeint(&buf[2],l);
  if(len) *len=l;
  return(buf);
}
void FB_put_image(char *data,int x, int y) {
  if(x<screen.clip_x||y<screen.clip_y||x>screen.width||y>screen.height) return;
  bmp2bitmap(data,screen.pixels+y*screen.scanline,x,screen.width,screen.height-y,16);
}



void FB_bmp2pixel(char *s,unsigned short *d,int w, int h, unsigned short color) {
  int i,j;
  unsigned char a;
  for(j=0;j<h;j++) {
  a=*s++;
  for(i=0;i<8;i++) {
    if((a>>i)&1) *d=color;
    else *d=WHITE;
    d++;
    if((a>>i)&1) printf("##");
    else printf("..");
  }
  a=*s++;
  for(i=0;i<8;i++) {
     if((a>>i)&1) *d=color;
    else *d=WHITE;
    d++;
    if((a>>i)&1) printf("##");
    else printf("..");
  }
  printf("\n");
  }
}
void FB_bmp2mask(char *s,unsigned char *d,int w, int h) {
  int i,j;
  unsigned char a;
  for(j=0;j<h;j++) {
  a=*s++;
  for(i=0;i<8;i++) {
    if((a>>i)&1) *d=255;
    else *d=0;
    d++;
    if((a>>i)&1) printf("##");
    else printf("..");
  }
  a=*s++;
  for(i=0;i<8;i++) {
     if((a>>i)&1) *d=255;
    else *d=0;
    d++;
    if((a>>i)&1) printf("##");
    else printf("..");
  }
  printf("\n");
  }

}

void DrawCircle(int x, int y, int r, unsigned short color) {
  int i, row, col, px, py;
  long int sum;
  if(r==0) {
    FB_PutPixel(x,y,color);
    return;
  }

  x+=r;
  y+=r;

  py = r<<1;
  px = 0;
  sum = 0;
  while (px <= py) {
    if (!(px & 1)) {
      col = x + (px>>1);
      row = y + (py>>1);FB_PutPixel(col,row,color);
      row = y - (py>>1);FB_PutPixel(col,row,color);
      col = x - (px>>1);FB_PutPixel(col,row,color);
      row = y + (py>>1);FB_PutPixel(col,row,color);
      col = x + (py>>1);
      row = y + (px>>1);FB_PutPixel(col,row,color);
      row = y - (px>>1);FB_PutPixel(col,row,color);
      col = x - (py>>1);FB_PutPixel(col,row,color);
      row = y + (px>>1);FB_PutPixel(col,row,color);
    }
    sum+=px++;
    sum += px;
    if (sum >= 0) {
	sum-=py--;
	sum-=py;
    }
  }
}

#include <math.h>

void FB_Arc(int x, int y, int r1, int r2, int a1, int a2) {
/* muss man mit polygonen machen.... */
  if(r1==0 && r2==0) {
    FB_PutPixel(x,y,screen.fcolor);
    return;
  }
  if(a1+64*360==a2) {
    if(r1==r2) DrawCircle(x,y,r1/2,screen.fcolor);
    else ; /*ellipse*/
    return;
  } else {
    int i;
    int ox=x;
    int oy=y;
    int dx,dy;
//    printf("Draw-Arc: a1=%d,a2=%d\n",a1,a2);
    dx=(double)r1*cos((double)a1/180*3.14159/64);
    dy=(double)r2*sin((double)a1/180*3.14159/64);
    FB_line(x,y,x+dx,y+dy);
    for(i=a1/64+1;i<=a2/64;i++) {
      ox=dx;oy=dy;
      dx=(double)r1*cos((double)i/180*3.14159);
      dy=(double)r2*sin((double)i/180*3.14159);
      FB_line(x+ox,y+oy,x+dx,y+dy);
    }
    FB_line(x,y,x+dx,y+dy);
  }
}

void FillCircle(int x, int y, int r, unsigned short color) {
  int i, row, col_start, col_end, t_row, t_col, px, py;
  long int sum;
  x+=r;
  y+=r;
  
  py = r<<1;
  px = 0;
  sum = 0;
  while (px <= py) {
    col_start=x-(px>>1);
    col_end=x+(px>>1);
    row =y+(py>>1);
    fillLine(col_start, col_end, row, color);
    col_start =x - (py>>1);
    col_end =x + (py>>1);
    row =y+(px>>1);
    fillLine(col_start, col_end, row, color);

    col_start = x - (px>>1);
    col_end = x + (px>>1);
    row =y - (py>>1);
    fillLine(col_start, col_end, row, color);

    col_start = x - (py>>1);
    col_end = x + (py>>1);
    row =y - (px>>1);
    fillLine(col_start, col_end, row, color);

    sum +=px++;
    sum += px;
    if (sum >= 0) {
       sum-=py--;
       sum -=py;
    }
  }
}
void FB_pArc(int x, int y, int r1, int r2, int a1, int a2) {
/* muss man mit polygonen machen.... */
  if(r1==0 && r2==0) {
    FB_PutPixel(x,y,screen.fcolor);
    return;
  }
  if(a1+360*64==a2) {
    if(r1==r2) FillCircle(x,y,r1/2,screen.fcolor);
    else ; /*ellipse*/
    return;
  } else {
    int point[((a2-a1)/64+2)*2];
    int i;
    int count=0;
    int ox=x;
    int oy=y;
    int dx,dy;
    printf("Draw-Arc: a1=%d,a2=%d\n",a1,a2);
    point[count++]=x;
    point[count++]=y;
    dx=(double)r1*cos((double)a1/180*3.14159/64);
    dy=(double)r2*sin((double)a1/180*3.14159/64);
    point[count++]=x+dx;
    point[count++]=y+dy;
    for(i=a1/64+1;i<=a2/64;i++) {
      ox=dx;oy=dy;
      dx=(double)r1*cos((double)i/180*3.14159);
      dy=(double)r2*sin((double)i/180*3.14159);
      point[count++]=x+dx;
      point[count++]=y+dy;
    }
    fill2Poly(screen.fcolor,point,count/2);
  }
}

/*
  ���������������������������������������������������������ͻ
  �                                                         �
  �                                                         �
  � fil2Poly() = fills a polygon in specified color by      �
  �              filling in boundaries resulting from       �
  �              connecting specified points in the         �
  �              order given and then connecting last       �
  �              point to first.  Uses an array to          �
  �              store coordinates.                         �
  �                                                         �
  ���������������������������������������������������������ͼ
*/
int sort_function (const long int *a, const long int *b) {
	if (*a < *b)  return(-1);
	if (*a == *b) return(0);
	if (*a > *b)  return(1);
}


void fill2Poly(unsigned short color,int *point, int num) {
  #define sign(x) ((x) > 0 ? 1:  ((x) == 0 ? 0:  (-1)))

  int dx, dy, dxabs, dyabs, i=0, index=0, j, k, px, py, sdx, sdy, x, y,
      xs,xe,ys,ye,toggle=0, old_sdy, sy0;
  long int coord[4000];
  if (num<3) return;
  i=2*num+1;
  px = point[0];
  py = point[1];
  if (point[1] == point[3]) {
    coord[index++] = px | (long)py << 16;
  }
  for (j=0; j<=i-3; j+=2) {
		xs = point[j];
		ys = point[j+1];
		if ((j == (i-3)) || (j == (i-4)))
		{
			xe = point[0];
			ye = point[1];
		}
		else
		{
			xe = point[j+2];
			ye = point[j+3];
		}
    dx = xe - xs;
    dy = ye - ys;
    sdx = sign(dx);
    sdy = sign(dy);
    if (j==0) {
      old_sdy = sdy;
      sy0 = sdy;
    }
    dxabs = abs(dx);
    dyabs = abs(dy);
    x = 0;
    y = 0;
    if (dxabs >= dyabs) {
      for (k=0; k<dxabs; k++) {
	y += dyabs;
	if (y>=dxabs) {
	  y -= dxabs;
	  py += sdy;
	  if (old_sdy != sdy) {
	    old_sdy = sdy;
	    index--;
	  }
	  coord[index++] = (px + sdx) | (long)py << 16;
	}
	px += sdx;
	FB_PutPixel(px,py,color);
      }
    } else {
      for (k=0; k<dyabs; k++) {
	x += dxabs;
	if (x>=dyabs) {
	  x -= dyabs;
	  px += sdx;
	}
	py += sdy;
	if (old_sdy != sdy) {
	  old_sdy = sdy;
	  if (sdy != 0) index--;
        }
	FB_PutPixel(px,py,color);
	coord[index] = px | (long)py << 16;
	index++;
      }
    }
  }
  if(sy0 + sdy == 0) index--;
  qsort(coord,index,sizeof(coord[0]),(int(*)
		(const void *, const void *))sort_function);
  for (i=0; i<index; i++) {
    xs = min(screen.width-1,(max(0,(int)((signed short)(coord[i])))));
    xe = min(screen.width-1,(max(0,(int)((signed short)(coord[i + 1])))));
    ys = min(screen.height-1,(max(0,(int)((signed short)(coord[i] >> 16)))));
    ye = min(screen.height-1,(max(0,(int)((signed short)(coord[i + 1] >> 16)))));
    if ((ys == ye) && (toggle == 0)) {
      fillLine(xs, xe, ys, color);
      toggle = 1;
    } else toggle = 0;
  }
}


void FB_pPolygon(XPoint *points, int n,int shape, int mode) {
  fill2Poly(screen.fcolor,(int *)points,n);
}


#define MAXQUEUELEN 1024

XEvent eque[MAXQUEUELEN];

int queueptr=0;
int escflag=0;
int numbers[16];
int anznumbers;
int number=0;

void process_char(int a) {
  a&=0xff;
  
  if(escflag>=2) {
    if(a==';') {
      if(anznumbers<15) numbers[anznumbers++]=number;
      number=0;
    } else if(a=='?') {
      escflag++;
    } else if(a>='0' && a<='9') {
      number=number*10+(a-'0');
    } else {      
      escflag=0;
      if(anznumbers<15)  numbers[anznumbers++]=number;
      if(a=='o') {
        eque[queueptr].type=MotionNotify;
        eque[queueptr].xmotion.x=numbers[0];
        eque[queueptr].xmotion.y=numbers[1];
        eque[queueptr].xmotion.x_root=numbers[0];
        eque[queueptr].xmotion.y_root=numbers[1];
        eque[queueptr].xmotion.state=numbers[2];
#if DEBUG
	printf("Mausmotion: %d %d %d \n",numbers[0],numbers[1],numbers[2]);
#endif
        queueptr++;
      } else if(a=='M') {
        if(anznumbers>=3 ||numbers[2]>0) eque[queueptr].type=ButtonPress;
        else eque[queueptr].type=ButtonRelease;
        eque[queueptr].xbutton.x=numbers[0];
        eque[queueptr].xbutton.y=numbers[1];
        eque[queueptr].xbutton.button=1;
        eque[queueptr].xbutton.state=numbers[2];
        eque[queueptr].xbutton.x_root=numbers[0];
        eque[queueptr].xbutton.y_root=numbers[1];
#if DEBUG
	printf("Mausklick: %d %d %d \n",numbers[0],numbers[1],numbers[2]);
#endif
	queueptr++;
	if(eque[queueptr-1].type==ButtonPress) {
	eque[queueptr].type=ButtonRelease;
        eque[queueptr].xbutton.x=numbers[0];
        eque[queueptr].xbutton.y=numbers[1];
        eque[queueptr].xbutton.button=1;
        eque[queueptr].xbutton.state=numbers[2];
        eque[queueptr].xbutton.x_root=numbers[0];
        eque[queueptr].xbutton.y_root=numbers[1];
#if DEBUG
	printf("Mausrelease: %d %d %d \n",numbers[0],numbers[1],numbers[2]);
#endif
	queueptr++;
	}
      } else if(a>='A' && a<='D') {
        if(a=='A') a=0x52;
	else if(a=='B') a=0x54;
	else if(a=='C') a=0x53;
	else if(a=='D') a=0x51;
	
        eque[queueptr].type=KeyPress;
        eque[queueptr].xkey.keycode=(char)a;
        eque[queueptr].xkey.ks=a&255|0xff00;
        eque[queueptr].xkey.buf[0]=(char)a;
        eque[queueptr].xkey.buf[1]=0;
        queueptr++;
        eque[queueptr].xkey.keycode=(char)a;
        eque[queueptr].xkey.ks=a&255|0xff00;
        eque[queueptr].type=KeyRelease;
        eque[queueptr].xkey.buf[0]=(char)a;
        eque[queueptr].xkey.buf[1]=0;
        queueptr++;
      } else printf("Unknown ESC-Code: %d\n",a);
    }    
  } else if(escflag==1) {
    if(a=='[') {escflag=2;number=anznumbers=0;}
    else escflag=0;
  } else {
    if(a==27) escflag=1;
    else if(a==127||a==10||a==8||a==9||a==27) {
      if(a==10) a=13;
      else if(a==127) a=8;
      eque[queueptr].type=KeyPress;
      eque[queueptr].xkey.keycode=(char)a;
      eque[queueptr].xkey.ks=a&255|0xff00;
      eque[queueptr].xkey.buf[0]=(char)a;
      eque[queueptr].xkey.buf[1]=0;
      queueptr++;
      eque[queueptr].xkey.keycode=(char)a;
      eque[queueptr].xkey.ks=a&255|0xff00;
      eque[queueptr].type=KeyRelease;
      eque[queueptr].xkey.buf[0]=(char)a;
      eque[queueptr].xkey.buf[1]=0;
      queueptr++;
    } else {
      eque[queueptr].xkey.keycode=(char)a;
      eque[queueptr].xkey.ks=a&255;
      eque[queueptr].type=KeyPress;
      eque[queueptr].xkey.buf[0]=(char)a;
      eque[queueptr].xkey.buf[1]=0;
      queueptr++;
      eque[queueptr].xkey.keycode=(char)a;
      eque[queueptr].xkey.ks=a&255;
      eque[queueptr].type=KeyRelease;
      eque[queueptr].xkey.buf[0]=(char)a;
      eque[queueptr].xkey.buf[1]=0;
      queueptr++;
    }
  }
  if(queueptr>=MAXQUEUELEN-5) {
    queueptr=0;
    printf("error: Eventqueue is full!\n");
  }
}


void collect_events(){
  fd_set set;
  int a,rc;
#ifdef TIMEVAL_WORKAROUND
  struct { int  tv_sec; 
           int  tv_usec; } tv;
#else 
    struct timeval tv;
#endif
   /* memset(&tv, 0, sizeof(tv));  */   
    FD_ZERO(&set);
    FD_SET(0, &set);

  while(1) {
    tv.tv_sec=0; tv.tv_usec=0;
    FD_ZERO(&set);
    FD_SET(0, &set);
    rc=select(1, &set, 0, 0, &tv);
    if(rc==0) return;
    else if (rc<0) printf("select failed: errno=%d\n",errno);
    else {
      a=getc(stdin);
      process_char(a);
#if DEBUG
      printf("PC: %d, %d Events pending.\n",a,queueptr);
#endif
    }
  }
}
void wait_events(){
  fd_set set;
  int rc=0;
#ifdef TIMEVAL_WORKAROUND
  struct { int  tv_sec; 
           int  tv_usec; } tv;
#else 
    struct timeval tv;
#endif
   /* memset(&tv, 0, sizeof(tv));  */   
  while(rc==0) {
    tv.tv_sec=1; tv.tv_usec=0;
    FD_ZERO(&set);
    FD_SET(0, &set);
    rc=select(1, &set, 0, 0, &tv);
    if (rc<0) printf("select failed: errno=%d\n",errno);
  }

}

int FB_check_event(int mask, XEvent *event) {
  int i,r=-1;
  collect_events();
  if(queueptr) {
    for(i=0;i<queueptr;i++) {
      if(eque[i].type&mask) {
         r=i;
      }
    }
  }
  if(r<0) return(0);
  *event=eque[r]; 
   if(r<=queueptr-1) {
    for(i=r;i<queueptr-1;i++) {
      eque[i]=eque[i+1];
    }
  }
  if(queueptr>0) queueptr--;
  return(1);
}


void FB_event(int mask, XEvent *event) {
  int i,r=-1;
  while(r==-1) {
    collect_events();
    
    if(queueptr) {
      for(i=0;i<queueptr;i++) {
        if(eque[i].type&mask) {
           r=i;
	   break;
        }
      }
    }
    if(r<0) wait_events();
  }
  
  *event=eque[r]; 
 /* now remove the event from the list */
  if(r<=queueptr-1) {
    for(i=r;i<queueptr-1;i++) {
      eque[i]=eque[i+1];
    }
  }
  if(queueptr>0) queueptr--;
}
void FB_next_event(XEvent *event) {
  FB_event(0xffffffff, event);
}


void FB_Query_pointer(int *rx,int *ry,int *x,int *y,int *k) {
  XEvent ev;
  if(FB_check_event(ButtonPressMask|PointerMotionMask|ButtonReleaseMask, &ev)) {
    FB_hide_mouse();
    if(ev.type==ButtonPress||ev.type==ButtonRelease) {
      screen.mouse_x=ev.xbutton.x;
      screen.mouse_y=ev.xbutton.y;
      screen.mouse_k=ev.xbutton.button;
      screen.mouse_s=ev.xbutton.state;      
    } else if(ev.type==MotionNotify) {
      screen.mouse_x=ev.xmotion.x;
      screen.mouse_y=ev.xmotion.y;
      screen.mouse_s=ev.xmotion.state;
    }
    FB_show_mouse();
  }
  *rx=*x=screen.mouse_x;
  *ry=*y=screen.mouse_y;
  *k=screen.mouse_k;
}


void FB_mouse(int onoff){
  static int mousecount=0;
  if(onoff) {
    printf("\033[?10h");
    mousecount++;
  } else {
    mousecount--;
    if(mousecount==0) printf("\033[?10l");
  }  
  fflush(stdout);
}