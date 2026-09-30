#include "call_context.h"
namespace MilesHostContext {
namespace {
#if defined(_MSC_VER)
__declspec(thread) const Scope *current = 0;
#else
thread_local const Scope *current = 0;
#endif
}
Scope::Scope(const Origin &origin) throw() : origin_(origin), result_(InvalidOrigin) {
    if (!origin_.sessionIncarnation || !origin_.wireRequest || !origin_.lane || !origin_.admissionOrdinal)
        return;
    if (current) {
        result_ = current->origin_.sessionIncarnation == origin_.sessionIncarnation ? NestedEntry : ForeignSessionEntry;
        return;
    }
    current = this;
    result_ = Entered;
}
Scope::~Scope() throw() {
    if (result_ == Entered && current == this) current = 0;
}
SnapshotResult snapshot(uint64_t sessionIncarnation, Origin &out) throw() {
    if (!sessionIncarnation) return InvalidSession;
    if (!current) return Unsolicited;
    if (current->origin_.sessionIncarnation != sessionIncarnation) return SessionMismatch;
    out = current->origin_;
    return Ready;
}
}
