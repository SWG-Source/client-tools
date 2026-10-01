#ifndef MILES_HOST_BINK_SERVICE_H
#define MILES_HOST_BINK_SERVICE_H
#include "bink_owner.h"
#include "../backend/reply.h"
#include "../wire/resource_registry.h"
#include "../host-runtime/host_file_runtime.h"
namespace MilesHostBink {
// Heap-retained with Backend until process exit. No DLL unload or IO rebinding.
class Service {
public:
    explicit Service(MilesTransport::ResourceRegistry &);
    bool intercept(const MilesWire::Header &,const MilesWire::Call &,
                   const std::vector<unsigned char> &,bool started,bool shutdown,
                   MilesHostRuntime50::Runtime *,bool filesInstalled,StartupBridge::OwnedReply &);
    bool permitsMilesShutdown() const;
    bool transferActive() const { return transfer_!=0; }
private:
    MilesTransport::ResourceRegistry &registry_;
    Runtime *runtime_;
    MilesWire::Handle driver_;
    bool attempted_,ready_,stopped_,opening_;
    MilesTransport::ResourceRegistry::Reservation reservation_;
    std::vector<std::unique_ptr<Video> > videos_;
    Video *transfer_;
    MilesWire::Handle transferId_;
    uint32_t offset_;
    Service(const Service &);
    Service &operator=(const Service &);
};
}
#endif
