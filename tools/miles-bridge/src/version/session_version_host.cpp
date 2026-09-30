#include "session_version_host.h"
#if !defined(_WIN32) || defined(_WIN64)
#error Private version90 helper requires the selected original x86 Windows host
#endif
#include <windows.h>
#include <Mss.h>
namespace MilesSessionVersion {
namespace {
struct ModulePin {
    HMODULE value;
    explicit ModulePin(HMODULE module):value(module) {}
    ~ModulePin() { if(value) FreeLibrary(value); }
  private:
    ModulePin(const ModulePin &);
    ModulePin &operator=(const ModulePin &);
};
}
bool queryNativeVersion(MilesTransport::Bytes query, std::vector<unsigned char> &frame) {
    MilesWire::Header request={}; uint32_t capacity=0;
    if(!validateQuery(query,request,capacity)) return false;
    // Initialized owned extent before the API; no caller stack bytes cross the pipe.
    std::vector<unsigned char> scratch(capacity,0xa5);
    HMODULE loaded=LoadLibraryA(MSSDLLNAME);
    size_t written=1;
    if(reinterpret_cast<UINT_PTR>(loaded)<=32u) scratch[0]=0;
    else {
        ModulePin pin(loaded);
        const int count=LoadStringA(loaded,1,reinterpret_cast<char *>(scratch.data()),
                                    static_cast<int>(capacity));
        // Count0 plus an actual NUL is ordinary empty/truncated/missing output.
        // This write form is observed on probe84's Windows/ACP1252 environment,
        // not asserted as a universal failure-write contract for every platform.
        if(count<0 || static_cast<uint32_t>(count)>=capacity || scratch[static_cast<size_t>(count)]!=0)
            return false;
        written=static_cast<size_t>(count)+1;
    }
    return makeReply(request,MilesTransport::Bytes(scratch.data(),written),frame);
}
}
