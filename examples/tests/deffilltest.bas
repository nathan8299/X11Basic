' deffill <rule>,<style>,<pattern>


weiss=get_color(65535,65535,65535)
rot=get_color(65535,32000,32000)
schwarz=get_color(0,0,0)
color weiss
pbox 0,0,640,400

for j=0 to 6
  for i=0 to 6
    color schwarz
    deffill ,2,i+j*7
    pbox j*32,i*32,j*32+32,i*32+16
    color get_color((i+j*7)*4000,32000,32000)
    pcircle 320+j*50,i*64,32
    color schwarz
    deffill ,0
    text 5+j*32,i*32+30,str$(i+j*7)
    color rot
    box j*32,i*32,j*32+32,i*32+32
  next i
  vsync
next j
vsync
end