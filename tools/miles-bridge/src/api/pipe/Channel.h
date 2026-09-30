#ifndef CLIENT_MILES_PIPE_CHANNEL_H
#define CLIENT_MILES_PIPE_CHANNEL_H

#include "../../backend/reply.h"

namespace ClientMilesPipe {
class Session;
// Private adapter seam. Production calls use the existing framed client;
// scripted implementations live only in tests. No game-facing header includes it.
class Channel {
  public:
    virtual ~Channel() {
    }
    virtual StartupBridge::OwnedReply call(uint32_t opcode, const MilesWire::Call &fields,
                                           MilesTransport::Bytes payload, MilesTransport::Bytes text,
                                           const std::vector<MilesWire::Handle> &verifiedResources,
                                           const Session &replyOwner) = 0;
    // Candidate64 refuses normal finish until paired shutdown proof exists.
    virtual void finish() = 0;
};
} // namespace ClientMilesPipe
#endif
