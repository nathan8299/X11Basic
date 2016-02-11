
/* gkommandos.c Grafik-Befehle  (c) Markus Hoffmann     */


/* This file is part of X11BASIC, the basic interpreter for Unix/X
 * ============================================================
 * X11BASIC is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

/* Fehler beim Mouseevent beseitigt 2.6.1999   M. Hoffmann   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "globals.h"
#include "protos.h"
#include "gkommandos.h"
#include "window.h"

/* Line-Funktion fuer ltext */

void line(int x1,int y1,int x2, int y2) {
   XDrawLine(display[usewindow],pix[usewindow],gc[usewindow],x1,y1,x2,y2); 
}
void box(int x1,int y1,int x2, int y2) {
  int x,y,w,h;
  x=min(x1,x2);
  y=min(y1,y2);
  w=abs(x2-x1);
  h=abs(y2-y1);
  XDrawRectangle(display[usewindow],pix[usewindow],gc[usewindow],x,y,w,h); 
}
void pbox(int x1,int y1,int x2, int y2) {
  int x,y,w,h;
  x=min(x1,x2);
  y=min(y1,y2);
  w=abs(x2-x1);
  h=abs(y2-y1);
  XFillRectangle(display[usewindow],pix[usewindow],gc[usewindow],x,y,w,h); 
}



void c_rootwindow(char *n){
  usewindow=0; 
  graphics();
}
void c_norootwindow(char *n){
  usewindow=DEFAULTWINDOW; 
  graphics();
}
void c_usewindow(char *n){
  usewindow=max(0,min(MAXWINDOWS,(int)parser(n)));
  graphics();
}

void c_vsync(char *n) {
  activate();
}

void c_plot(PARAMETER *plist,int e) {
  if(e==2) {
    graphics();
    XDrawPoint(display[usewindow],pix[usewindow],gc[usewindow],
    plist[0].integer,plist[1].integer);
  }
}
int get_point(int x, int y) {
    int d,r;
    XImage *Image;
    graphics();
    d=XDefaultDepthOfScreen(XDefaultScreenOfDisplay(display[usewindow]));
    Image=XGetImage(display[usewindow],win[usewindow],
                x, y, 1, 1, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
    r=XGetPixel(Image, 0, 0);
    XDestroyImage(Image);
    return(r);
}

void c_savescreen(char *n) {
    int d,b,x,y,w,h,len,i;
    char *name=s_parser(n);
    char *data;
    XImage *Image;
    Window root;
    Colormap map;
    XWindowAttributes xwa;
    XColor ppixcolor[256];
    graphics();
    
      XGetGeometry(display[usewindow],
        RootWindow(display[usewindow],XDefaultScreen(display[usewindow]) ),
	&root,&x,&y,&w,&h,&b,&d); 
      XGetWindowAttributes(display[usewindow], root, &xwa);
    
    Image=XGetImage(display[usewindow],root,
                x, y, w, h, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
		
    if(d==8) {
      map =XDefaultColormapOfScreen ( XDefaultScreenOfDisplay(display[usewindow]));
      for(i=0;i<256;i++) ppixcolor[i].pixel=i;
      XQueryColors(display[usewindow], map, ppixcolor,256);
    }
    data=imagetoxwd(Image,xwa.visual,ppixcolor,&len);	
    bsave(name,data,len);
    XDestroyImage(Image);free(name);free(data);
}
void c_savewindow(char *n) {
    int d,b,x,y,w,h,len,i;
    char *name=s_parser(n);
    char *data;
    XImage *Image;
    Window root;
    Colormap map;
    XColor ppixcolor[256];
    XWindowAttributes xwa;
    
    graphics();
    
    XGetGeometry(display[usewindow], win[usewindow], 
	&root,&x,&y,&w,&h,&b,&d); 
    XGetWindowAttributes(display[usewindow], win[usewindow], &xwa);
    
    Image=XGetImage(display[usewindow],pix[usewindow],
                0, 0, w, h, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
    if(d==8) {
      map =XDefaultColormapOfScreen ( XDefaultScreenOfDisplay(display[usewindow]));
      for(i=0;i<256;i++) ppixcolor[i].pixel=i;
      XQueryColors(display[usewindow], map, ppixcolor,256);
    }
    data=imagetoxwd(Image,xwa.visual,ppixcolor,&len);
    bsave(name,data,len);
    XDestroyImage(Image);
    free(data);
    free(name);
}
XImage *xwdtoximage(char *,Visual *);

void c_get(PARAMETER *plist,int e) {
  int d,b,bx,by,bw,bh,len,i;
  char *data;
  XImage *Image;
  Window root;
  Colormap map;
  XColor ppixcolor[256];
  XWindowAttributes xwa;
  if(e==5) { 
    graphics();
    XGetGeometry(display[usewindow], win[usewindow], 
	&root,&bx,&by,&bw,&bh,&b,&d); 
    XGetWindowAttributes(display[usewindow], win[usewindow], &xwa);
    Image=XGetImage(display[usewindow],pix[usewindow],
                plist[0].integer,plist[1].integer,plist[2].integer,
		plist[3].integer, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
    if(d==8) {
      map =XDefaultColormapOfScreen ( XDefaultScreenOfDisplay(display[usewindow]));
      for(i=0;i<256;i++) ppixcolor[i].pixel=i;
      XQueryColors(display[usewindow], map, ppixcolor,256);
    }
    data=imagetoxwd(Image,xwa.visual,ppixcolor,&len);
    zuweissbuf(plist[4].pointer,data,len);
    XDestroyImage(Image);
    free(data);
  }
}
void c_put(PARAMETER *plist,int e) {  
  XImage *ximage;
  XWindowAttributes xwa;
   if(e==3 || e==4) {
    graphics();    
    XGetWindowAttributes(display[usewindow], win[usewindow], &xwa);
    ximage=xwdtoximage(plist[2].pointer,xwa.visual);
    if(xwa.depth!=ximage->depth)
      printf("Grafik hat die falsche Farbtiefe !\n");
    else {
      XPutImage(display[usewindow],pix[usewindow],gc[usewindow],
                ximage, 0,0,plist[0].integer,plist[1].integer, ximage->width, ximage->height);
    }
    XDestroyImage(ximage);
  }
}

void c_sget(char *n) {
    int d,b,x,y,w,h,len,i;
    char *data;
    XImage *Image;
    Window root;
    Colormap map;
    XColor ppixcolor[256];
    XWindowAttributes xwa;
    
    graphics();
    
    XGetGeometry(display[usewindow], win[usewindow], 
	&root,&x,&y,&w,&h,&b,&d); 
    XGetWindowAttributes(display[usewindow], win[usewindow], &xwa);
    
    Image=XGetImage(display[usewindow],pix[usewindow],
                0, 0, w, h, AllPlanes,(d==1) ?  XYPixmap : ZPixmap);
    if(d==8) {
      map =XDefaultColormapOfScreen ( XDefaultScreenOfDisplay(display[usewindow]));
      for(i=0;i<256;i++) ppixcolor[i].pixel=i;
      XQueryColors(display[usewindow], map, ppixcolor,256);
    }
    data=imagetoxwd(Image,xwa.visual,ppixcolor,&len);
    zuweissbuf(n,data,len);
    XDestroyImage(Image);
    free(data);
}
void c_sput(char *n) {
    STRING str=string_parser(n);
    XImage *ximage;
    XWindowAttributes xwa;

    graphics();    
    XGetWindowAttributes(display[usewindow], win[usewindow], &xwa);

    ximage=xwdtoximage(str.pointer,xwa.visual);
    XPutImage(display[usewindow],pix[usewindow],gc[usewindow],
                ximage, 0,0,0,0, ximage->width, ximage->height);
    XDestroyImage(ximage);
    free(str.pointer);
}

void c_line(PARAMETER *plist,int e) {
  graphics(); 
  line(plist[0].integer,plist[1].integer,plist[2].integer,plist[3].integer);
}
void c_box(PARAMETER *plist,int e) {
  graphics(); 
  box(plist[0].integer,plist[1].integer,plist[2].integer,plist[3].integer);
}
void c_pbox(PARAMETER *plist,int e) {
  graphics(); 
  pbox(plist[0].integer,plist[1].integer,plist[2].integer,plist[3].integer);
}

void c_dotodraw(char *n) {
  int e2,x,y;
  char *u,*t;
  t=malloc(strlen(n)+1);u=malloc(strlen(n)+1);
  xtrim(n,TRUE,n);
  e2=wort_sep(n,',',TRUE,u,t);
  if(e2<2) error(42,"DRAW TO"); /* Zu wenig Parameter */
  else {
    x=(int)parser(u);
    y=(int)parser(t);
    line(turtlex,turtley,x,y); 
    turtlex=x;
    turtley=y;
  }
  free(u);free(t);
  return;
}

void c_draw(char *n) {
  char *v,*t,*u;
  int e,e2;
  v=malloc(strlen(n)+1);t=malloc(strlen(n)+1);u=malloc(strlen(n)+1);
  strcpy(v,n);
  xtrim(v,TRUE,v);
  e=wort_sep2(v,"TO ",TRUE,t,v);  
  xtrim(t,TRUE,t);
  
  if(e>0) {
    graphics();
    if(strlen(t)) {
      e2=wort_sep(t,',',TRUE,u,t);
      
      if(e2<2) error(42,"DRAW"); /* Zu wenig Parameter */
      else {
        turtlex=(int)parser(u);
        turtley=(int)parser(t);
      }
    }
    if(e==1) line(turtlex,turtley,turtlex,turtley);
    else if(e==2) {
      e2=wort_sep2(v,"TO ",TRUE,t,v);
      while(e2) {  
        xtrim(t,TRUE,t);
        c_dotodraw(t);
	e2=wort_sep2(v,"TO ",TRUE,t,v);
      }
    }
  }
  free(v);free(t);free(u);
  return;
}

void c_circle(PARAMETER *plist,int e) {
  int r,x,y,a1=0,a2=64*360;
  if(e>=3) {
    x=plist[0].integer-plist[2].integer;
    y=plist[1].integer-plist[2].integer;
    r=2*plist[2].integer;

    if(e>=4) a1=plist[3].integer*64;
    if(e>=5) a2=plist[4].integer*64;
    
    graphics(); 
    XDrawArc(display[usewindow],pix[usewindow],gc[usewindow],x,y,r,r,a1,a2-a1); 
  }
}
void c_pcircle(PARAMETER *plist,int e) {
  int r,x,y,a1=0,a2=64*360;
  if(e>=3) {
    x=plist[0].integer-plist[2].integer;
    y=plist[1].integer-plist[2].integer;
    r=2*plist[2].integer;

    if(e>=4) a1=plist[3].integer*64;
    if(e>=5) a2=plist[4].integer*64;
    
    graphics(); 
    XFillArc(display[usewindow],pix[usewindow],gc[usewindow],x,y,r,r,a1,a2-a1); 
  }
}

void c_ellipse(PARAMETER *plist,int e) {
  int r1,r2,x,y,a1=0,a2=64*360;
  if(e>=4) {
    x=plist[0].integer-plist[2].integer;
    y=plist[1].integer-plist[3].integer;
    r1=2*plist[2].integer;
    r2=2*plist[3].integer;

    if(e>=5) a1=plist[4].integer*64;
    if(e>=6) a2=plist[5].integer*64;

    graphics(); 
    XDrawArc(display[usewindow],pix[usewindow],gc[usewindow],x,y,r1,r2,a1,a2-a1);
  }
}
void c_pellipse(PARAMETER *plist,int e) {
  int r1,r2,x,y,a1=0,a2=64*360;
  if(e>=4) {
    x=plist[0].integer-plist[2].integer;
    y=plist[1].integer-plist[3].integer;
    r1=2*plist[2].integer;
    r2=2*plist[3].integer;

    if(e>=5) a1=plist[4].integer*64;
    if(e>=6) a2=plist[5].integer*64;

    graphics(); 
    XFillArc(display[usewindow],pix[usewindow],gc[usewindow],x,y,r1,r2,a1,a2-a1);
  }
}


void c_color(char *n) {  
  graphics();
  XSetForeground(display[usewindow],gc[usewindow],(int)parser(n));
  return;
}

int get_fcolor() {
   XGCValues gc_val;  
  XGetGCValues(display[usewindow], gc[usewindow],  GCForeground, &gc_val);
  return(gc_val.foreground);
}

void c_graphmode(char *n) {
  graphics();
  set_graphmode((int)parser(n));
}
void c_setfont(char *n) {
  char *ttt;
  graphics();
  ttt=s_parser(n);
  set_font(ttt);
  free(ttt);
}

void c_scope(char *n) {                                      /* SCOPE y()[,sy[,oy,[,mod]]]   */
  char w1[strlen(n)+1],w2[strlen(n)+1];                      /* oder                         */
  int e,i=0,scip=0;			                     /* SCOPE y(),x(),sy,oy,sx,ox    */
  int vnrx=-1,vnry=-1,typ,mode=0,xoffset=0,yoffset=0;
  char *r;
  double yscale=1,xscale=1;
  
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
     scip=0;
     if(strlen(w1)) {
       switch(i) {
         case 0: { /* Array mit y-Werten */     
	   /* Typ bestimmem. Ist es Array ? */
           typ=type2(w1);
	   if(typ & ARRAYTYP) {
             r=varrumpf(w1);
             vnry=variable_exist(r,typ);
             free(r);
	     if(vnry==-1) error(15,w1); /* Feld nicht dimensioniert */
	   } else printf("SCOPE: Kein ARRAY. \n");
	   break;
	   }
	 case 1: {   /* Array mit x-Werten */
	   /* Typ bestimmem. Ist es Array ? */
           typ=type2(w1);
	   if(typ & ARRAYTYP) {
             r=varrumpf(w1);
             vnrx=variable_exist(r,typ);
             free(r);
	     if(vnrx==-1) error(15,w1); /* Feld nicht dimensioniert */
	   } else scip=1;
	   break;
	   } 
	 
	 case 2: { mode=(int)parser(w1); break; } 
	 case 3: { yscale=parser(w1); break; } 
	 case 4: {  yoffset=(int)parser(w1); break;} 
	 case 5: {  xscale=parser(w1); break;} 
	 case 6: {  xoffset=(int)parser(w1); break;} 
	   
         default: break;
       }
     }
     if(scip==0) e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(vnry!=-1) {
    int nn=do_dimension(vnry);
    int x1,x2,y1,y2;
    if((variablen[vnry].typ & FLOATTYP)) {   /* Was machen wir mit int-Arrays ????  */
      double *varptry=(double  *)(variablen[vnry].pointer+variablen[vnry].opcode*INTSIZE);
      double *varptrx;
      if(vnrx!=-1) {
        nn=min(do_dimension(vnrx),nn);
        varptrx=(double  *)(variablen[vnrx].pointer+variablen[vnrx].opcode*INTSIZE);
      }
    
      graphics();

      for(i=0;i<nn-1;i++) {
        y1=(int)(varptry[i]*yscale)+yoffset;
        y2=(int)(varptry[i+1]*yscale)+yoffset;
        if(vnrx!=-1) {
          x1=(int)(varptrx[i]*xscale)+xoffset;
          x2=(int)(varptrx[i+1]*xscale)+xoffset;
        } else {x1=i*xscale+xoffset;x2=(i+1)*xscale+xoffset;}
        if(mode==0) XDrawLine(display[usewindow],pix[usewindow],gc[usewindow],x1,y1,x2,y2);
	else if(mode==1 && vnrx!=-1) XDrawPoint(display[usewindow],pix[usewindow],gc[usewindow],x1,y1);
        else XDrawLine(display[usewindow],pix[usewindow],gc[usewindow],x1,yoffset,x1,y2); 
      }
    }
  }
}
void c_polymark(char *n) { do_polygon(0,n);}
void c_polyline(char *n) { do_polygon(1,n);}
void c_polyfill(char *n) { do_polygon(2,n);}
void do_polygon(int doit,char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int e,i=0,anz=0,shape=Nonconvex;
  int vnrx=-1,vnry=-1,typ,mode=CoordModeOrigin,xoffset=0,yoffset=0;
  char *r;
  
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 1: { /* Array mit x-Werten */     
	   /* Typ bestimmem. Ist es Array ? */
           typ=type2(w1);
	   if(typ & ARRAYTYP) {
             r=varrumpf(w1);
             vnrx=variable_exist(r,typ);
             free(r);
	     if(vnrx==-1) error(15,w1); /* Feld nicht dimensioniert */
	     else anz=min(anz,do_dimension(vnrx));
	   } else printf("POLYLINE/FILL/MARK: Kein ARRAY. \n");
	   break;
	   }
	 case 2: {   /* Array mit x-Werten */
	   /* Typ bestimmem. Ist es Array ? */
           typ=type2(w1);
	   if(typ & ARRAYTYP) {
             r=varrumpf(w1);
             vnry=variable_exist(r,typ);
             free(r);
	     if(vnry==-1) error(15,w1); /* Feld nicht dimensioniert */
	     anz=min(anz,do_dimension(vnry));
	   } else printf("POLYLINE/FILL/MARK: Kein ARRAY. \n");
	   break;
	   } 
	 
	 case 0: { anz=max(0,(int)parser(w1)); break; } 
	 case 3: { xoffset=(int)parser(w1); break; } 
	 case 4: { yoffset=(int)parser(w1); break;} 
	 case 5: { mode=(((int)parser(w1))&1) ?CoordModePrevious:CoordModeOrigin; break;} 
	 case 6: { shape=(int)parser(w1); break;} 
	   
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(vnrx!=-1 && vnry!=-1 && anz>0) {
    if((variablen[vnry].typ & FLOATTYP)) {   /* Was machen wir mit int-Arrays ????  */
      XPoint points[anz];
      double *varptry=(double  *)(variablen[vnry].pointer+variablen[vnry].opcode*INTSIZE);
      double *varptrx=(double  *)(variablen[vnrx].pointer+variablen[vnrx].opcode*INTSIZE);
      for(i=0;i<anz;i++) {
        points[i].x=(int)(varptrx[i])+xoffset;
        points[i].y=(int)(varptry[i])+yoffset;
      }
      graphics();
      if(doit==0) XDrawPoints(display[usewindow],pix[usewindow],gc[usewindow],points,anz,mode);
      else if(doit==1) XDrawLines(display[usewindow],pix[usewindow],gc[usewindow],points,anz,mode);
     else if(doit==2) XFillPolygon(display[usewindow],pix[usewindow],gc[usewindow],points,anz,shape,mode);
    }
  }
}


void set_graphmode(int n) { 
/*            n=1 copy src
              n=2 src xor dest
              n=3 invert (not dest)
              n=4 src and dest
              n=5 not (src xor dest)
              n<0 uebergibt -n an X-Server
 */  
  XGCValues gc_val;  
  switch (n) {
    case 1:gc_val.function=GXcopy;break;
    case 2:gc_val.function=GXxor;break;
    case 3:gc_val.function=GXinvert;break;
    case 4:gc_val.function=GXand;break;
    case 5:gc_val.function=GXequiv;break;
    default:
    if(n>=0) gc_val.function=n;
    else gc_val.function=-n;
    break;
  } 
  XChangeGC(display[usewindow], gc[usewindow],  GCFunction, &gc_val);
}
void set_font(char *name) {
   XGCValues gc_val;  
   XFontStruct *fs;
   fs=XLoadQueryFont(display[usewindow], name);
   if(fs!=NULL)  {
     gc_val.font=fs->fid;
     XChangeGC(display[usewindow], gc[usewindow],  GCFont, &gc_val);
   }
}

int get_graphmode() {
   XGCValues gc_val;  
  XGetGCValues(display[usewindow], gc[usewindow],  GCFunction, &gc_val);
  return(gc_val.function);
}
#include "bitmaps/fill.xbm"

void c_deffill(PARAMETER *plist,int e) {
  graphics();
  if(e>=1 && plist[0].typ!=PL_LEER){
    int fs=plist[0].integer;
    if(fs>=0 && fs<2) XSetFillRule(display[usewindow], gc[usewindow], fs);
  } 
  if(e>=2 && plist[1].typ!=PL_LEER){
    int fs=plist[1].integer;
    if(fs>=0 && fs<4) XSetFillStyle(display[usewindow], gc[usewindow], fs);
  }
  if(e>=3 && plist[2].typ!=PL_LEER){
    Pixmap fill_pattern;
    int pa=plist[2].integer;
    pa=max(0,min(fill_height/fill_width,pa));
    fill_pattern = XCreateBitmapFromData(display[usewindow],win[usewindow],
                fill_bits+pa*fill_width*fill_width/8,fill_width,fill_width);
           /*XSetFillStyle(display[usewindow], gc[usewindow], FillStippled); */
	   
           XSetStipple(display[usewindow], gc[usewindow],fill_pattern );
  }
}
void c_defline(PARAMETER *plist,int e) { 
  static int style=0,width=0,cap=0,join=0;
  if(e>=1 && plist[0].typ!=PL_LEER) style=plist[0].integer;
  if(e>=2 && plist[1].typ!=PL_LEER) width=plist[1].integer;
  if(e>=3 && plist[2].typ!=PL_LEER)   cap=plist[2].integer;
  if(e>=4 && plist[3].typ!=PL_LEER)  join=plist[3].integer;
  graphics();
  XSetLineAttributes(display[usewindow], gc[usewindow],width,style,cap,join);
}
void c_copyarea(PARAMETER *plist,int e) {
  if(e==6) {
    graphics();   
    XCopyArea(display[usewindow],pix[usewindow],pix[usewindow],gc[usewindow],
    plist[0].integer,plist[1].integer,plist[2].integer,
    plist[3].integer,plist[4].integer,plist[5].integer);
  }
}

double ltextwinkel=0,ltextxfaktor=0.3,ltextyfaktor=0.5;
int ltextpflg=0;

void c_deftext(PARAMETER *plist,int e) {
  if(e>=1 && plist[0].typ!=PL_LEER)    ltextpflg=plist[0].integer;
  if(e>=2 && plist[1].typ!=PL_LEER) ltextxfaktor=plist[1].real;
  if(e>=3 && plist[2].typ!=PL_LEER) ltextyfaktor=plist[2].real;
  if(e>=4 && plist[3].typ!=PL_LEER)  ltextwinkel=plist[3].real;
}


int mousex() {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(win_x_return);
}
int mousey() {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(win_y_return);
}
int mousek() {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(mask_return>>8);
}
int mouses() {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
   return(mask_return & 255);
}

void c_mouse(PARAMETER *plist,int e) {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);

  if(e>=1 && plist[0].typ!=PL_LEER) zuweis(plist[0].pointer,(double)win_x_return);
  if(e>=2 && plist[1].typ!=PL_LEER) zuweis(plist[1].pointer,(double)win_y_return);
  if(e>=3 && plist[2].typ!=PL_LEER) zuweis(plist[2].pointer,(double)mask_return);
  if(e>=4 && plist[3].typ!=PL_LEER) zuweis(plist[3].pointer,(double)root_x_return);
  if(e>=5 && plist[4].typ!=PL_LEER) zuweis(plist[4].pointer,(double)root_y_return);

#ifdef DEBUG
  printf("Mouse: x=%d y=%d m=%d\n",win_x_return,win_y_return,mask_return);
#endif
}

void c_setmouse(PARAMETER *plist,int e) {
  int mode=0;
  if(e>=2) {
    if(e==3) mode=plist[2].integer;
    graphics();
    if(mode==0) XWarpPointer(display[usewindow], None, win[usewindow], 0, 0,0,0,plist[0].integer,plist[1].integer);
    else if(mode==1) XWarpPointer(display[usewindow], None, None, 0, 0,0,0,plist[0].integer,plist[1].integer);
  }
}


void c_mouseevent(char *n) {
   XEvent event;

   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
   graphics();
    
   XWindowEvent(display[usewindow], win[usewindow],ButtonPressMask|ExposureMask, &event);
   while(event.type!=ButtonPress) { 
     handle_event(usewindow,&event);
     XWindowEvent(display[usewindow], win[usewindow],ButtonPressMask|ExposureMask, &event);
   }
   if(event.type==ButtonPress) {
     e=wort_sep(n,',',TRUE,w1,w2);
     while(e) {
       if(strlen(w1)) {
         switch(i) {
         case 0: {zuweis(w1,(double)event.xbutton.x);     break;}
	 case 1: {zuweis(w1,(double)event.xbutton.y);     break;} /* Dicke */
	 case 2: {zuweis(w1,(double)event.xbutton.button);break;} 
	 case 3: {zuweis(w1,(double)event.xbutton.x_root);break;} 
	 case 4: {zuweis(w1,(double)event.xbutton.y_root);break;}   
	 case 5: {zuweis(w1,(double)event.xbutton.state); break;}  
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
    }
  } 
}

void c_motionevent(char *n) {
   XEvent event;

   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
   graphics();
    
   XWindowEvent(display[usewindow], win[usewindow],PointerMotionMask|ExposureMask, &event);
   while(event.type!=MotionNotify) { 
     handle_event(usewindow,&event);
     XWindowEvent(display[usewindow], win[usewindow],PointerMotionMask|ExposureMask, &event);
   }   
   if(event.type==MotionNotify) {
     e=wort_sep(n,',',TRUE,w1,w2);
     while(e) {
       if(strlen(w1)) {
         switch(i) {
         case 0: {zuweis(w1,(double)event.xmotion.x);     break;}
	 case 1: {zuweis(w1,(double)event.xmotion.y);     break;} 
	 case 2: {zuweis(w1,(double)event.xmotion.x_root);break;} 
	 case 3: {zuweis(w1,(double)event.xmotion.y_root);break;} 
	 case 4: {zuweis(w1,(double)event.xmotion.state); break;}
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
    }
  }
}

void c_keyevent(char *n) {
   XEvent event;

   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
   graphics();
    
   XWindowEvent(display[usewindow], win[usewindow],KeyPressMask|ExposureMask, &event);
   while(event.type!=KeyPress) { 
     handle_event(usewindow,&event);
     XWindowEvent(display[usewindow], win[usewindow],KeyPressMask|ExposureMask, &event);
   }   
   if(event.type==KeyPress) {
     char buf[4];
     XComposeStatus status;
     KeySym ks;
     
     XLookupString((XKeyEvent *)&event,buf,sizeof(buf),&ks,&status);   

     e=wort_sep(n,',',TRUE,w1,w2);
     while(e) {
       if(strlen(w1)) {
         switch(i) {
         case 0: {zuweis(w1,(double)event.xkey.keycode);break;}
	 case 1: {zuweis(w1,(double)ks);                break;} 
	 case 2: {zuweiss(w1,buf);                      break;} 
	 case 3: {zuweis(w1,(double)event.xkey.state);  break;} 
	 case 4: {zuweis(w1,(double)event.xkey.x);      break;} 
	 case 5: {zuweis(w1,(double)event.xkey.y);      break;} 
	 case 6: {zuweis(w1,(double)event.xkey.x_root); break;}  
	 case 7: {zuweis(w1,(double)event.xkey.y_root); break;}  
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
    }
  }
}
void c_allevent(char *n) {
   XEvent event;

   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
   graphics();
    
   XWindowEvent(display[usewindow], win[usewindow],
         KeyPressMask|ButtonPressMask|PointerMotionMask|ExposureMask, &event);
   while(event.type==Expose) { 
     handle_event(usewindow,&event);
     XWindowEvent(display[usewindow], win[usewindow],KeyPressMask|ButtonPressMask|PointerMotionMask|ExposureMask, &event);
   }   
   
     e=wort_sep(n,',',TRUE,w1,w2);
     while(e) {
       if(strlen(w1)) {
         switch(i) {
         case 0: {zuweis(w1,(double)event.type);          break;}
	 case 1: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)event.xmotion.x);     
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.x);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.x);
	   break;}
	 case 2: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)event.xmotion.y);     
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.y);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.y);
	   break;}
	 case 3: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)event.xmotion.x_root);     
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.x_root);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.x_root);
	   break;}
	 case 4: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)event.xmotion.y_root);     
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.y_root);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.y_root);
	   break;}
	 case 5: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)(event.xmotion.state &255));   
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.state);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.state);
	   break;}
	 case 6: {
	   if(event.type==MotionNotify)     zuweis(w1,(double)(event.xmotion.state>>8));     
	   else if(event.type==ButtonPress) zuweis(w1,(double)event.xbutton.button);
	   else if(event.type==KeyPress)    zuweis(w1,(double)event.xkey.keycode);
	   break;}
	 case 7: {
	   if(event.type==MotionNotify)     ;     
	   else if(event.type==ButtonPress) ;
	   else if(event.type==KeyPress)  {
             char buf[4];
             XComposeStatus status;
             KeySym ks;
     
             XLookupString((XKeyEvent *)&event,buf,sizeof(buf),&ks,&status);   
             zuweis(w1,(double)ks);       
           }
	   break;}
	 case 8: {
	   if(event.type==MotionNotify)     ;     
	   else if(event.type==ButtonPress) ;
	   else if(event.type==KeyPress)  {
             char buf[4];
             XComposeStatus status;
             KeySym ks;
     
             XLookupString((XKeyEvent *)&event,buf,sizeof(buf),&ks,&status);   
             zuweiss(w1,buf);       
           }
	   break;}
	   
	   
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
    }
}
void c_infow(char *n) {
  error(9,"INFOW");
}
void c_titlew(char *n) {
  char *v,*t;
  int winnr=DEFAULTWINDOW,e;
 
  graphics();
  v=malloc(strlen(n)+1);t=malloc(strlen(n)+1);
  e=wort_sep(n,',',TRUE,v,t);
  if(e) { if(v[0]=='#') winnr=(int)parser(v+1); else winnr=(int)parser(v);}
  if(winnr<MAXWINDOWS) {
    if (!XStringListToTextProperty(&n, 1, &win_name[winnr]))    printf("Couldn't set Name of Window.\n");
    XSetWMName(display[usewindow], win[usewindow], &win_name[usewindow]);
  } else printf("Ungueltige Windownr. %d. Maximal %d \n",winnr,MAXWINDOWS);
  free(v);free(t);
}
void c_clearw(char *n) {
  int winnr=usewindow,x,y,w,h,b,d;
  
  if(strlen(n)) {
    if(n[0]=='#') winnr=max(0,(int)parser(n+1)); else winnr=max(0,(int)parser(n));
  }
  if(winnr<MAXWINDOWS) {
    graphics();
    if(winbesetzt[winnr]) {  	
      Window root;
      GC sgc;   
      XGCValues gc_val;  
       /* Erst den Graphic-Kontext retten  */
    sgc=XCreateGC(display[winnr], win[winnr], 0, &gc_val);
    XCopyGC(display[winnr], gc[winnr],GCForeground , sgc);
      XGetGeometry(display[winnr],win[winnr],&root,&x,&y,&w,&h,&b,&d);
      XSetForeground(display[winnr],gc[winnr],0);
      XFillRectangle(display[winnr],pix[winnr],gc[winnr],x,y,w,h); 

      /* XClearWindow(display[winnr],win[winnr]); */
      
      XCopyGC(display[winnr], sgc,GCForeground, gc[winnr]);
      XFreeGC(display[winnr],sgc); 
   
    }
  }
}
void c_closew(char *n) {
  int winnr=usewindow;
  graphics();
  if(strlen(n)) {
    if(n[0]=='#') winnr=max(0,(int)parser(n+1)); else winnr=max(0,(int)parser(n));
  }
  close_window(winnr);
}
void c_openw(char *n) {
  int winnr=DEFAULTWINDOW;
  graphics();
  if(strlen(n)) {
    if(n[0]=='#') winnr=max(0,(int)parser(n+1)); else winnr=max(0,(int)parser(n));
  }
  open_window(winnr);
}

void c_sizew(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int winnr=usewindow,e,i=0;
  int w=640,h=400;
  
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
     if(strlen(w1)) {
         switch(i) {
         case 0: {
	   if(w1[0]=='#') winnr=(int)parser(w1+1);
	   else winnr=(int)parser(w1);
	   break;
	   }
	 case 1: {w=max(0,(int)parser(w1));  break;} 
	 case 2: {h=max(0,(int)parser(w1));  break;} 
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
    
  if(winnr<MAXWINDOWS && winnr>0 ) {
    graphics();
    if(winbesetzt[winnr]) {
        Pixmap pixi;
	Window root;
	int ox,oy,ow,oh,ob,d;
	
	XGetGeometry(display[winnr],win[winnr],&root,&ox,&oy,&ow,&oh,&ob,&d);
	pixi=XCreatePixmap(display[winnr], win[winnr], w, h, d);
        XResizeWindow(display[winnr], win[winnr], w, h);
	XCopyArea(display[winnr],pix[winnr],pixi,gc[winnr],0,0,min(w,ow),min(h,oh),0,0);
	XFreePixmap(display[winnr],pix[winnr]);	
	pix[winnr]=pixi;
	XFlush(display[winnr]);
    } else  printf("Window existiert nicht. \n");
  } else if(winnr==0) printf("Rootwindow laesst sich nicht veraendern. \n");
  else printf("Ungueltige Windownr. %d. Maximal %d \n",winnr,MAXWINDOWS);
}

void c_movew(char *n) {
  char v[strlen(n)+1],t[strlen(n)+1];
  int winnr=usewindow,e;
 
  graphics();
  wort_sep(n,',',TRUE,v,t);
  if(strlen(v)) { if(v[0]=='#') winnr=(int)parser(v+1); else winnr=(int)parser(v);}
  if(winnr<MAXWINDOWS) {
    int x=100,y=100;
    e=wort_sep(t,',',TRUE,v,t);
    if(e) {
      x=max((int)parser(v),0);
      e=wort_sep(t,',',TRUE,v,t);
      if(e) {
        y=max((int)parser(v),0);
        XMoveWindow(display[winnr], win[winnr], x, y);
      }
    }
  } else if(winnr==0) printf("Rootwindow laesst sich nicht veraendern. \n");
  else printf("Ungueltige Windownr. %d. Maximal %d \n",winnr,MAXWINDOWS);
}


#include "bitmaps/biene.bmp"
#include "bitmaps/biene_mask.bmp"
#include "bitmaps/hand.bmp"
#include "bitmaps/hand_mask.bmp"
#include "bitmaps/zeigehand.bmp"
#include "bitmaps/zeigehand_mask.bmp"

void c_defmouse(char *n) {
  int form=(int)parser(n),formt;
  Cursor maus;
 
  if(form<0) formt=-form;   
  else if(form==0) formt=68;     /* Pfeil  */
  else if(form==1) formt=152;    /* Doppelklammer  */
  else if(form==2) formt=150;    /* Biene  bzw. Sanduhr*/
  else if(form==3) formt=60;     /* Hand mit Zeigefinger  */
  else if(form==4) formt=24;     /* offene Hand */
  else if(form==5) formt=34;     /* Fadenkreuz fein */
  else if(form==6) formt=90;     /* Fadenkreuz grob */
  else if(form==7) formt=30;     /* Fadenkreuz weiss mit Umrandung */
  else if(form>=8) formt=(form-8)*2;
  if(form==2 || form==3 || form==4) {
    Pixmap mausp,mausm;
    XColor f,b;
    int hotx=8,hoty=8;
    graphics();
    if(form==2) {
      mausp=XCreateBitmapFromData(display[usewindow],win[usewindow],
                biene_bits,biene_width,biene_height);
      mausm=XCreateBitmapFromData(display[usewindow],win[usewindow],
                biene_mask_bits,biene_mask_width,biene_mask_height);
    } else if(form==3) {
      hotx=0;hoty=0;
      mausp=XCreateBitmapFromData(display[usewindow],win[usewindow],
                zeigehand_bits,zeigehand_width,zeigehand_height);
      mausm=XCreateBitmapFromData(display[usewindow],win[usewindow],
                zeigehand_mask_bits,zeigehand_mask_width,zeigehand_mask_height);
    } else if(form==4) {
      mausp=XCreateBitmapFromData(display[usewindow],win[usewindow],
                hand_bits,hand_width,hand_height);
      mausm=XCreateBitmapFromData(display[usewindow],win[usewindow],
                hand_mask_bits,hand_mask_width,hand_mask_height);

    }
    f.pixel=1;b.pixel=0;
    f.red=0;b.red=65535;    f.green=0;b.green=65535;f.blue=0;b.blue=65535;
    
    maus=XCreatePixmapCursor(display[usewindow],mausp,mausm,&f,&b,hotx,hoty);
    XDefineCursor(display[usewindow], win[usewindow], maus);
    XFreePixmap(display[usewindow],mausp);XFreePixmap(display[usewindow],mausm);
  } else if(formt<153 && formt>=0) {
    graphics();
    maus=XCreateFontCursor(display[usewindow],formt);
    XDefineCursor(display[usewindow], win[usewindow], maus);
  }
}

void c_text(char *n) {
  int x,y,e;
  char *v,*t,*buffer;
  graphics();
  v=malloc(strlen(n)+1);t=malloc(strlen(n)+1);
  e=wort_sep(n,',',TRUE,v,t);
  if(e) x=parser(v);
  e=wort_sep(t,',',TRUE,v,t);
  if(e) y=parser(v);
  buffer=s_parser(t);
  XDrawString(display[usewindow],pix[usewindow],gc[usewindow],x,y,buffer,strlen(buffer));
  free(v);free(t);free(buffer);
}


void c_ltext(char *n) {
  int x,y,e;
  char *v,*t,*buffer;
  graphics();
  v=malloc(strlen(n)+1);t=malloc(strlen(n)+1);
  e=wort_sep(n,',',TRUE,v,t);
  if(e) x=parser(v);
  e=wort_sep(t,',',TRUE,v,t);
  if(e) y=parser(v);
  buffer=s_parser(t);
  ltext(x,y,ltextxfaktor,ltextyfaktor,ltextwinkel,ltextpflg,buffer); 

  free(v);free(t);free(buffer);
}

void c_alert(PARAMETER *plist,int e) {
  /* setzt nur das Format in einen FORM_ALERT Aufruf um */
  char buffer[MAXSTRLEN];
  
  if(e==5) {
    sprintf(buffer,"[%d][%s][%s]",plist[0].integer,plist[1].pointer,plist[3].pointer);
    zuweis(plist[4].pointer,(double)form_alert(plist[2].integer,buffer));
  } 
}

char *fsel_input(char *,char *,char *);

void c_fileselect(PARAMETER *plist,int e) { 
  if(e==4) {
    char *backval=fsel_input(plist[0].pointer,plist[1].pointer,plist[2].pointer);
    zuweiss(plist[3].pointer,backval);
    free(backval);
  } 
}

#define MAXMENUENTRYS 200
#define MAXMENUTITLES 80

int menuaction=-1;
int menuflags[MAXMENUENTRYS];
char *menuentry[MAXMENUENTRYS];
char *menutitle[MAXMENUTITLES];
int menutitlesp[MAXMENUTITLES];
int menutitlelen[MAXMENUTITLES];
int menutitleflag[MAXMENUTITLES];
int menuanztitle=0;

void c_menu(char *n) {
  int sel;
  int pc2;
  char pos[20];
  if(menuaction!=-1) {
    sel=do_menu_select();
    if(sel>=0) {
      sprintf(pos,"%d",sel);
      if(do_parameterliste(pos,procs[menuaction].parameterliste)) error(42,pos); /* Zu wenig Parameter */
      else {
        batch=1;
        pc2=procs[menuaction].zeile;
        if(sp<STACKSIZE) {stack[sp++]=pc;pc=pc2+1;}
        else {printf("Stack-Overflow ! PC=%d\n",pc); batch=0;}
      }
    }
  }
}
void c_menudef(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int i=0,e;
  int count=0,nn,vnr=-1,typ,pc2=-1;
  char *r;
  STRING *varptr;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
       if(strlen(w1)) {
       switch(i) {
         case 0: {
	   typ=type2(w1);
	   if(typ & STRINGARRAYTYP) {
             r=varrumpf(w1);
             vnr=variable_exist(r,typ);
             free(r);
	     if(vnr==-1) error(15,w1); /* Feld nicht dimensioniert */
	   } else printf("MENUDEF: Kein ARRAY. \n");
	   break;
	   }
	 case 1: {   
	     pc2=procnr(w1,1);
	 break;
	   } 	 
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(i<2) {
    error(42,""); /* Zu wenig Parameter  */
  } else {
    if(pc2==-1)   error(19,w2); /* Procedure nicht gefunden */
    else {
      if(vnr>-1) {
 	nn=do_dimension(vnr);
	varptr=(STRING *)(variablen[vnr].pointer+variablen[vnr].opcode*INTSIZE);
	menuanztitle=0;
	count=0;
	for(i=0;i<nn;i++) {
	  menuentry[i]=varptr[i].pointer;
	  if(count==0 && varptr[i].len) {
	    menutitle[menuanztitle]=varptr[i].pointer;
	    menutitlesp[menuanztitle]=i+1;
	    menutitleflag[menuanztitle]=NORMAL;
	    count++;
	  } else if(varptr[i].len) {
	    if(*(varptr[i].pointer)=='-') menuflags[i]=DISABLED; else menuflags[i]=NORMAL;
	    count++;
	  } else if(count) {
	    menutitlelen[menuanztitle]=count-1;
	    menuanztitle++;
	    count=0;
	  }
	}
	menutitlelen[menuanztitle]=count;
	menuanztitle++;
        menuaction=pc2;
        do_menu_draw(); 
      }
    }
  }
}
void c_menuset(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int i=0,e;
  int nr,flag;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
       if(strlen(w1)) {
       switch(i) {
         case 0: {
	   nr=parser(w1);
	   break;
	   }
	 case 1: {   
	   flag=parser(w1);
	   break;
	   } 	 
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(i<2) {
    error(42,""); /* Zu wenig Parameter  */
  } else {
  
  if(nr<MAXMENUENTRYS && nr>0) menuflags[nr]=flag;    
  else printf("Nr. des Menueintrags zu gross. Max %d.\n",MAXMENUENTRYS);
  }
}
void c_menukill(char *n) {
  if(menuaction!=-1) {
    menuaction=-1;
  }
}


/*********************************************************/



int rsrc_load(char *);

void c_rsrc_load(char *n) {
  char *pname=s_parser(n);
  if(rsrc_load(pname)) printf("Fehler bei RSRC_LOAD.\n");
  free(pname);
}
void c_rsrc_free(char *n) {
  if(rsrc_free()) printf("Fehler bei RSRC_FREE.\n");
}
void c_form_do(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int backval;
  int i=0,e;
  char *varname=NULL;
  int tnr=0;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
       if(strlen(w1)) {
       switch(i) {
         case 0: { tnr=(int)parser(w1); break; }
	 case 1: {
	   varname=malloc(strlen(w1)+1);
	   strcpy(varname,w1);
	   break;
	   } 
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(i>=1) {	  
    OBJECT *tree;
    graphics();
    gem_init();
    rsrc_gaddr(R_TREE,tnr,&tree);
    backval=form_do(tree);
    zuweis(varname,(double)backval);
  } else error(42,""); /* Zu wenig Parameter  */
  free(varname);
}
void c_alert_do(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int backval;
  int i=0,e;
  char *varname=NULL;
  int tnr=0,def=1;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
       if(strlen(w1)) {
       switch(i) {
        case 0: {tnr=(int)parser(w1);break;}
        case 1: {
	   def=(int)parser(w1);
	   break;
	   }
	 
	 case 2: {
	   varname=malloc(strlen(w1)+1);
	   strcpy(varname,w1);
	   break;
	   } 
         default: break;
       }
     }
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  if(i>=2) {	  
    char *tree;
    graphics();
    gem_init();
    rsrc_gaddr(R_FRSTR,tnr,&tree);
    backval=form_alert(def,tree);
    zuweis(varname,(double)backval);
  } else error(42,""); /* Zu wenig Parameter  */
  free(varname);
}

void c_xload(char *n) {
    char *name=fsel_input("Programm laden:","./*.bas","");
    if(strlen(name)) {
      if(exist(name)) {
        programbufferlen=0; 
        mergeprg(name);
      } else printf("LOAD/MERGE: Datei %s nicht gefunden !\n",name);
    }
    free(name);
}
void c_xrun(char *n) {
  c_xload(n);
  c_run("");
}