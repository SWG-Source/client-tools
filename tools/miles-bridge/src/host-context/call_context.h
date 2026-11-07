#ifndef MILES_HOST_CALL_CONTEXT41_H
#define MILES_HOST_CALL_CONTEXT41_H
#include <stdint.h>
namespace MilesHostContext {
struct Origin {
    uint64_t sessionIncarnation, wireRequest, lane, lease, admissionOrdinal;
};
enum EnterResult { Entered, InvalidOrigin, NestedEntry, ForeignSessionEntry };
enum SnapshotResult { Ready, Unsolicited, SessionMismatch, InvalidSession };
// Trusted host admission only. Stack lifetime and destruction stay on this thread.
class Scope {
public:
    explicit Scope(const Origin &origin) throw();
    ~Scope() throw();
    EnterResult result() const throw() { return result_; }
private:
    const Origin origin_;
    EnterResult result_;
    Scope(const Scope &);
    Scope &operator=(const Scope &);
    friend SnapshotResult snapshot(uint64_t, Origin &) throw();
};
// Owner supplies session association; output remains unchanged unless Ready.
SnapshotResult snapshot(uint64_t sessionIncarnation, Origin &out) throw();
}
#endif
