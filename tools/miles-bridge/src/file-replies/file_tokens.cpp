#include "file_tokens.h"
namespace MilesHostFiles49 {
namespace {
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
}
FileTokens::FileTokens(uint32_t count):registry(count),records(new Record[count]),capacity(count),lastToken(0) {}
FileTokens::~FileTokens() {}
FileTokens::Record *FileTokens::find(uint32_t token) throw() {
    if(!token)return 0;
    for(uint32_t i=0;i<capacity;++i)if(records[i].state!=Free && records[i].token==token)return &records[i];
    return 0;
}
const FileTokens::Record *FileTokens::find(uint32_t token) const throw() {
    if(!token)return 0;
    for(uint32_t i=0;i<capacity;++i)if(records[i].state!=Free && records[i].token==token)return &records[i];
    return 0;
}
bool FileTokens::reserveOpen(uint32_t &token) {
    if(lastToken==UINT32_MAX)return false; // Numeric exhaustion before another open is sent; never wrap/reuse.
    Record *record=0;
    for(uint32_t i=0;i<capacity;++i)if(records[i].state==Free){record=&records[i];break;}
    if(!record || !registry.reserve(MilesWire::File,MilesWire::Handle(),record->reservation))return false;
    record->state=Reserved;record->token=++lastToken;record->hostIdentity=MilesWire::Handle();
    record->remoteFile=MilesWire::Handle();token=record->token;return true;
}
bool FileTokens::cancelOpen(uint32_t token) throw() {
    Record *r=find(token);if(!r || r->state!=Reserved)return false;
    registry.cancel(r->reservation);r->state=Free;r->token=0;return true;
}
bool FileTokens::publishOpen(uint32_t token,const MilesWire::Handle &remote) throw() {
    Record *r=find(token);
    if(!r || r->state!=Reserved || remote.kind!=MilesWire::File || !remote.slot || !remote.generation)return false;
    for(uint32_t i=0;i<capacity;++i)
        if((records[i].state==Live || records[i].state==Closing) && same(records[i].remoteFile,remote))return false;
    MilesWire::Handle host={};
    if(!registry.publish(r->reservation,r,host))return false;
    r->hostIdentity=host;r->remoteFile=remote;r->state=Live;return true;
}
bool FileTokens::resolve(uint32_t token,MilesWire::Handle &remote) const throw() {
    const Record *r=find(token);void *raw=0;
    if(!r || r->state!=Live || !registry.resolve(r->hostIdentity,MilesWire::File,raw) || raw!=r)return false;
    remote=r->remoteFile;return true;
}
bool FileTokens::beginClose(uint32_t token) throw() {
    Record *r=find(token);
    if(!r || r->state!=Live || !registry.beginClose(r->hostIdentity))return false;
    r->state=Closing;return true;
}
bool FileTokens::finishClose(uint32_t token) throw() {
    Record *r=find(token);
    if(!r || r->state!=Closing || !registry.retire(r->hostIdentity))return false;
    r->state=Free;r->token=0;r->hostIdentity=MilesWire::Handle();r->remoteFile=MilesWire::Handle();return true;
}
}
