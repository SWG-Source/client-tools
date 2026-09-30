#ifndef HOST_FILE_RUNTIME50_H
#define HOST_FILE_RUNTIME50_H
#include "../transport/endpoint.h"
#include "../file-replies/reply_transaction.h"
#include "../host-context/call_context.h"
namespace MilesHostRuntime50 {
__declspec(noreturn) void fatal() throw();
// Dedicated-process lifetime for this initial source composition. Authenticated
// connected callback handle transfers here; no other thread may operate it.
// Destruction is deliberately forbidden until a paired shutdown proof is added.
class Runtime {
public:
    Runtime(HANDLE authenticatedCallbackPipe,uint64_t session,uint64_t registration,
            uint64_t backgroundLane,uint32_t liveFiles);
    ~Runtime();
    uint64_t session() const { return session_; }
    uint64_t registration() const { return registration_; }
    uint32_t invoke(uint32_t opcode,uint32_t token,const char *name,
                    int32_t offset,uint32_t countOrOrigin,void *destination,
                    uint32_t &openedToken);
private:
    struct Ticket {
        std::vector<unsigned char> request,reply;
        MilesHostFiles49::ReplyTransaction *transaction;
        Ticket():transaction(0){}
    };
    HANDLE pipe_,thread_,ready_,work_,reply_,consumed_,done_;
    CRITICAL_SECTION producer_;
    const uint64_t session_,registration_,backgroundLane_;
    uint64_t lastRequest_;
    MilesHostFiles49::FileTokens files_;
    Ticket *ticket_;
    static DWORD WINAPI entry(void *);
    void run();
    Runtime(const Runtime &);
    Runtime &operator=(const Runtime &);
};
// Actual SDK install entry; only command owner after lifecycle/admission checks.
// Dedicated process permits one immutable published Runtime. Never replacement.
bool installSdkCallbacks(Runtime &);
// Command owner supplies independently admitted origin and actual Backend state.
// Decode/preencode before SDK effect; false denotes rejected input, not installation.
bool installAdmitted(Runtime &,MilesTransport::Bytes,const MilesWire::Header &,
                     const MilesHostContext::Origin &,bool started,bool shutdown,
                     unsigned char *reply,size_t capacity,size_t &written);
}
#endif
