#ifndef MILES_HOST_EOS_H
#define MILES_HOST_EOS_H
#include "../protocol/miles_wire.h"
namespace MilesHostRuntime50 { class Runtime; }
namespace MilesHostEos {
void initialize(MilesHostRuntime50::Runtime &);
// Caller validates kind/token before entry. All allocation precedes SDK effect.
uint64_t registerCallback(void *,const MilesWire::Handle &,uint64_t token);
void released(void *,const MilesWire::Handle &);
void shutdownComplete();
}
#endif
