#include "client_eos.h"
#include "../callback-guard/invocation_guard.h"
#include <windows.h>
#include <stdexcept>
#include <cstdlib>
namespace MilesEos {
struct ClientJob::State {
    ClientMiles::HSAMPLE sample;
    ClientMiles::SampleCallback sampleCallback;
    ClientMiles::HSTREAM stream;
    ClientMiles::StreamCallback streamCallback;
    std::shared_ptr<void> lifetime;
    HANDLE done;
    bool succeeded;
    State(ClientMiles::HSAMPLE s,ClientMiles::SampleCallback sc,
          ClientMiles::HSTREAM t,ClientMiles::StreamCallback tc,std::shared_ptr<void> pin)
        :sample(s),sampleCallback(sc),stream(t),streamCallback(tc),lifetime(pin),
         done(CreateEventA(0,TRUE,FALSE,0)),succeeded(false) {
        if(!done)throw std::runtime_error("EOS completion event");
    }
    ~State(){CloseHandle(done);}
};
struct ClientJob::Context {
    std::shared_ptr<ClientJob> job;
    explicit Context(std::shared_ptr<ClientJob> value):job(value){}
};
ClientJob::ClientJob(ClientMiles::HSAMPLE s,ClientMiles::SampleCallback sc,
    ClientMiles::HSTREAM t,ClientMiles::StreamCallback tc,std::shared_ptr<void> pin)
    :state_(new State(s,sc,t,tc,pin)){}
ClientJob::~ClientJob(){delete state_;}
std::shared_ptr<ClientJob> ClientJob::submit(MilesFileExecutor30::EngineFileWorker &worker,
    ClientMiles::HSAMPLE s,ClientMiles::SampleCallback sc,ClientMiles::HSTREAM t,
    ClientMiles::StreamCallback tc,std::shared_ptr<void> pin) {
    if(!pin || !((s && sc && !t && !tc) || (!s && !sc && t && tc)))
        throw std::invalid_argument("one typed EOS callback required");
    std::shared_ptr<ClientJob> job(new ClientJob(s,sc,t,tc,pin));
    Context *context=new Context(job);
    if(!worker.submit(context,execute)) {delete context;throw std::runtime_error("EOS worker submission");}
    return job;
}
void ClientJob::execute(void *opaque) {
    std::unique_ptr<Context> context(static_cast<Context *>(opaque));
    State &state=*context->job->state_;
    try {
        MilesCallbackGuard47::Scope guard;
        if(guard.admitted()) {
            if(state.sampleCallback)state.sampleCallback(state.sample);
            else state.streamCallback(state.stream);
            state.succeeded=!guard.violated();
        }
    }catch(...){state.succeeded=false;}
    if(!SetEvent(state.done))std::abort();
}
ClientJob::Status ClientJob::status() const {
    const DWORD result=WaitForSingleObject(state_->done,0);
    return result==WAIT_TIMEOUT ? Pending :
        result==WAIT_OBJECT_0 && state_->succeeded ? Completed : Failed;
}
}
