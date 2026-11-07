#ifndef MILES_HOST_BINK_FILE_IO_H
#define MILES_HOST_BINK_FILE_IO_H
#include <bink.h>
namespace MilesHostRuntime50 { class Runtime; }
namespace MilesHostBink {
typedef U32 (RADLINK *TimerRead)();
// One immutable process-owned binding, published before BinkSetIO/BinkOpen.
// Keep runtime and genuine timer DLL alive through all Bink close/background
// producer completion; only then may the reverse file runtime be stopped.
BINKIOOPEN bindFileIo(MilesHostRuntime50::Runtime &, TimerRead);
}
#endif
