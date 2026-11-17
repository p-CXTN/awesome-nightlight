#include "../NightlightRevK/RecoveryFeedback.h"
#include <cassert>
int main(){
 assert(!recoveryLedOn(2999,true,true));
 assert(!recoveryLedOn(3000,true,true));
 assert(!recoveryLedOn(3400,true,true));
 assert(!recoveryLedOn(3800,true,true));
 assert(!recoveryLedOn(9999,true,true));
 assert(recoveryLedOn(10000,true,true));
 assert(!recoveryLedOn(10100,true,true));
 assert(recoveryLedOn(10200,true,true));
 assert(!recoveryLedOn(10200,false,true));
 assert(!recoveryLedOn(10200,true,false));
}
