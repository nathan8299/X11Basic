' rootwindow
CLEARW 1
COLOR COLOR_RGB(1,1,0)
PCIRCLE 400,10,10
COLOR COLOR_RGB(1,0,0)
FOR i=0 to 5
  pcircle i*20+10,4,4
  line i*20+10,4,i*20+10-2*i,20
  line i*20+10,4,i*20+10+2*i,20
  copyarea i*20,0,20,20,(10-i)*20,0
  print i
next i

COLOR COLOR_RGB(1,1,0)
LINE 150,4,150,0
FOR i=0 to 10
  color get_color(i*6553,32000+i*6553/2,i*6553)
  pcircle 350+i/3,20-i/3,20-2*i
next i
COPYAREA 330,0,40,40,200,200
TEXT 100,170,"Copyarea mit XBASIC V.1.03   (c) Markus Hoffmann"
i=0
REPEAT
  COPYAREA 600,380,20,20,x,y+20
  MOUSE x,y,k
  COPYAREA x,y+20,20,20,600,380
  GRAPHMODE 2
  COPYAREA i*20,0,20,20,x,y+20
  GRAPHMODE 1
  SHOWPAGE
  PAUSE 0.1
  INC i
  i=i MOD 10
UNTIL k<>0
QUIT
