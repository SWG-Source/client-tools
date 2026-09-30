#ifndef CLIENT_MILES_PRIVATE_FAILURE52_H
#define CLIENT_MILES_PRIVATE_FAILURE52_H
#include <stdint.h>

namespace ClientMilesPrivate52 {
// Composition-only boundary, absent from the public game header. Bind once before
// any game call and before concurrent access; the reporter remains immutable.
typedef void (*FatalReporter)(uint32_t reason, const char *message);
enum Fault {
    InvalidComposition = 1,
    InvalidArgument = 2,
    PrivateException = 3,
    UnknownException = 4
};
void bindFatalReporter(FatalReporter reporter);
void requireFatalReporter();
#if defined(_MSC_VER)
__declspec(noreturn)
#else
[[noreturn]]
#endif
void fail(Fault reason, const char *message);
}
#endif
