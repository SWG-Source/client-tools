#ifndef CLIENT_MILES_DEVELOPMENT_AUDIO_BOOTSTRAP_H
#define CLIENT_MILES_DEVELOPMENT_AUDIO_BOOTSTRAP_H
#include "../failure/failure_boundary.h"

namespace ClientMilesDevelopment {
// Private development composition, with no STL or Session type at the engine boundary.
// Caller supplies initialized engine Thread/TLS and preserves engine global state
// until shutdownAndCloseAudio returns. Module pins preserve code, not that state.
void connectAudio(const char *hostExecutable, const char *originalDll,
    const char *uploadBudgetBytes, ClientMilesPrivate52::FatalReporter reporter);
// Only after a successful startup. Failure is terminal and retains the roots.
void shutdownAndCloseAudio();
}
#endif
