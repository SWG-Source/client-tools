#include "VideoState.h"
namespace {
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
}
namespace ClientMilesPipe {
bool VideoState::owns(const MilesWire::Handle &h) const {
    for(auto i=rows.begin();i!=rows.end();++i)if((*i)->live && same((*i)->wire,h))return true;
    return false;
}
bool VideoState::anyLive() const {
    for(auto i=rows.begin();i!=rows.end();++i)if((*i)->live)return true;
    return false;
}
bool VideoState::validate(uint32_t op,const MilesWire::Call &c,const StartupBridge::OwnedReply &r) const {
    if(op<MilesWire::BinkInitialize || op>MilesWire::BinkShutdown)return true;
    if(r.result.transport_status!=StartupBridge::Success)return true;
    if(op==MilesWire::BinkOpen) {
        if(!same(c.target,driver))return false;
        if(r.result.resource.kind)
            for(auto i=rows.begin();i!=rows.end();++i)
                if(same((*i)->wire,r.result.resource))return false;
    }
    if(op==MilesWire::BinkPixelsBegin) {
        if(!copying || !same(c.target,copyingVideo) || c.value[0]!=format)return false;
        const MilesWire::Result &v=r.result;
        return v.value[0]==copyingInfo.width && v.value[1]==copyingInfo.height &&
            v.value[2]==total && v.value[3]==copyingInfo.frame && v.value[4]==format;
    }
    if(op==MilesWire::BinkPixelsChunk)
        return copying && same(c.target,copyingVideo) && c.value[0]==offset &&
            c.value[1] && c.value[1]<=total-offset && r.bytes.size()==c.value[1];
    return true;
}
}
