
/* graphics.c      graphics-Routinen (c) Markus Hoffmann    */


/* This file is part of X11BASIC, the basic interpreter for Unix/X
 * ======================================================================
 * X11BASIC is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "graphics.h"

#include "window.h"


#ifdef USE_SDL
unsigned int fcolor=0xffffffff,bcolor;
SDL_Surface *display[MAXWINDOWS];
#endif

#ifdef USE_X11
char *display_name = NULL;  /* NULL: Nimm Argument aus setenv DISPLAY */

#endif

int marker_typ=0,marker_size=1;
int boundary=-1;
double ltextwinkel=0,ltextxfaktor=0.3,ltextyfaktor=0.5;
int ltextpflg=0;


#ifdef FRAMEBUFFER
#include "bitmaps/bombe.bmp"
#include "bitmaps/bombe_mask.bmp"
#endif

#ifdef USE_X11
#include "bitmaps/bombe_gross.bmp"
#endif


/* Line-Funktion (fuer ltext) */
void line(int x1,int y1,int x2,int y2) {
#ifdef USE_X11
  XDrawLine(display[usewindow],pix[usewindow],gc[usewindow],x1,y1,x2,y2);
#elif defined FRAMEBUFFER
  FB_line(x1,y1,x2,y2);
#elif defined USE_SDL
  lineColor(display[usewindow],x1,y1,x2,y2,fcolor);
#else
  DrawLine(x1,y1,x2,y2);
#endif
}
void box(int x1,int y1,int x2, int y2) {
  DrawRectangle(min(x1,x2),min(y1,y2),abs(x2-x1),abs(y2-y1)); 
}
void pbox(int x1,int y1,int x2, int y2) {
  FillRectangle(min(x1,x2),min(y1,y2),abs(x2-x1)+1,abs(y2-y1)+1); 
}
int get_point(int x, int y) {
    int d,r;
#ifdef WINDOWS_NATIVE
#endif
#ifdef USE_SDL
r=sdl_getpixel(display[usewindow],x,y);
#endif
#ifdef FRAMEBUFFER
  r=FB_point(x,y);
#endif
#ifdef USE_X11
    XImage *Image;
    graphics();
    d=XDefaultDepthOfScreen(XDefaultScreenOfDisplay(display[usewindow]));
    Image=XGetImage(display[usewindow],win[usewindow],
                x, y, 1, 1, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
    r=XGetPixel(Image, 0, 0);
    XDestroyImage(Image);
#endif
    return(r);
}

int get_fcolor() {
#ifdef WINDOWS_NATIVE
  return(global_color);
#endif
#ifdef FRAMEBUFFER
  return(screen.fcolor);
#endif
#ifdef USE_X11
  XGCValues gc_val;  
  XGetGCValues(display[usewindow], gc[usewindow],  GCForeground, &gc_val);
  return(gc_val.foreground);
#endif
#ifdef USE_SDL
  return(fcolor);
#endif
}
int get_bcolor() {
#ifdef FRAMEBUFFER
  return(screen.bcolor);
#endif
#ifdef USE_X11
  XGCValues gc_val;  
  XGetGCValues(display[usewindow], gc[usewindow],  GCBackground, &gc_val);
  return(gc_val.background);
#endif
#ifdef USE_SDL
  return(bcolor);
#endif
}
void set_graphmode(int n) { 
/*            n=1 copy src
              n=2 src xor dest
              n=3 invert (not dest)
              n=4 src and dest
              n=5 not (src xor dest)
              n<0 uebergibt -n an X-Server
 */  

#ifdef USE_X11
  XGCValues gc_val;  
  switch (n) {
    case 1:gc_val.function=GXcopy;       break;
    case 2:gc_val.function=GXxor;        break;
    case 3:gc_val.function=GXinvert;     break;
    case 4:gc_val.function=GXand;        break;
    case 5:gc_val.function=GXequiv;      break;
    case 6:gc_val.function=GXandInverted;break;
    case 7:gc_val.function=GXor;         break;
    case 8:gc_val.function=GXorInverted; break;
    default:
    if(n>=0) gc_val.function=n;
    else gc_val.function=-n;
    break;
  } 
  XChangeGC(display[usewindow], gc[usewindow],  GCFunction, &gc_val);
#endif
#ifdef FRAMEBUFFER
  switch (n) {
    case 1:FB_set_alpha(255);break;
    case 2:break;
    case 3:break;
    case 4:FB_set_alpha(127);break;
    case 5:break;
    default:
    if(n>=0) FB_set_alpha(n);
    else FB_set_alpha(-n);
    break;
  } 
#endif
}
void set_font(char *name) {
#ifdef USE_X11
   XGCValues gc_val;  
   XFontStruct *fs;
   fs=XLoadQueryFont(display[usewindow], name);
   if(fs!=NULL)  {
     gc_val.font=fs->fid;
     XChangeGC(display[usewindow], gc[usewindow],  GCFont, &gc_val);
   }
#endif
}

int get_graphmode() {
#ifdef WINDOWS_NATIVE
  return(0);
#endif
#ifdef USE_X11
  XGCValues gc_val;  
  XGetGCValues(display[usewindow], gc[usewindow],  GCFunction, &gc_val);
  return(gc_val.function);
#endif
#ifdef FRAMEBUFFER
  if(screen.alpha==255) return(0);
  if(screen.alpha==127) return(4);
  return(screen.alpha);
#endif
}
void set_fill(int c) {
#ifdef USE_X11
#include "bitmaps/fill.xbm"
  static Pixmap fill_pattern;
  static fill_alloc=0;
    if(fill_alloc) XFreePixmap(display[usewindow],fill_pattern);
    fill_pattern = XCreateBitmapFromData(display[usewindow],win[usewindow],
                fill_bits+c*fill_width*fill_width/8,fill_width,fill_width);
           /*XSetFillStyle(display[usewindow], gc[usewindow], FillStippled); */
    fill_alloc=1;   
    XSetStipple(display[usewindow], gc[usewindow],fill_pattern);
#endif
}
int mousex() {
#ifdef WINDOWS_NATIVE
  return(global_mousex);
#endif
  int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
#ifdef USE_X11
  Window root_return,child_return;
#endif
#if defined USE_X11 || defined USE_SDL
  graphics(); 
  XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
  return(win_x_return);
#endif
}
int mousey() {
#ifdef WINDOWS_NATIVE
  return(global_mousey);
#endif
    int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
#ifdef USE_X11
  Window root_return,child_return;
#endif
#if defined USE_X11 || defined USE_SDL
  graphics();
   
  XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
  return(win_y_return);
#endif
}
int mousek() {
#ifdef WINDOWS_NATIVE
  return(global_mousek);
#endif
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
#ifdef USE_X11
   Window root_return,child_return;
#endif
#if defined USE_X11 || defined USE_SDL
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(mask_return>>8);
#endif
}
int mouses() {
#ifdef WINDOWS_NATIVE
  return(global_mouses);
#endif
  int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
#ifdef USE_X11
   Window root_return,child_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(mask_return & 255);
#endif
#ifdef USE_SDL
return(0);
#endif
}


/* Alloziert einen Farbeintrag in der Palette.
   Rueckgabewert ist die Nummer des Farbeintrags.
   Ist kein neuer Eintrag mehr frei, so wird die Nummer des Eintrags
   zurueckgeliefert, der der spezifizierten Farbe am naechsten kommt.
   Diese Routine kann also kein XAllocColor failed mehr produzieren.

   (c) markus hoffmann  1998                                   */


unsigned int get_color(int r, int g, int b) {
#ifdef WINDOWS_NATIVE
  return(RGB(r>>8,g>>8,b>>8));
#endif
#ifdef FRAMEBUFFER
  return(FB_get_color(r,g,b));
#endif
#ifdef USE_SDL
  return((unsigned int)SDL_MapRGB(display[usewindow]->format, r>>8, g>>8, b>>8)<<8 |0xff);
#else
  Colormap map;
  XColor pixcolor;

  map =XDefaultColormapOfScreen ( XDefaultScreenOfDisplay ( display[usewindow] ) );

  pixcolor.red=r;
  pixcolor.green=g;
  pixcolor.blue=b;
  if(my_XAllocColor(display[usewindow], map, &pixcolor)==0)
    printf("could not switch to color.\n");

  return(pixcolor.pixel);
#endif
}



#ifdef USE_X11
Status my_XAllocColor(Display *display,Colormap map,XColor *pixcolor) {
  Status rval;
  if((rval=XAllocColor(display, map, pixcolor))==0) {
    int i,r,g,b;
    unsigned long e=0xfffffff,d;
    XColor savecolor;
    XColor ppixcolor[256];
	for(i=0;i<256;i++) ppixcolor[i].pixel=i;
	XQueryColors(display, map, ppixcolor,256);
	
    r=pixcolor->red;
    g=pixcolor->green;
    b=pixcolor->blue;

    savecolor=*pixcolor;
	
   for(i=0;i<256;i++) {
     d=(ppixcolor[i].red-r)/16*(ppixcolor[i].red-r)/16+
       (ppixcolor[i].green-g)/16*(ppixcolor[i].green-g)/16+
       (ppixcolor[i].blue-b)/16*(ppixcolor[i].blue-b)/16;
     if(d<e) {
       e=d;
       savecolor=ppixcolor[i];
     }
   }
   /*
   printf("Gewuenscht: r= %x, g=%x, b=%x.\n",r,g,b);
   printf("Farbe wurde abgewandelt: %d, Close %d.\n",savecolor.pixel, e);
   if(e==0) printf("** Volltreffer: recycled \n");
   printf("Neu: r= %x, g=%x, b=%x.\n",savecolor.red,savecolor.green,savecolor.blue);
   */
   rval=XAllocColor(display, map, &savecolor);
   pixcolor->pixel=savecolor.pixel;
  }
  return(rval);

}
#endif

/**** Flood Fill *********************************************************/

#define QUEUESIZE   256		/* The size of Queue (400) */

#define FF_UP	    1
#define FF_DN	   -1

#define SCAN_UNTIL  0
#define SCAN_WHILE  1
#define FF_FILLED   (!0)

typedef struct {
	int	x;
	int	y;
	int	w;
	int	h;
} ARECT;

extern ARECT sbox;

struct	tagParams	{
		int    xl;     /* leftmost pixel in run */
		int    xr;     /* rightmost pixel in run */
		int y;	    /* y-coordinate of run */
		unsigned char f; /* TRUE if run is filled (blocked) */
};

struct tagQUEUE	{
	struct tagQUEUE	*pQ;     /* pointer to opposite queue */
	int 	    d;	    /* direction (UP or DN) */
	int 	    n;	    /* number of unfilled runs in queue */
	int 	    h;	    /* index of head of queue */
	int 	    t;	    /* index of tail of queue */
	struct tagParams *run;
};

typedef	struct tagQUEUE	FF_QUEUE;

int 	ff_scan_left(int, int, int, int );
int 	ff_scan_right(int,int, int, int );
void    ff_add_queue(FF_QUEUE *,int,int, int, int );
int	ff_next_branch(int,int,int,int,int);
int	ff_in_queue(FF_QUEUE *,int,int,int);


void  ffill(int x0,int y0,int fill_color, int border_color) {
  int y=y0,x,xl=x0,xr=x0,xln,xrn;
  int qp,bcd;
  int scan_type;
  struct tagParams ff_buf1[QUEUESIZE];
  struct tagParams ff_buf2[QUEUESIZE];
  FF_QUEUE Qup,Qdn,*Q;

   /* do nothing if the seed pixel is a border pixel */
  if(border_color==-1)  {
    border_color=get_point(x0,y0);
    scan_type=SCAN_WHILE;
  } else  scan_type=SCAN_UNTIL;
  if(scan_type==SCAN_UNTIL) {
    if(get_point(x0,y0)==border_color) return;
  } else {
    if(get_point(x0,y0)==fill_color) return;
  }
  Qup.run = ff_buf1;
  Qdn.run = ff_buf2;
  Qup.pQ = &Qdn;		/* pointer to opposite queue */
  Qup.d = FF_UP; 		/* direction for queue */
  Qup.h = -1;
  Qup.t = 0;
  Qup.n = 0;

  Qdn.pQ = &Qup;
  Qdn.d = FF_DN;
  Qdn.h = -1;
  Qdn.t = 0;
  Qdn.n = 0;
  /* put the seed run in the up queue */
  Q = &Qup;
  ff_add_queue(Q,ff_scan_left(x0,y0,border_color,scan_type),
		ff_scan_right(x0,y0,border_color,scan_type),y0,!FF_FILLED);

  for(;;) {
    if(Q->n==0) {
      Q=Q->pQ;
      if(Q->n==0) break;
    }
    qp = Q->h;
    while(qp>=Q->t) {
      if(!Q->run[qp].f) {
	y=Q->run[qp].y;
	xl=Q->run[qp].xl;
	xr=Q->run[qp].xr;
	line(xl,y,xr+1,y);      /* fill the run */
	activate();
	Q->run[qp].f=FF_FILLED;
	Q->n--;
	break;
      } else qp-- ;
    }
/*    printf("Queue(%d) [%d,%d]:  n=%d\n",Q->d,Q->h,Q->t,Q->n);*/
    if(Q->d==FF_UP) {
      while((Q->h>qp)&&(Q->run[Q->h].y<(y-1))) Q->h--;
    } else {
      while((Q->h>qp)&&(Q->run[Q->h].y>(y+1))) Q->h--;
    }
    xln=ff_next_branch(xl,xr,y+Q->d,border_color,scan_type);
    while(xln>=0) {
      x=(xln>xl)?xln:xl;
      xrn=ff_scan_right(x,y+Q->d,border_color,scan_type);
      if(!ff_in_queue(Q,xln,xrn,y+Q->d)) ff_add_queue(Q,xln,xrn,y+Q->d,!FF_FILLED);
      if(xrn>(xr-2)) break;
      else {
        x=xrn+2;
	xln=ff_next_branch(x,xr,y+Q->d,border_color,scan_type);
      }
    }
    bcd=0;
    xln=ff_next_branch(xl,xr,y-Q->d,border_color,scan_type);
    while(xln>=0) {
      x=(xln>xl)?xln:xl;
      xrn=ff_scan_right(x,y-Q->d,border_color,scan_type);
      if(!ff_in_queue(Q,xln,xrn,y-Q->d)) {
        ff_add_queue( Q->pQ, xln, xrn, y-Q->d,!FF_FILLED);
        bcd=1;
      }
      if(xrn>(xr-2)) break;
      else {
        x=xrn+2;
	xln=ff_next_branch(x,xr,y-Q->d,border_color,scan_type);
      }
    }
    if(bcd) {
      Q=Q->pQ;
      ff_add_queue(Q,xl,xr,y,FF_FILLED);
    }
  }
}
int ff_scan_left(int xl,int y, int ucPixel, int f ) {
/*  printf("Scan-left: x=%d,y=%d  ",xl,y);*/
  if(f==SCAN_UNTIL)  {
    if(get_point(xl,y)==ucPixel) return -1;
    do {
      if(--xl<sbox.x) break;
    } while(get_point(xl,y)!=ucPixel);
  }  else {
    if(get_point(xl,y)!=ucPixel) return -1;
    do {
      if(--xl<sbox.x) break;
    } while (get_point(xl,y)==ucPixel);
  }
  return ++xl;
}


int ff_scan_right(int xr,int y,int ucPixel,int f) {
/*  printf("Scan-right: x=%d,y=%d  ",xr,y); */
  if(f==SCAN_UNTIL) {
    if(get_point(xr,y)==ucPixel) return -1;
    do {
      if(++xr>=sbox.x+sbox.w) break;
    } while(get_point(xr,y)!=ucPixel);
  } else {
    if(get_point(xr,y)!=ucPixel) return -1;
    do  {
      if(++xr>=sbox.x+sbox.w) break;
    } while(get_point(xr,y)==ucPixel);
  }
  return --xr;
}

void  ff_add_queue(FF_QUEUE *Q,int xl,int xr,int y,int f) {
  int   qp,i;
/*    printf("Add-queue: x=%d..%d,y=%d\n",xl,xr,y); */
  if(Q->d==FF_UP) {
    for ( qp = Q->t; qp <= Q->h; qp++ )
      if ( Q->run[qp].y <= y ) break;
  } else {
    for ( qp = Q->t; qp <= Q->h; qp++ )
      if ( Q->run[qp].y >= y ) break;
  }
  if(qp<=Q->h) {
    Q->h++;
    for(i=Q->h;i>qp;--i) {
      Q->run[i].xl=Q->run[i-1].xl;
      Q->run[i].xr=Q->run[i-1].xr;
      Q->run[i].y=Q->run[i-1].y;
      Q->run[i].f=Q->run[i-1].f;
    }
  } else Q->h++;
  Q->run[qp].xl=xl;
  Q->run[qp].xr=xr;
  Q->run[qp].y=y;
  Q->run[qp].f=f;
  if(!f) Q->n++ ;
}

int ff_next_branch(int xl,int xr,int y,int border_color,int scan_type) {
  int xln;
/*  printf("Next Branch: x=[%d,%d] y=%d   %d %d\n",xl,xr,y,sbox.y,sbox.h);*/
  if((y<sbox.y)||(y>=sbox.y+sbox.h)) return(-1);
  xln=ff_scan_left(xl,y,border_color,scan_type);
  if(xln==-1) {
    xln=ff_scan_right(xl,y,border_color,(scan_type==SCAN_WHILE)?SCAN_UNTIL:SCAN_WHILE);
    if(xln<xr) xln++ ;
    else xln=-1;
  }
  return xln;
}
int ff_in_queue(FF_QUEUE *Q, int xl, int xr, int y) {
  int  qp;
  if(Q->d==FF_UP) {
    for(qp=Q->h;qp>=0;--qp) {
      if( Q->run[qp].y>y ) break;
      if((Q->run[qp].y==y) && (Q->run[qp].xl==xl) && (Q->run[qp].xr==xr))
	return(1);
    }
  } else {
    for(qp=Q->h;qp>=0;--qp) {
      if(Q->run[qp].y<y) break;
      if((Q->run[qp].y==y) && (Q->run[qp].xl==xl) && (Q->run[qp].xr==xr))
	return(1);
    }
  }
  return(0);
}

/**** End ******************* Flood Fill ********************************/

#ifdef USE_X11

/* Bereite die Pixmap mit dem Bomben-Icon und der Fensternummer
   vor. */

void fetch_icon_pixmap(int nummer) {
  GC gc;XGCValues gc_val;
  char t[10];
  sprintf(t,"%2d",nummer);
  icon_pixmap[nummer]=XCreateBitmapFromData(display[nummer],win[nummer],
    bombe_gross_bits,bombe_gross_width,bombe_gross_height);
  gc = XCreateGC(display[nummer], icon_pixmap[nummer], 0, &gc_val);
  XSetForeground(display[nummer], gc, 0);
  XDrawString(display[nummer],icon_pixmap[nummer],gc,9,24,t,strlen(t));
  XFreeGC(display[nummer],gc);
}

#endif