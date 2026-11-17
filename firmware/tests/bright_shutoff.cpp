#include "../NightlightRevK/Lamp.h"
#include <cassert>
int main() {
 LampSettings s; s.shutoffTenths=200; Lamp l; l.begin(0);
 assert(l.tick(10000,true,false,false,s)==0);
 assert(l.tick(11000,true,true,false,s)==0);
 auto mid=l.tick(11250,true,false,false,s); assert(mid>0 && mid<64);
 assert(l.tick(11500,true,false,false,s)==64);
 // Lower threshold crossings while lit must not change the output.
 for(unsigned t=12000;t<13000;t+=10) assert(l.tick(t,true,t%20==0,false,s)==64);
 assert(l.tick(13000,true,false,false,s,true)==64);
 auto fading=l.tick(14000,true,true,false,s); assert(fading>0 && fading<64);
 assert(l.tick(15000,true,true,false,s)==0);
 assert(l.tick(15500,true,true,false,s)==0);
 assert(l.tick(16000,true,true,false,s)==0);
 assert(l.tick(16500,true,false,false,s)==64);
 // Absence and missing sensor readings also fade, rather than jumping to zero.
 assert(l.tick(17000,false,false,false,s)==64);
 assert(l.tick(18000,false,false,false,s)==64);
 assert(l.tick(19000,false,false,false,s)==64);
 assert(l.tick(20000,false,false,false,s)>0);
 assert(l.tick(21000,false,false,false,s)==0);
 l.tick(22000,true,true,false,s); assert(l.tick(22500,true,true,false,s)==64);
 assert(l.tick(23000,true,true,false,s,false,false)==64);
 assert(l.tick(24000,true,true,false,s,false,false)>0);
 assert(l.tick(25000,true,true,false,s,false,false)==0);
}
