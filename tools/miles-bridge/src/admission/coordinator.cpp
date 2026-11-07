#include "coordinator.h"
#include <stdexcept>
namespace MilesCoordinator {
namespace {
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
bool valid(const MilesWire::Handle &h) {
    return h.kind>=MilesWire::Driver && h.kind<=MilesWire::Video && h.slot && h.generation;
}
}
Coordinator::Coordinator(Id id,const Limits &limits)
    :incarnation(id),lastAdmission(0),lastRegistration(0),lastLease(0),sessionFilesRegistration(0),bounds(limits),
     current(Active),admissionOrdinal(0),admissionLane(0),admissionLease(0),ownerLane(0),leaseId(0),
     depth(0),admissionAction(Ordinary),callbackCount(0) {
    if(!id || !bounds.maximumId || !bounds.registrations || !bounds.callbacks || !bounds.requestResources)
        throw std::invalid_argument("zero coordinator bound or incarnation");
}
Error Coordinator::admitGame(Id s,Id r,Id lane,Id lease,Action a,const std::vector<MilesWire::Handle> &resources) {
    return admit(s,r,lane,lease,a,resources,false);
}
Error Coordinator::admitCleanup(Id s,Id r,Id lane,Id lease,Action a,const std::vector<MilesWire::Handle> &resources) {
    return admit(s,r,lane,lease,a,resources,true);
}
Error Coordinator::admit(Id s,Id r,Id lane,Id lease,Action a,const std::vector<MilesWire::Handle> &resources,bool cleanup) {
    if(s!=incarnation)return StaleSession;
    if(current==Failed || (current==Draining && (!cleanup || a==AcquireLock)))return WrongState;
    if(admissionOrdinal)return Busy;
    if(!r || !lane || a<Ordinary || a>ReleaseLock)return InvalidIdentity;
    if(lastAdmission==bounds.maximumId)return Capacity;
    if(r!=lastAdmission+1 || r>bounds.maximumId)return InvalidIdentity;
    if(resources.size()>bounds.requestResources)return Capacity;
    if(depth) {
        if(lane!=ownerLane || lease!=leaseId)return WrongLease;
        if(a==AcquireLock && depth==UINT32_MAX)return Capacity;
    } else {
        if(lease || a==ReleaseLock)return WrongLease;
        if(a==AcquireLock && lastLease==bounds.maximumId)return Capacity;
    }
    for(size_t i=0;i<resources.size();++i) {
        if(!valid(resources[i]))return InvalidIdentity;
        for(size_t j=0;j<i;++j)if(same(resources[i],resources[j]))return InvalidIdentity;
        if(!cleanup)for(std::map<Id,Registration>::const_iterator reg=registrations.begin();reg!=registrations.end();++reg)
            if(reg->second.scope==ResourceScoped && reg->second.closing && same(reg->second.resource,resources[i]))return ClosingResource;
    }
    std::vector<MilesWire::Handle> copied(resources); // Allocation before publication.
    pins.swap(copied);admissionOrdinal=r;admissionLane=lane;admissionLease=lease;admissionAction=a;lastAdmission=r;
    return Ok;
}
bool Coordinator::causalPending(Id admissionOrdinal) const {
    for(std::map<Id,Registration>::const_iterator r=registrations.begin();r!=registrations.end();++r)
        for(std::map<Id,Callback>::const_iterator c=r->second.callbacks.begin();c!=r->second.callbacks.end();++c)
            if(c->second.kind==CausalReverseIo && c->second.cause==admissionOrdinal && !c->second.acknowledged)return true;
    return false;
}
Error Coordinator::completeAdmission(Id s,Id r,bool actionApplied) {
    if(s!=incarnation)return StaleSession;
    if(!r || admissionOrdinal!=r)return Unknown;
    if(causalPending(r))return PendingCallback;
    // Even in Failed, an actual completion observation may release request pins.
    // It does not restore a healthy session or turn an uncertain lease into success.
    if(current!=Failed && actionApplied) {
        if(admissionAction==AcquireLock) {
            if(!depth) { leaseId=++lastLease;ownerLane=admissionLane; }
            ++depth;
        } else if(admissionAction==ReleaseLock) {
            --depth;
            if(!depth) { leaseId=0;ownerLane=0; }
        }
    }
    pins.clear();admissionOrdinal=admissionLane=admissionLease=0;return Ok;
}
Error Coordinator::registerCallback(Id s,Id id,const MilesWire::Handle &resource) {
    if(s!=incarnation)return StaleSession;
    if(current!=Active)return WrongState;
    if(!id || !valid(resource))return InvalidIdentity;
    if(lastRegistration==bounds.maximumId || registrations.size()>=bounds.registrations)return Capacity;
    if(id!=lastRegistration+1 || id>bounds.maximumId)return InvalidIdentity;
    for(std::map<Id,Registration>::const_iterator r=registrations.begin();r!=registrations.end();++r)
        if(r->second.scope==ResourceScoped && r->second.closing && same(r->second.resource,resource))return ClosingResource;
    Registration reg;reg.resource=resource;
    registrations.insert(std::make_pair(id,reg));lastRegistration=id;return Ok;
}
Error Coordinator::registerSessionFiles(Id s,Id id) {
    if(s!=incarnation)return StaleSession;
    if(current!=Active)return WrongState;
    if(!id)return InvalidIdentity;
    if(lastRegistration==bounds.maximumId || registrations.size()>=bounds.registrations)return Capacity;
    if(id!=lastRegistration+1 || id>bounds.maximumId)return InvalidIdentity;
    if(sessionFilesRegistration)return Busy;
    Registration reg;reg.scope=SessionFiles;
    registrations.insert(std::make_pair(id,reg));
    lastRegistration=id;sessionFilesRegistration=id;return Ok;
}
Error Coordinator::admitCallback(Id s,Id id,CallbackKind kind,Id cause,CallbackId &out) {
    out.registration=out.sequence=0;
    if(s!=incarnation)return StaleSession;
    if(kind!=CausalReverseIo && kind!=Unsolicited)return InvalidIdentity;
    std::map<Id,Registration>::iterator r=registrations.find(id);
    if(r==registrations.end())return Unknown;
    if(kind==CausalReverseIo) { if(!cause || cause!=admissionOrdinal)return InvalidIdentity; }
    else if(cause)return InvalidIdentity;
    if(callbackCount>=bounds.callbacks || r->second.issued==bounds.maximumId)return Capacity;
    const Id sequence=r->second.issued+1;
    Callback callback={kind,cause,false};
    r->second.callbacks.insert(std::make_pair(sequence,callback));
    r->second.issued=sequence;++callbackCount;
    if(r->second.closing)r->second.closeFrontier=sequence;
    out.registration=id;out.sequence=sequence;
    return Ok; // Records late observations even in Failed; does not execute callback policy.
}
Error Coordinator::acknowledge(Id s,const CallbackId &id) {
    if(s!=incarnation)return StaleSession;
    std::map<Id,Registration>::iterator r=registrations.find(id.registration);
    if(r==registrations.end())return Unknown;
    std::map<Id,Callback>::iterator c=r->second.callbacks.find(id.sequence);
    if(c==r->second.callbacks.end() || c->second.acknowledged)return Unknown;
    c->second.acknowledged=true;
    while(!r->second.callbacks.empty()) {
        c=r->second.callbacks.begin();
        if(!c->second.acknowledged)break;
        r->second.frontier=c->first;r->second.callbacks.erase(c);--callbackCount;
    }
    return Ok;
}
Error Coordinator::beginClose(Id s,Id id) {
    if(s!=incarnation)return StaleSession;
    std::map<Id,Registration>::iterator r=registrations.find(id);
    if(r==registrations.end())return Unknown;
    // Session close requires ordinary admission drained; Failed still records close.
    if(r->second.scope==SessionFiles && current==Active)return WrongState;
    r->second.closing=true;r->second.closeFrontier=r->second.issued;return Ok;
}
Error Coordinator::readiness(Id s,Id id,Readiness &out) const {
    if(s!=incarnation)return StaleSession;
    std::map<Id,Registration>::const_iterator r=registrations.find(id);
    if(r==registrations.end())return Unknown;
    Readiness value={0,0,r->second.closeFrontier,r->second.frontier,r->second.closing,true};
    if(r->second.scope==SessionFiles)
        value.requestPins=admissionOrdinal ? 1u : 0u;
    else
        for(size_t i=0;i<pins.size();++i)if(same(pins[i],r->second.resource))++value.requestPins;
    for(std::map<Id,Callback>::const_iterator c=r->second.callbacks.begin();c!=r->second.callbacks.end();++c)
        if(!c->second.acknowledged)++value.callbackPins;
    out=value;return Ok;
}
Error Coordinator::beginDrain(Id s) {
    if(s!=incarnation)return StaleSession;
    if(current==Failed)return WrongState;
    current=Draining;return Ok;
}
Error Coordinator::fail(Id s) {
    if(s!=incarnation)return StaleSession;
    current=Failed;return Ok;
}
}
