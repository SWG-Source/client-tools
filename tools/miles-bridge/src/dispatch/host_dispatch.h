#ifndef EXPERIMENTAL_MILES_HOST_DISPATCH_H
#define EXPERIMENTAL_MILES_HOST_DISPATCH_H
#include "../protocol/miles_wire.h"
namespace MilesHost {
enum DispatchStatus { Complete=0, Unsupported=1, InvalidResource=2, InvalidFields=3 };
// Implemented by transport registry adapter; must validate kind, slot, generation,
// live state, and borrowed sample parent lifetime. Never dereference wire tokens.
struct Resolver {
 virtual ~Resolver() {}
 virtual bool resolve(MilesWire::Handle const &,uint32_t allowedKindMask,uintptr_t &native)=0;
};
bool supports(uint32_t opcode);
// Precondition: session initialized, lane+lock lease admission established.
// No lifecycle/admission implementation and no worker-thread policy here.
DispatchStatus dispatch(uint32_t opcode,MilesWire::Call const &,MilesWire::Result &,Resolver &);
}
#endif
