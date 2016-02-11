
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
#include "protos.h"
#include "xbasic.h"
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

void c_plot(char *n) {
  char v[MAXVARNAMLEN],t[MAXSTRLEN];
  int e;
  
  e=wort_sep(n,',',TRUE,v,t);
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics();
     XDrawPoint(display[usewindow],pix[usewindow],gc[usewindow],(int)parser(v),(int)parser(t));
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

void c_box(char *n) {
 c_x4(n,1);
}
void c_pbox(char *n) {
 c_x4(n,2);
}
void c_line(char *n) {
 c_x4(n,0);
}

int c_x4(char *n, int k) {
  char v[MAXSTRLEN],t[MAXSTRLEN],u[MAXSTRLEN],w[MAXSTRLEN];
  int e;
  
  wort_sep(n,',',TRUE,v,t);
  wort_sep(t,',',TRUE,t,u);
  e=wort_sep(u,',',TRUE,u,w);
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics(); 
    
    if(k==0) line((int)parser(v),(int)parser(t),(int)parser(u),(int)parser(w)); 
    else if(k==1) box((int)parser(v),(int)parser(t),(int)parser(u),(int)parser(w)); 
    else if(k==2) pbox((int)parser(v),(int)parser(t),(int)parser(u),(int)parser(w)); 
  }
  return(0);
}

void c_dotodraw(char *n) {
  int e2,x,y;
  char *u,*t;
  t=malloc(strlen(n)+1);u=malloc(strlen(n)+1);
  xtrim(n,TRUE,n);
  e2=wort_sep(n,',',TRUE,u,t);
  if(e2<2) printf("Zuwenig Parameter !\n");
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
      
      if(e2<2) printf("Zuwenig Parameter !\n");
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

void c_circle(char *n) {
  char v[MAXSTRLEN],t[MAXSTRLEN],u[MAXSTRLEN];
  int e,r,x,y,a1=0,a2=64*360;
  
  wort_sep(n,',',TRUE,v,t);
  e=wort_sep(t,',',TRUE,t,u);
  
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics(); 
    x=(int)parser(v);
    y=(int)parser(t);   
    wort_sep(u,',',TRUE,u,t);
    e=wort_sep(t,',',TRUE,t,v); 
    r=(int)parser(u);
    if(e==2) {a1=(int)parser(t)*64;a2=(int)parser(v)*64;}
    XDrawArc(display[usewindow],pix[usewindow],gc[usewindow],x-r,y-r,2*r,2*r,a1,a2-a1); 
  }
  return;
}

void c_ellipse(char *n) {
  char v[MAXSTRLEN],t[MAXSTRLEN],u[MAXSTRLEN],w[MAXSTRLEN];
  int e,r1,r2,x,y,a1=0,a2=64*360;
  
  wort_sep(n,',',TRUE,v,t);
  wort_sep(t,',',TRUE,t,u);
  e=wort_sep(u,',',TRUE,u,w);
  
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics(); 
    x=(int)parser(v);
    y=(int)parser(t); 
    r1=(int)parser(u);   
    wort_sep(w,',',TRUE,w,t);
    e=wort_sep(t,',',TRUE,t,v); 
    r2=(int)parser(w);
    if(e==2) {a1=(int)parser(t)*64;a2=(int)parser(v)*64;}
    XDrawArc(display[usewindow],pix[usewindow],gc[usewindow],x-r1,y-r2,2*r1,2*r2,a1,a2-a1);
  }
  return;
}

void c_pellipse(char *n) {
  char v[MAXSTRLEN],t[MAXSTRLEN],u[MAXSTRLEN],w[MAXSTRLEN];
  int e,r1,r2,x,y,a1=0,a2=64*360;
  
  wort_sep(n,',',TRUE,v,t);
  wort_sep(t,',',TRUE,t,u);
  e=wort_sep(u,',',TRUE,u,w);
  
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics(); 
    x=(int)parser(v);
    y=(int)parser(t); 
    r1=(int)parser(u);   
    wort_sep(w,',',TRUE,w,t);
    e=wort_sep(t,',',TRUE,t,v); 
    r2=(int)parser(w);
    if(e==2) {a1=(int)parser(t)*64;a2=(int)parser(v)*64;}
    XFillArc(display[usewindow],pix[usewindow],gc[usewindow],x-r1,y-r2,2*r1,2*r2,a1,a2-a1); 
  }  
  return;
}
void c_pcircle(char *n) {
  char v[MAXSTRLEN],t[MAXSTRLEN],u[MAXSTRLEN];
  int e,r,x,y,a1=0,a2=64*360;
  
  wort_sep(n,',',TRUE,v,t);
  e=wort_sep(t,',',TRUE,t,u);
  
  if(e<2) printf("Zuwenig Parameter !\n");
  else {
    graphics(); 
    x=(int)parser(v);
    y=(int)parser(t);   
    wort_sep(u,',',TRUE,u,t);
    e=wort_sep(t,',',TRUE,t,v); 
    r=(int)parser(u);
    if(e==2) {a1=(int)parser(t)*64;a2=(int)parser(v)*64;}
    XFillArc(display[usewindow],pix[usewindow],gc[usewindow],x-r,y-r,2*r,2*r,a1,a2-a1); 
  }
  return;
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

void set_graphmode(int n) {
   XGCValues gc_val;  
  gc_val.function=n;
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

void c_deffill(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int e,i=0;
  e=wort_sep(n,',',TRUE,w1,w2);
  graphics();
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 0: break;
	 case 1: {
	   int fs;
	   fs=(int)parser(w1);
	   if(fs>=0 && fs<2) XSetFillRule(display[usewindow], gc[usewindow], fs);
	   break;
	   } 
	 case 2: {
	   int fs;
	   fs=(int)parser(w1);
	   if(fs>=0 && fs<4) XSetFillStyle(display[usewindow], gc[usewindow], fs);
	   break;
	   }
         default: break;
       }
     }
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
}
void c_defline(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int e,i=0;
  static int width=0,style=0,cap=0,join=0;
  e=wort_sep(n,',',TRUE,w1,w2);
  graphics();
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 0: {
	   style=min(max((int)parser(w1)-1,0),3);
	   break;
	   }
	 case 1: {   /* Dicke */
	   width=max((int)parser(w1),0);
	   break;
	   } 
	 
	 case 2: {
	   cap=min(max((int)parser(w1),0),3);
	   break;
	   } 
	 case 3: {
	   join=min(max((int)parser(w1),0),2);
	   break;
	   } 
	   
         default: break;
       }
     }
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  XSetLineAttributes(display[usewindow], gc[usewindow], width, style,cap,join);
}

double ltextwinkel=0,ltextxfaktor=0.3,ltextyfaktor=0.5;
int ltextpflg=0;

void c_deftext(char *n) {
  char w1[strlen(n)+1],w2[strlen(n)+1];
  int e,i=0;
  e=wort_sep(n,',',TRUE,w1,w2);
  graphics();
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 0: {
	   ltextpflg=(int)parser(w1);
	   break;
	   }
	 case 1: {   /* Dicke */
	   ltextxfaktor=parser(w1);
	   break;
	   } 
	 
	 case 2: {
	   ltextyfaktor=parser(w1);
	   break;
	   } 
	 case 3: {
	   ltextwinkel=parser(w1);
	   break;
	   } 
	   
         default: break;
       }
     }
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
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

void c_mouse(char *n) {
   Window root_return,child_return;
   int root_x_return, root_y_return,win_x_return, win_y_return,mask_return;
   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
   graphics();
   
   XQueryPointer(display[usewindow], win[usewindow], &root_return, &child_return,
       &root_x_return, &root_y_return,
       &win_x_return, &win_y_return,&mask_return);
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 0: {
	   zuweis(w1,(double)win_x_return);
	   break;
	   }
	 case 1: {   /* Dicke */
	   zuweis(w1,(double)win_y_return);
	   break;
	   } 
	 
	 case 2: {
	   zuweis(w1,(double)mask_return);
	   break;} 
	 case 3: {zuweis(w1,(double)root_x_return);break;}
	 case 4: {zuweis(w1,(double)root_y_return);break;}  
         default: break;
       }
     }
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
#ifdef DEBUG
  printf("Mouse: x=%d y=%d m=%d\n",win_x_return,win_y_return,mask_return);
#endif
}

void c_setmouse(char *n) {
   int root_x, root_y,win_x, win_y,mode=0;
   int e,i=0;
   char w1[strlen(n)+1],w2[strlen(n)+1];
   
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
     if(strlen(w1)) {
       switch(i) {
         case 0: {
	   win_x=(int)parser(w1);
	   break;
	   }
	 case 1: {   
	   win_y=(int)parser(w1);
	   break;
	   } 
	 
	 case 2: {mode=(int)parser(w1);
	   
	   break;} 
	 case 3: {break;}
	 case 4: {break;}  
         default: break;
       }
     }
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }   
  graphics();
  if(mode==0) XWarpPointer(display[usewindow], None, win[usewindow], 0, 0,0,0, win_x, win_y);
  else if(mode==1) XWarpPointer(display[usewindow], None, None, 0, 0,0,0, win_x, win_y);
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

void c_alert(char *n) {
  /* setzt nur das Format in einen FORM_ALERT Aufruf um */
  char w1[strlen(n)+1],w2[strlen(n)+1];
  char *buffer=malloc(MAXSTRLEN);
  char *boxtext,*buttontext;
  int dbut,i=0,symbol,e;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e>1) {
       switch(i) {
         case 0: {
	   symbol=(int)parser(w1);
	   break;
	   }
	 case 1: {   
	   boxtext=s_parser(w1);
	   break;
	   } 
	 
	 case 2: {
	   dbut=(int)parser(w1);
	   break;
	   } 
	 case 3: {
	   buttontext=s_parser(w1);
	   break;
	   } 
	 case 4: {
	   ; /* nixtun, denn w1=backvar  */
	 }
         default: break;
       }
     
  
     e=wort_sep(w2,',',TRUE,w1,w2);
     i++;
  }
  
  if(i>=4) {
    sprintf(buffer,"[%d][%s][%s]",symbol,boxtext,buttontext);
    zuweis(w1,(double)form_alert(dbut,buffer));
  } else error(42,""); /* Zu wenig Parameter  */
  if(i>1) free(boxtext);
  if(i>3) free(buttontext);
  free(buffer);
}

char *fsel_input(char *,char *,char *);

void c_fileselect(char *n) {
  
  char w1[strlen(n)+1],w2[strlen(n)+1];
  char *backval;
  int i=0,e;
  char *titel=NULL,*pfad=NULL,*sel=NULL,*varname=NULL;
  e=wort_sep(n,',',TRUE,w1,w2);
  while(e) {
       if(strlen(w1)) {
       switch(i) {
         case 0: {
	   titel=s_parser(w1);
	   break;
	   }
	 case 1: {   
	   pfad=s_parser(w1);
	   break;
	   } 
	 
	 case 2: {
	   sel=s_parser(w1);
	   break;
	   } 
	 case 3: {
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
  if(i>=3) {
    backval=fsel_input(titel,pfad,sel);
    
    zuweiss(varname,backval);
    
    free(backval);
  } else error(42,""); /* Zu wenig Parameter  */
  
  free(sel);free(varname);free(pfad);free(titel);
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
