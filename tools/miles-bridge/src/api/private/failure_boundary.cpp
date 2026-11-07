#include "failure_boundary.h"
#include <cstdlib>

namespace ClientMilesPrivate52 {
namespace {
// No standard-library object construction before the guarded public calls.
FatalReporter reporter = 0;
}
void bindFatalReporter(FatalReporter value) {
    if (!value || reporter)
        fail(InvalidComposition, "Miles fatal reporter must be bound exactly once before use");
    reporter = value;
}
void requireFatalReporter() {
    if (!reporter)
        fail(InvalidComposition, "Miles fatal reporter was not bound before a game call");
}
void fail(Fault reason, const char *message) {
    // The engine consumes this borrowed diagnostic synchronously. A returning or
    // throwing reporter cannot turn this fault into a normal Miles return value.
    try {
        if (reporter)
            reporter(static_cast<uint32_t>(reason), message);
    } catch (...) {
        // No engine/private exception is permitted to escape the failure path.
    }
    std::abort();
}
}
