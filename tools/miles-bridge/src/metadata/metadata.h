#ifndef EXPERIMENTAL_MILES_STARTUP_METADATA_H
#define EXPERIMENTAL_MILES_STARTUP_METADATA_H
#include "../dispatch/host_dispatch.h"
#include "../wire/codec.h"
#include <vector>
#include <list>
namespace MilesStartup {
enum { TextNull = 1, RequestTextLimit = MilesWire::MaxFrameBytes - 48 - 88, ReplyTextLimit = MilesWire::MaxFrameBytes - 48 - 80 };
enum Status { Complete=0, Unsupported=1, InvalidResource=2, InvalidFields=3, TextTooLong=4, InputBudgetExceeded=5 };
// Must outlive vendor shutdown. No early retirement is provided.
class SessionInputs {
    std::list<std::vector<unsigned char> > paths;
    size_t used;
    SessionInputs(const SessionInputs&);
    SessionInputs& operator=(const SessionInputs&);
public:
    SessionInputs():used(0){}
    // Precondition: Bytes refers to a valid readable allocation; this is not a pointer probe.
    const char* retain(MilesTransport::Bytes);
    size_t retainedBytes() const { return used; }
};
struct Reply {
    MilesWire::Result result;
    std::vector<unsigned char> text;
    Reply();
};
bool preferenceValue(int64_t clientValue, uint32_t& wireBits);
int64_t signedValue(uint32_t wireBits);
Status validate(uint32_t opcode, const MilesWire::Call&, MilesTransport::Bytes frame);
// The pointer must be null or a valid vendor C string. This is not a memory probe.
// On failure, destination is unchanged. Null and nonnull empty are distinct.
bool copyText(const char* vendorText, Reply& destination);
// Caller owns session/lane admission. This component supplies neither startup nor a shim.
Status dispatch(uint32_t opcode, const MilesWire::Call&, MilesTransport::Bytes frame,
                Reply&, MilesHost::Resolver&, SessionInputs&);
}
#endif
