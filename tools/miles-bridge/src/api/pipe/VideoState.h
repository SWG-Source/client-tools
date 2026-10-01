#ifndef CLIENT_BINK_PIPE_STATE_H
#define CLIENT_BINK_PIPE_STATE_H
#include "../ClientBink.h"
#include "../../backend/reply.h"
#include <list>
#include <memory>
namespace ClientBink {
struct VideoToken { MilesWire::Handle wire; bool live; VideoToken():wire(),live(false){} };
}
namespace ClientMilesPipe {
struct VideoState {
    std::list<std::unique_ptr<ClientBink::VideoToken> > rows;
    bool attempted, started, stopped, copying;
    uint32_t budget, offset, total, format;
    MilesWire::Handle driver, copyingVideo;
    ClientBink::Info copyingInfo;
    std::string error;
    VideoState():attempted(false),started(false),stopped(false),copying(false),
        budget(0),offset(0),total(0),format(0),driver(),copyingVideo(),copyingInfo(){}
    bool owns(const MilesWire::Handle &) const;
    bool anyLive() const;
    bool validate(uint32_t,const MilesWire::Call &,const StartupBridge::OwnedReply &) const;
};
}
#endif
