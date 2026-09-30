#include "host_file_runtime.h"
#include <cstring>
namespace MilesHostRuntime50 {
bool installAdmitted(Runtime &runtime,MilesTransport::Bytes frame,const MilesWire::Header &header,
                     const MilesHostContext::Origin &origin,bool started,bool shutdown,
                     unsigned char *reply,size_t capacity,size_t &written) {
    using namespace MilesFileProtocol48;
    InstallRequest request;
    if(!reply || capacity<InstallReplyBytes || !decodeInstall(frame,header,request))return false;
    if(origin.sessionIncarnation!=runtime.session() || origin.wireRequest!=header.request ||
       origin.lane!=header.lane || origin.lease!=header.lock_lease || !origin.admissionOrdinal)
        return false;
    unsigned char success[InstallReplyBytes],refusal[InstallReplyBytes];
    size_t successBytes=0,refusalBytes=0;
    // All serialization validation occurs before SDK state can change.
    if(!encodeInstallReply(request,Installed,success,sizeof(success),successBytes) ||
       !encodeInstallReply(request,LifecycleRefused,refusal,sizeof(refusal),refusalBytes))return false;
    bool installed=false;
    if(started && !shutdown && request.registration==runtime.registration()) {
        MilesHostContext::Scope scope(origin);
        if(scope.result()!=MilesHostContext::Entered)return false;
        installed=installSdkCallbacks(runtime);
    }
    const size_t size=installed?successBytes:refusalBytes;
    std::memcpy(reply,installed?success:refusal,size);written=size;
    return true;
}
}
