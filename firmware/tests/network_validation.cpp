#include "../NightlightRevK/NetworkValidation.h"
#include <cassert>
using NetworkValidation::staticAddress;
int main() {
  assert(staticAddress(0xc0a80132, 0xc0a80101, 0xffffff00, 0x08080808));
  assert(staticAddress(0x0a010232, 0x0a000001, 0xff000000, 0x0a000001));
  assert(!staticAddress(0xc0a80100, 0xc0a80101, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a801ff, 0xc0a80101, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a80101, 0xc0a80101, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80201, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80101, 0xffff00ff, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80101, 0xffffffff, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80101, 0, 0x08080808));
  assert(!staticAddress(0x7f000002, 0x7f000001, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xe0000002, 0xe0000001, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80101, 0xffffff00, 0));
  assert(!staticAddress(0xc0a80480, 0xc0a80481, 0xffffff00, 0x08080808));
  assert(!staticAddress(0xc0a80482, 0xc0a80481, 0xffffff80, 0x08080808));
  assert(!staticAddress(0xc0a80132, 0xc0a80101, 0xffff0000, 0x08080808));
}
