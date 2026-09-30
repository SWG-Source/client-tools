// SDK remains an external private build input; no SDK header copied into snapshot.
#include "Mss.h"
#include "host_file_runtime.h"
#include <type_traits>
#include <cstring>
#ifdef _WIN64
#error original Miles helper callback ABI is Win32 only
#endif
namespace MilesHostRuntime50 {
namespace {
Runtime *volatile published=0;
Runtime &runtime() {
    Runtime *p=static_cast<Runtime *>(InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID volatile *>(&published),0,0));
    if(!p)fatal();return *p;
}
U32 AILCALLBACK openFile(MSS_FILE const *name,UINTa *out) {
    try {
        if(!out)fatal();
        uint32_t token=0;
        uint32_t status=runtime().invoke(MilesWire::FileOpen,0,name,0,0,0,token);
        // Ordinary failed open leaves SDK output untouched; status is not bool.
        if(status)*out=token;
        return status;
    } catch(...){fatal();}
}
void AILCALLBACK closeFile(UINTa file) {
    try {uint32_t unused=0;runtime().invoke(MilesWire::FileClose,file,0,0,0,0,unused);}
    catch(...){fatal();}
}
S32 AILCALLBACK seekFile(UINTa file,S32 offset,U32 origin) {
    try {
        uint32_t unused=0,bits=runtime().invoke(MilesWire::FileSeek,file,0,offset,origin,0,unused);
        S32 result;std::memcpy(&result,&bits,sizeof(result));return result;
    }catch(...){fatal();}
}
U32 AILCALLBACK readFile(UINTa file,void *buffer,U32 count) {
    try {uint32_t unused=0;return runtime().invoke(MilesWire::FileRead,file,0,0,count,buffer,unused);}
    catch(...){fatal();}
}
static_assert(sizeof(UINTa)==4,"opaque SDK token is Win32");
static_assert(std::is_same<decltype(&openFile),AIL_file_open_callback>::value,"open ABI");
static_assert(std::is_same<decltype(&closeFile),AIL_file_close_callback>::value,"close ABI");
static_assert(std::is_same<decltype(&seekFile),AIL_file_seek_callback>::value,"seek ABI");
static_assert(std::is_same<decltype(&readFile),AIL_file_read_callback>::value,"read ABI");
}
bool installSdkCallbacks(Runtime &binding) {
    // Publication before SDK entry permits callbacks during installation. Any
    // ambiguous failure remains process terminal; never withdraw this binding.
    if(InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile *>(&published),
                                        &binding,0)!=0)return false;
    // /EHsc treats this extern-C SDK call as nonthrowing; callback thunks retain their own catches.
    ::AIL_set_file_callbacks(&openFile,&closeFile,&seekFile,&readFile);
    return true;
}
}
