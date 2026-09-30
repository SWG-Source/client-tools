#include "Session.h"
#include "../../callback-guard/invocation_guard.h"
#include "../../failure/failure_boundary.h"
#include "../../protocol/pair_outputs.h"
#include <list>
#include <cstring>
#include <limits>
#include "../../image/upload_policy.h"

namespace ClientMilesPipe {
struct DriverProxy {
    MilesWire::Handle wire;
    const ClientMilesPipe::Session *owner;
};
struct SampleProxy {
    MilesWire::Handle wire;
    bool live;
    SampleProxy() : wire(), live(false) {}
};
struct StreamProxy {
    MilesWire::Handle wire;
    SampleProxy borrowed;
    bool live;
    StreamProxy() : wire(), live(false) {}
};
} // namespace ClientMilesPipe

namespace ClientMilesPipe {
struct SampleState {
    typedef std::list<std::unique_ptr<ClientMilesPipe::SampleProxy> > Proxies;
    Proxies proxies;
    typedef std::list<std::unique_ptr<ClientMilesPipe::StreamProxy> > Streams;
    Streams streams;
    std::vector<ClientMiles::SampleCallback> sampleCallbacks;
    std::vector<ClientMiles::StreamCallback> streamCallbacks;
};
}

namespace {
using ClientMilesPipe::DriverProxy;
using ClientMilesPipe::SampleProxy;
using ClientMilesPipe::StreamProxy;
// Tokens are opaque object-pointer representations only. Never dereference or
// cast a caller token back to a proxy. Lookup compares freshly generated tokens
// from the existing authoritative rows, then uses only the matched row pointer.
ClientMiles::HDIGDRIVER driverToken(DriverProxy *row) {
    return reinterpret_cast<ClientMiles::HDIGDRIVER>(row);
}
ClientMiles::HSAMPLE sampleToken(SampleProxy *row) {
    return reinterpret_cast<ClientMiles::HSAMPLE>(row);
}
ClientMiles::HSTREAM streamToken(StreamProxy *row) {
    return reinterpret_cast<ClientMiles::HSTREAM>(row);
}
ClientMilesPipe::Session *selectedSession = 0;

void fail(ClientMilesPipeCore57::FailureReason reason, const char *message) {
    throw ClientMilesPipeCore57::Failure(reason, message);
}

void checkStatus(uint32_t status) {
    using ClientMilesPipeCore57::FailureReason;
    switch (status) {
    case StartupBridge::Success:
        return;
    case StartupBridge::Unsupported:
        fail(FailureReason::Unsupported, "unsupported Miles operation");
        break;
    case StartupBridge::InvalidResource:
        fail(FailureReason::InvalidDriver, "invalid Miles driver");
        break;
    case StartupBridge::InvalidFields:
        fail(FailureReason::InvalidArgument, "invalid Miles arguments");
        break;
    case StartupBridge::LifecycleRefused:
        fail(FailureReason::WrongState, "Miles lifecycle refused operation");
        break;
    case StartupBridge::TextTooLong:
        fail(FailureReason::ResultLimit, "Miles result exceeds text limit");
        break;
    case StartupBridge::InputBudgetExceeded:
        fail(FailureReason::InputLimit, "Miles retained input limit");
        break;
    case StartupBridge::VersionQueryFailed:
        fail(FailureReason::QueryFailed, "Miles version query failed");
        break;
    default:
        fail(FailureReason::BackendFailed, "unknown backend result");
    }
}

ClientMilesPipeCore57::OwnedText ownedText(const StartupBridge::OwnedReply &reply) {
    ClientMilesPipeCore57::OwnedText out;
    out.isNull = reply.result.null_mask == MilesStartup::TextNull;
    if (!out.isNull) {
        if (reply.text.empty() || reply.text.back() ||
            std::memchr(&reply.text[0], 0, reply.text.size() - 1))
            fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "invalid owned backend text");
        out.value.assign(reinterpret_cast<const char *>(&reply.text[0]), reply.text.size() - 1);
    } else if (!reply.text.empty()) {
        fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "null backend text contains bytes");
    }
    return out;
}

// Swap only after the owned reply is complete. Separate Session fields preserve
// cross-function pointer stability; null remains distinct from nonnull empty.
const char *replaceSnapshot(std::string &target, ClientMilesPipeCore57::OwnedText &text) {
    target.swap(text.value);
    return text.isNull ? 0 : target.c_str();
}

uint32_t floatBits(float value) {
    static_assert(sizeof(float) == sizeof(uint32_t), "wire F32 width");
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

MilesWire::Call driverCall(ClientMilesPipe::Session &session, ClientMiles::HDIGDRIVER driver) {
    session.requireRunning();
    // Compare the opaque identity before inspecting any pointed-to bytes.
    DriverProxy *const selected = session.driver.get();
    if (!driver || !selected || driver != driverToken(selected) || selected->owner != &session)
        fail(ClientMilesPipeCore57::FailureReason::InvalidDriver, "driver does not belong to selected Miles session");
    MilesWire::Call fields = {};
    fields.target = selected->wire;
    return fields;
}

intptr_t preferenceResult(const StartupBridge::OwnedReply &reply) {
    return static_cast<intptr_t>(MilesStartup::signedValue(reply.result.return_bits));
}
} // namespace

namespace ClientMilesPipe {
Session::Session(Channel *channel, MilesClientRuntime53::Runtime &runtime,
    std::shared_ptr<void> callbackCodeLifetime, uint32_t uploadBudgetBytes)
    : started(false), stopped(false), samples(new SampleState), commandThread_(GetCurrentThreadId()), channel_(channel), closed_(false),
      faulted_(false), runtime_(runtime), callbackCodeLifetime_(callbackCodeLifetime),
      filesPrepared_(false), filesInstalled_(false), uploadBudgetBytes_(uploadBudgetBytes),
      uploadPhase_(UploadIdle), uploadId_(), uploadBytes_(0), uploadOpcode_(0),
      uploadTarget_(), uploadBlock_(0), sourceView_(0) {
    if (!MilesImage93::validBudget(uploadBudgetBytes_))
        fail(ClientMilesPipeCore57::FailureReason::InputLimit,"explicit upload byte budget required");
    if (selectedSession || !channel_)
        fail(ClientMilesPipeCore57::FailureReason::WrongState, "one Miles session must be selected");
    selectedSession = this;
}

Session::~Session() {
    // Destruction never manufactures shutdown or close proof.
    if(!closed_)std::terminate();
}

Session &Session::selected() {
    MilesCallbackGuard47::requireForwardAllowed();
    if (!selectedSession)
        fail(ClientMilesPipeCore57::FailureReason::WrongState, "Miles implementation not selected");
    selectedSession->requireCommandThread();
    return *selectedSession;
}

size_t Session::sampleProxyCount() const { return samples ? samples->proxies.size() : 0; }

void Session::rejectResult() {
    faulted_ = true;runtime_.fail();
    fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "invalid sample reply; outcome uncertain");
}

void Session::requireCommandThread() const {
    if(GetCurrentThreadId()!=commandThread_)
        fail(ClientMilesPipeCore57::FailureReason::WrongState,"Miles command called from another thread");
}

void Session::requireAvailable() const {
    requireCommandThread();
    if (faulted_)
        fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "Miles session has an uncertain outcome");
    if (stopped || closed_)
        fail(ClientMilesPipeCore57::FailureReason::WrongState, "Miles session already stopped");
}

void Session::requireRunning() const {
    requireAvailable();
    if (!started)
        fail(ClientMilesPipeCore57::FailureReason::WrongState, "Miles is not started");
}

StartupBridge::OwnedReply Session::request(uint32_t opcode, const MilesWire::Call &fields,
                                           MilesTransport::Bytes text, MilesTransport::Bytes payload) {
    MilesCallbackGuard47::requireForwardAllowed();
    requireCommandThread();
    if (faulted_ || closed_)
        fail(ClientMilesPipeCore57::FailureReason::BackendFailed,
             "Miles session cannot issue another request");
    StartupBridge::OwnedReply reply;
    try {
        reply = channel_->call(opcode, fields, payload, text, verifiedResources(fields), *this);
    } catch (...) {
        faulted_ = true;runtime_.fail();
        fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "Miles channel failed");
    }
    if (!StartupBridge::knownStatus(reply.result.transport_status)) {
        faulted_ = true;runtime_.fail();
        fail(ClientMilesPipeCore57::FailureReason::BackendFailed, "unknown Miles channel status");
    }
    // Known, validated operation refusals are distinguishable from an uncertain
    // transport outcome. They do not manufacture a successful SDK return value.
    checkStatus(reply.result.transport_status);
    return reply;
}

bool Session::validateReply(uint32_t opcode,const MilesWire::Call &fields,
    const StartupBridge::OwnedReply &reply) const {
    const MilesWire::Result &r=reply.result;
    // Wire decoder has already validated refusal shape/status. A known refusal
    // is still an observed return, and must settle before request() propagates it.
    if(!StartupBridge::knownStatus(r.transport_status))return false;
    const bool setter=opcode==MilesWire::AIL_set_sample_file || opcode==MilesWire::AIL_set_named_sample_file;
    if(setter) {
        if(uploadPhase_!=UploadClassifying || opcode!=uploadOpcode_ ||
           fields.value[0]!=uploadBytes_ || fields.value[1]!=uploadBlock_ ||
           uploadTarget_.kind!=MilesWire::OwnedSample ||
           fields.target.kind!=uploadTarget_.kind || fields.target.slot!=uploadTarget_.slot ||
           fields.target.generation!=uploadTarget_.generation ||
           uploadId_.kind!=MilesWire::Buffer || fields.resource.kind!=uploadId_.kind ||
           fields.resource.slot!=uploadId_.slot || fields.resource.generation!=uploadId_.generation)
            return false;
    }
    if(r.transport_status!=StartupBridge::Success)return true;
    if(opcode==MilesWire::AIL_register_EOS_callback && r.callback>samples->sampleCallbacks.size())return false;
    if(opcode==MilesWire::AIL_register_stream_callback && r.callback &&
       (r.callback<65 || r.callback-64>samples->streamCallbacks.size()))return false;
    if(opcode==MilesWire::SessionVersion)
        return MilesSessionVersion::validCapacity(fields.value[0]) &&
            !reply.bytes.empty() && reply.bytes.size()<=fields.value[0] &&
            reply.bytes.back()==0 && reply.text.empty();
    const auto same=[](const MilesWire::Handle &a,const MilesWire::Handle &b){
        return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
    };
    if(opcode==MilesWire::BufferBegin) {
        if(uploadPhase_!=UploadBeginning || uploadId_.kind ||
           r.resource.kind!=MilesWire::Buffer || !r.resource.slot || !r.resource.generation ||
           fields.value[0]!=uploadBytes_)return false;
    }
    if(opcode==MilesWire::BufferChunk || opcode==MilesWire::BufferSeal ||
       opcode==MilesWire::BufferRelease || opcode==MilesWire::AIL_file_type || opcode==MilesWire::AIL_WAV_info || setter) {
        const bool query=opcode==MilesWire::AIL_file_type || opcode==MilesWire::AIL_WAV_info || setter;
        const MilesWire::Handle &id=query ? fields.resource : fields.target;
        if(!uploadId_.kind || !same(uploadId_,id))return false;
        if((opcode==MilesWire::BufferChunk && uploadPhase_!=UploadChunks) ||
           (opcode==MilesWire::BufferSeal && uploadPhase_!=UploadSealing) ||
           (query && (uploadPhase_!=UploadClassifying || opcode!=uploadOpcode_ || fields.value[0]!=uploadBytes_ ||
               fields.output_mask!=(opcode==MilesWire::AIL_WAV_info ? 1u : 0u))) ||
           (opcode==MilesWire::BufferRelease && uploadPhase_!=UploadReleasing))return false;
    }
    if(opcode==MilesWire::AIL_sample_ms_position || opcode==MilesWire::AIL_stream_ms_position ||
       opcode==MilesWire::AIL_sample_volume_levels || opcode==MilesWire::AIL_sample_reverb_levels){
        const uint32_t mask=fields.output_mask;
        if(!MilesWire::validPairMask(mask) || (!(mask&1u)&&r.value[0]) ||
           (!(mask&2u)&&r.value[1]) || ((mask&4u)&&r.value[0]!=r.value[1]))return false;
    }
    if(opcode==MilesWire::AIL_speaker_configuration &&
       ((fields.output_mask&~8u) || (!(fields.output_mask&8u)&&r.value[3])))return false;
    if(opcode==MilesWire::AIL_open_digital_driver && !StartupBridge::nullHandle(r.resource) && driver)return false;
    if(opcode==MilesWire::AIL_allocate_sample_handle && !StartupBridge::nullHandle(r.resource)){
        for(auto i=samples->proxies.begin();i!=samples->proxies.end();++i)
            if((*i)->live && same((*i)->wire,r.resource))return false;
    }
    if(opcode==MilesWire::AIL_open_stream && !StartupBridge::nullHandle(r.resource)){
        for(auto i=samples->streams.begin();i!=samples->streams.end();++i)
            if((*i)->live && same((*i)->wire,r.resource))return false;
    }
    if(opcode==MilesWire::AIL_stream_sample_handle){
        const MilesWire::Handle echo={r.value[0],r.value[1],r.value[2]};
        if(!same(echo,fields.target))return false;
        const ClientMilesPipe::StreamProxy *parent=0;
        for(auto i=samples->streams.begin();i!=samples->streams.end();++i)
            if((*i)->live && same((*i)->wire,fields.target))parent=i->get();
        if(!parent)return false;
        if(StartupBridge::nullHandle(r.resource))return !parent->borrowed.live;
        if(parent->borrowed.live && !same(parent->borrowed.wire,r.resource))return false;
        for(auto i=samples->streams.begin();i!=samples->streams.end();++i)
            if(i->get()!=parent && (*i)->borrowed.live && same((*i)->borrowed.wire,r.resource))return false;
    }
    return true;
}
std::vector<MilesWire::Handle> Session::verifiedResources(const MilesWire::Call &fields) const {
    std::vector<MilesWire::Handle> out;
    const MilesWire::Handle values[]={fields.target,fields.resource};
    for(unsigned n=0;n<2;++n){
        const MilesWire::Handle &h=values[n];
        if(!h.kind && !h.slot && !h.generation)continue;
        const auto same=[&](const MilesWire::Handle &v){return h.kind==v.kind && h.slot==v.slot && h.generation==v.generation;};
        bool found=(uploadId_.kind && same(uploadId_)) || (driver && same(driver->wire));
        for(auto i=samples->proxies.begin();i!=samples->proxies.end();++i)
            if((*i)->live && same((*i)->wire))found=true;
        for(auto i=samples->streams.begin();i!=samples->streams.end();++i)
            if((*i)->live && (same((*i)->wire) || ((*i)->borrowed.live && same((*i)->borrowed.wire))))found=true;
        if(!found)fail(ClientMilesPipeCore57::FailureReason::InvalidDriver,"unowned forward resource");
        if(out.empty() || !same(out.front()))out.push_back(h);
    }
    return out;
}
ScopedSourceImage::ScopedSourceImage(const void *base,uint32_t bytes) try
    : owner_(Session::selected()),base_(base),bytes_(bytes) {
    owner_.requireRunning();
    if(!base_ || !bytes_)
        fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"positive exact source view required");
    if(owner_.sourceView_ || owner_.uploadPhase_!=Session::UploadIdle)
        fail(ClientMilesPipeCore57::FailureReason::WrongState,"nested or active source view refused");
    ClientMilesPrivate52::requireFatalReporter();
    owner_.sourceView_=this; // publish only after every check; failed nesting preserves outer token
}catch(const std::exception &error){
    ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
}catch(...){
    ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"source extent setup failed");
}
ScopedSourceImage::~ScopedSourceImage() {
    if(owner_.sourceView_!=this)std::terminate();
    owner_.sourceView_=0; // also clear on exception; no RPC and no allocation destruction
}
int32_t Session::classifyImage(const void *image,uint32_t count) {
    const StartupBridge::OwnedReply reply=queryImage(image,count,MilesWire::AIL_file_type);
    return static_cast<int32_t>(MilesStartup::signedValue(reply.result.return_bits));
}
int32_t Session::queryWav(const void *image,ClientMiles::SampleInformation *result) {
    MilesCallbackGuard47::requireForwardAllowed();requireRunning();
    if(!result || !sourceView_ || sourceView_->base_!=image)
        fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"nonnull output and live exact source view required");
    const StartupBridge::OwnedReply reply=queryImage(image,sourceView_->bytes_,MilesWire::AIL_WAV_info);
    const int32_t status=static_cast<int32_t>(MilesStartup::signedValue(reply.result.return_bits));
    if(status) {
        ClientMiles::SampleInformation projected={};
        projected.format=static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[0]));
        projected.bits=static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[1]));
        projected.channels=static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[2]));
        projected.dataLength=reply.result.value[3];projected.rate=reply.result.value[4];
        projected.samples=reply.result.value[5];projected.blockSize=reply.result.value[6];
        *result=projected; // queryImage returned only after validated Release; native0 leaves untouched
    }
    return status;
}
int32_t Session::bindSampleImage(const MilesWire::Handle &sample,const void *image,
    uint32_t count,uint32_t opcode,int32_t block,const char *suffix) {
    MilesCallbackGuard47::requireForwardAllowed();requireRunning();
    if(opcode==MilesWire::AIL_set_sample_file) {
        if(!sourceView_ || sourceView_->base_!=image)
            fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"live exact source view required for sample file");
        count=sourceView_->bytes_;
    } else if(opcode!=MilesWire::AIL_set_named_sample_file) {
        fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"sample image setter required");
    }
    uint32_t textBytes=0;
    if(suffix) {
        if(opcode!=MilesWire::AIL_set_named_sample_file)
            fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"unnamed sample file forbids suffix");
        while(textBytes<MilesStartup::RequestTextLimit && suffix[textBytes])++textBytes;
        if(textBytes==MilesStartup::RequestTextLimit)
            fail(ClientMilesPipeCore57::FailureReason::InputLimit,"sample suffix exceeds request text limit");
        ++textBytes; // Includes NUL; preserves nonnull empty versus null suffix.
    }
    const StartupBridge::OwnedReply reply=queryImage(image,count,opcode,sample,
        static_cast<uint32_t>(block),MilesTransport::Bytes(suffix,textBytes));
    return static_cast<int32_t>(MilesStartup::signedValue(reply.result.return_bits));
}
StartupBridge::OwnedReply Session::queryImage(const void *image,uint32_t count,uint32_t opcode,
    const MilesWire::Handle &target,uint32_t block,MilesTransport::Bytes text) {
    MilesCallbackGuard47::requireForwardAllowed();requireRunning();
    if(uploadPhase_!=UploadIdle)
        fail(ClientMilesPipeCore57::FailureReason::WrongState,"one image transaction required");
    if(!image || !count)
        fail(ClientMilesPipeCore57::FailureReason::InvalidArgument,"nonnull positive image extent required in93");
    if(!MilesImage93::allows(uploadBudgetBytes_,count))
        fail(ClientMilesPipeCore57::FailureReason::InputLimit,"image exceeds private upload budget");
    // Member state survives any uncertainty; no destructor RPC or implicit retry.
    uploadBytes_=count;uploadOpcode_=opcode;uploadTarget_=target;uploadBlock_=block;
    uploadPhase_=UploadBeginning;
    try {
        MilesWire::Call fields={};fields.value[0]=count;
        const StartupBridge::OwnedReply begin=request(MilesWire::BufferBegin,fields);
        uploadId_=begin.result.resource;
        uploadPhase_=UploadChunks;
        uint32_t offset=0;
        while(offset<count) {
            const uint32_t remaining=count-offset;
            const uint32_t chunk=remaining>static_cast<uint32_t>(MilesImage93::ChunkBytes) ?
                static_cast<uint32_t>(MilesImage93::ChunkBytes) : remaining;
            fields=MilesWire::Call();fields.target=uploadId_;fields.value[0]=offset;
            request(MilesWire::BufferChunk,fields,MilesTransport::Bytes(),
                MilesTransport::Bytes(static_cast<const unsigned char *>(image)+offset,chunk));
            offset+=chunk;
        }
        uploadPhase_=UploadSealing;fields=MilesWire::Call();fields.target=uploadId_;
        request(MilesWire::BufferSeal,fields);
        uploadPhase_=UploadClassifying;fields=MilesWire::Call();fields.resource=uploadId_;fields.value[0]=count;
        fields.target=target;fields.value[1]=block;
        fields.output_mask=opcode==MilesWire::AIL_WAV_info ? 1u : 0u;
        const StartupBridge::OwnedReply classified=request(opcode,fields,text);
        uploadPhase_=UploadReleasing;fields=MilesWire::Call();fields.target=uploadId_;
        request(MilesWire::BufferRelease,fields);
        uploadId_=MilesWire::Handle();uploadBytes_=0;uploadOpcode_=0;
        uploadTarget_=MilesWire::Handle();uploadBlock_=0;uploadPhase_=UploadIdle;
        return classified;
    } catch(...) { faulted_=true;runtime_.fail();throw; }
}
void Session::installFiles(ClientMiles::FileOpenCallback open,ClientMiles::FileCloseCallback close,
    ClientMiles::FileSeekCallback seek,ClientMiles::FileReadCallback read){
    MilesCallbackGuard47::requireForwardAllowed();requireRunning();
    if(filesPrepared_ || !open || !close || !seek || !read || !callbackCodeLifetime_)
        fail(ClientMilesPipeCore57::FailureReason::WrongState,"one nonnull retained file table required");
    // Retain once, even on known install refusal: no automatic replacement/retry.
    filesPrepared_=true;
    if(!runtime_.prepare(1,open,close,seek,read,callbackCodeLifetime_)){
        faulted_=true;runtime_.fail();fail(ClientMilesPipeCore57::FailureReason::BackendFailed,"file preparation failed");
    }
    MilesWire::Call fields={};fields.callback=1;
    request(MilesWire::AIL_set_file_callbacks,fields);
    filesInstalled_=true; // only exact validated success after causal settlement
}
void Session::prepareEos(const MilesWire::Handle &resource,uint64_t callback,ClientMiles::HSAMPLE sample,
    ClientMiles::SampleCallback sc,ClientMiles::HSTREAM stream,ClientMiles::StreamCallback tc) {
    requireRunning();
    if(!runtime_.prepareEos(resource,callback,sample,sc,stream,tc,callbackCodeLifetime_))rejectResult();
}
void Session::retireEos(const MilesWire::Handle &resource) {
    if(!runtime_.retireEos(resource))rejectResult();
}
void Session::retireAllEos() {
    if(!runtime_.retireAllEos())rejectResult();
}
void Session::close() {
    MilesCallbackGuard47::requireForwardAllowed();
    requireCommandThread();
    if(closed_ || faulted_ || !stopped || started || sourceView_ || uploadPhase_!=UploadIdle || uploadId_.kind)
        fail(ClientMilesPipeCore57::FailureReason::WrongState,"close requires stopped session without active source or upload");
    try {channel_->finish();}
    catch(...){faulted_=true;runtime_.fail();throw;}
    // finish returned only after genuine SDK shutdown, exact paired close,
    // child exit 0, callback I/O drain, and worker/control-thread joins.
    delete channel_;channel_=0;
    driver.reset();samples.reset();callbackCodeLifetime_.reset();
    lastErrorSnapshot.clear();redistSnapshot.clear();
    closed_=true;selectedSession=0;
}

} // namespace ClientMilesPipe

namespace ClientMilesPipeCore57 {
int32_t WAV_info(const void *image,ClientMiles::SampleInformation *result) {
    return ClientMilesPipe::Session::selected().queryWav(image,result);
}
int32_t file_type(const void *image,uint32_t bytes) {
    return ClientMilesPipe::Session::selected().classifyImage(image,bytes);
}
int32_t startup() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireAvailable();
    if (session.started)
        fail(FailureReason::WrongState, "Miles startup already completed");
    StartupBridge::OwnedReply reply = session.request(MilesWire::AIL_startup, MilesWire::Call());
    const int32_t value = static_cast<int32_t>(MilesStartup::signedValue(reply.result.return_bits));
    session.started = value != 0;
    return value;
}

void shutdown() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    session.request(MilesWire::AIL_shutdown, MilesWire::Call());
    session.retireAllEos();
    session.samples->proxies.clear(); // confirmed vendor shutdown, local handles expire
    session.samples->streams.clear();
    session.started = false;
    session.stopped = true;
}

intptr_t get_preference(uint32_t number) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    if (number != 1 && number != 42)
        fail(FailureReason::InvalidArgument, "unsupported preference identifier");
    MilesWire::Call fields = {};
    fields.value[0] = number;
    return preferenceResult(session.request(MilesWire::AIL_get_preference, fields));
}

intptr_t set_preference(uint32_t number, intptr_t value) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    if (value < (std::numeric_limits<int32_t>::min)() ||
        value > (std::numeric_limits<int32_t>::max)() || number != 42 ||
        (value != 16 && value != 64))
        fail(FailureReason::InvalidArgument, "unsupported preference value");
    MilesWire::Call fields = {};
    fields.value[0] = number;
    fields.value[1] = static_cast<uint32_t>(value);
    return preferenceResult(session.request(MilesWire::AIL_set_preference, fields));
}

const char *last_error() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    OwnedText value = last_errorOwned();
    return replaceSnapshot(session.lastErrorSnapshot, value);
}

const char *set_redist_directory(const char *directory) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    OwnedText value = set_redist_directoryOwned(directory);
    return replaceSnapshot(session.redistSnapshot, value);
}

OwnedText last_errorOwned() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    return ownedText(session.request(MilesWire::AIL_last_error, MilesWire::Call()));
}

OwnedText set_redist_directoryOwned(const char *directory) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireAvailable();
    if (!directory)
        fail(FailureReason::InvalidArgument, "null redistribution directory");
    size_t size = 0;
    while (size < MilesStartup::RequestTextLimit && directory[size])
        ++size;
    if (size == MilesStartup::RequestTextLimit)
        fail(FailureReason::InputLimit, "redistribution directory exceeds limit");
    ++size; // Include the terminator; no scan beyond the accepted wire extent.
    return ownedText(session.request(MilesWire::AIL_set_redist_directory, MilesWire::Call(),
                                     MilesTransport::Bytes(directory, size)));
}

void MSS_version(char *destination, int32_t capacity) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireAvailable();
    if (!destination || capacity <= 0)
        fail(FailureReason::InvalidArgument, "version needs a writable positive extent");
    if (!MilesSessionVersion::validCapacity(static_cast<uint32_t>(capacity)))
        fail(FailureReason::InputLimit, "version capacity exceeds temporary pipe reply budget");
    MilesWire::Call fields = {};
    fields.value[0] = static_cast<uint32_t>(capacity);
    const StartupBridge::OwnedReply reply = session.request(MilesWire::SessionVersion, fields);
    // LiveChannel validated the capacity-specific prefix before the return join.
    std::memcpy(destination, reply.bytes.data(), reply.bytes.size());
}

HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    if (session.driver)
        fail(FailureReason::InvalidArgument, "digital driver already open");
    MilesWire::Call fields = {};
    fields.value[0] = frequency;
    fields.value[1] = static_cast<uint32_t>(bits);
    fields.value[2] = static_cast<uint32_t>(channels);
    fields.value[3] = flags;
    // Obtain caller-visible storage before the side-effecting vendor request.
    std::unique_ptr<DriverProxy> driver(new DriverProxy);
    StartupBridge::OwnedReply reply = session.request(MilesWire::AIL_open_digital_driver, fields);
    if (StartupBridge::nullHandle(reply.result.resource))
        return 0;
    driver->wire = reply.result.resource;
    driver->owner = &session;
    session.driver = std::move(driver);
    return driverToken(session.driver.get());
}

int32_t speaker_configuration_spec(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    // Compare the token, then inspect only the trusted selected private row.
    DriverProxy *const selected = session.driver.get();
    if (!driver || !selected || driver != driverToken(selected) || selected->owner != &session)
        fail(FailureReason::InvalidDriver, "driver does not belong to selected Miles session");
    MilesWire::Call fields = {};
    fields.target = selected->wire;
    fields.output_mask = 8;
    StartupBridge::OwnedReply reply = session.request(MilesWire::AIL_speaker_configuration, fields);
    return static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[3]));
}
// File callback registration is composed below; TLS/public-boundary adoption remains gated.
void set_listener_3D_position(HDIGDRIVER driver, float x, float y, float z) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    fields.value[0] = floatBits(x); fields.value[1] = floatBits(y); fields.value[2] = floatBits(z);
    session.request(MilesWire::AIL_set_listener_3D_position, fields);
}
void set_listener_3D_velocity_vector(HDIGDRIVER driver, float x, float y, float z) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    fields.value[0] = floatBits(x); fields.value[1] = floatBits(y); fields.value[2] = floatBits(z);
    session.request(MilesWire::AIL_set_listener_3D_velocity_vector, fields);
}
void set_listener_3D_orientation(HDIGDRIVER driver, float x, float y, float z,
                                 float upX, float upY, float upZ) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    fields.value[0] = floatBits(x); fields.value[1] = floatBits(y); fields.value[2] = floatBits(z);
    fields.value[3] = floatBits(upX); fields.value[4] = floatBits(upY); fields.value[5] = floatBits(upZ);
    session.request(MilesWire::AIL_set_listener_3D_orientation, fields);
}
void set_3D_rolloff_factor(HDIGDRIVER driver, float factor) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    fields.value[0] = floatBits(factor);
    session.request(MilesWire::AIL_set_3D_rolloff_factor, fields);
}
void lock() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    const MilesWire::Call fields = {};
    session.request(MilesWire::AIL_lock, fields);
}
void unlock() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    const MilesWire::Call fields = {};
    session.request(MilesWire::AIL_unlock, fields);
}
void serve() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    session.request(MilesWire::AIL_serve, MilesWire::Call());
}
int32_t active_sample_count(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    return static_cast<int32_t>(MilesStartup::signedValue(session.request(MilesWire::AIL_active_sample_count, driverCall(session, driver)).result.return_bits));
}
int32_t digital_CPU_percent(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    return static_cast<int32_t>(MilesStartup::signedValue(session.request(MilesWire::AIL_digital_CPU_percent, driverCall(session, driver)).result.return_bits));
}
int32_t digital_latency(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    return static_cast<int32_t>(MilesStartup::signedValue(session.request(MilesWire::AIL_digital_latency, driverCall(session, driver)).result.return_bits));
}
uint32_t get_timer_highest_delay() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    return session.request(MilesWire::AIL_get_timer_highest_delay, MilesWire::Call()).result.return_bits;
}
int32_t file_error() {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    session.requireRunning();
    return static_cast<int32_t>(MilesStartup::signedValue(session.request(MilesWire::AIL_file_error, MilesWire::Call()).result.return_bits));
}
int32_t room_type(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    return static_cast<int32_t>(MilesStartup::signedValue(
        session.request(MilesWire::AIL_room_type, driverCall(session, driver)).result.return_bits));
}
void set_room_type(HDIGDRIVER driver, int32_t room) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    fields.value[0] = static_cast<uint32_t>(room);
    session.request(MilesWire::AIL_set_room_type, fields);
}
// Private helper compares identity without dereferencing caller-provided pointers.
static SampleProxy &ownedSample(ClientMilesPipe::Session &session, HSAMPLE sample) {
    session.requireRunning();
    for (ClientMilesPipe::SampleState::Proxies::iterator i=session.samples->proxies.begin();
         i!=session.samples->proxies.end(); ++i) {
        SampleProxy *candidate = i->get();
        if (sample == sampleToken(candidate) && candidate->live && candidate->wire.kind == MilesWire::OwnedSample) return *candidate;
    }
    fail(FailureReason::InvalidArgument, "sample is not live in this selected Session");
    throw std::logic_error("unreachable");
}

static SampleProxy &controlSample(ClientMilesPipe::Session &session, HSAMPLE sample, bool borrowed) {
    session.requireRunning();
    if (borrowed) {
        for (ClientMilesPipe::SampleState::Streams::iterator i=session.samples->streams.begin();
             i!=session.samples->streams.end(); ++i) {
            StreamProxy &parent=**i;
            if (sample==sampleToken(&parent.borrowed) && parent.live && parent.borrowed.live &&
                parent.borrowed.wire.kind==MilesWire::BorrowedSample) return parent.borrowed;
        }
    }
    return ownedSample(session,sample);
}
static StreamProxy &liveStream(ClientMilesPipe::Session &session,HSTREAM stream) {
    session.requireRunning();
    for (ClientMilesPipe::SampleState::Streams::iterator i=session.samples->streams.begin();
         i!=session.samples->streams.end(); ++i)
        if (stream==streamToken(i->get()) && (*i)->live) return **i;
    fail(FailureReason::InvalidArgument,"stream is not live in selected Session");
    throw std::logic_error("unreachable");
}
ClientMiles::SampleCallback register_EOS_callback(HSAMPLE sample,ClientMiles::SampleCallback callback) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    SampleProxy &proxy=ownedSample(session,sample);
    uint64_t id=0;
    if(callback){
        std::vector<ClientMiles::SampleCallback> &functions=session.samples->sampleCallbacks;
        size_t index=0;for(;index<functions.size();++index)if(functions[index]==callback)break;
        if(index==functions.size()){
            if(index==64)fail(FailureReason::InputLimit,"64 distinct sample callbacks per session");
            functions.push_back(callback);
        }
        id=index+1;
    }
    session.prepareEos(proxy.wire,id,sample,callback,0,0);
    MilesWire::Call fields={};fields.target=proxy.wire;fields.callback=id;
    const uint64_t previous=session.request(MilesWire::AIL_register_EOS_callback,fields).result.callback;
    return previous ? session.samples->sampleCallbacks[static_cast<size_t>(previous-1)] : 0;
}
ClientMiles::StreamCallback register_stream_callback(HSTREAM stream,ClientMiles::StreamCallback callback) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    StreamProxy &proxy=liveStream(session,stream);
    uint64_t id=0;
    if(callback){
        std::vector<ClientMiles::StreamCallback> &functions=session.samples->streamCallbacks;
        size_t index=0;for(;index<functions.size();++index)if(functions[index]==callback)break;
        if(index==functions.size()){
            if(index==64)fail(FailureReason::InputLimit,"64 distinct stream callbacks per session");
            functions.push_back(callback);
        }
        id=index+65;
    }
    session.prepareEos(proxy.wire,id,0,0,stream,callback);
    MilesWire::Call fields={};fields.target=proxy.wire;fields.callback=id;
    const uint64_t previous=session.request(MilesWire::AIL_register_stream_callback,fields).result.callback;
    return previous ? session.samples->streamCallbacks[static_cast<size_t>(previous-65)] : 0;
}
HSTREAM open_stream(HDIGDRIVER driver,const char *filename,int32_t streamMem) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    MilesWire::Call fields=driverCall(session,driver);
    if (!filename) fail(FailureReason::InvalidArgument,"null stream filename");
    const size_t size=std::strlen(filename)+1;
    if (size>MilesWire::MaxFrameBytes-136u) fail(FailureReason::InputLimit,"stream filename exceeds frame capacity");
    fields.value[0]=static_cast<uint32_t>(streamMem);
    std::unique_ptr<StreamProxy> local(new StreamProxy);
    StreamProxy *stream=local.get();
    session.samples->streams.push_back(std::move(local));
    StartupBridge::OwnedReply reply;
    try { reply=session.request(MilesWire::AIL_open_stream,fields,MilesTransport::Bytes(filename,size)); }
    catch (...) { if (!session.uncertain())session.samples->streams.pop_back();throw; }
    if (StartupBridge::nullHandle(reply.result.resource)) {session.samples->streams.pop_back();return 0;}
    stream->wire=reply.result.resource;stream->live=true;return streamToken(stream);
}
static MilesWire::Call streamCall(ClientMilesPipe::Session &session,HSTREAM stream) {
    MilesWire::Call fields={};fields.target=liveStream(session,stream).wire;return fields;
}
void close_stream(HSTREAM stream) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    const MilesWire::Call close=streamCall(session,stream);
    session.request(MilesWire::AIL_close_stream,close);
    session.retireEos(close.target);
    for (ClientMilesPipe::SampleState::Streams::iterator i=session.samples->streams.begin();
         i!=session.samples->streams.end(); ++i)
        if (streamToken(i->get())==stream) {session.samples->streams.erase(i);break;}
}
HSAMPLE stream_sample_handle(HSTREAM stream) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    StreamProxy &parent=liveStream(session,stream);
    const StartupBridge::OwnedReply reply=session.request(MilesWire::AIL_stream_sample_handle,streamCall(session,stream));
    // Exact echo, stable alias and uniqueness were checked before return join.
    if (StartupBridge::nullHandle(reply.result.resource))return 0;
    parent.borrowed.wire=reply.result.resource;parent.borrowed.live=true;return sampleToken(&parent.borrowed);
}
void start_stream(HSTREAM stream) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();s.request(MilesWire::AIL_start_stream,streamCall(s,stream));
}
void set_stream_loop_count(HSTREAM stream,int32_t count) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();MilesWire::Call c=streamCall(s,stream);
    c.value[0]=static_cast<uint32_t>(count);s.request(MilesWire::AIL_set_stream_loop_count,c);
}
void set_stream_loop_block(HSTREAM stream,int32_t first,int32_t last) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();MilesWire::Call c=streamCall(s,stream);
    c.value[0]=static_cast<uint32_t>(first);c.value[1]=static_cast<uint32_t>(last);s.request(MilesWire::AIL_set_stream_loop_block,c);
}
int32_t stream_status(HSTREAM stream) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();
    return static_cast<int32_t>(MilesStartup::signedValue(s.request(MilesWire::AIL_stream_status,streamCall(s,stream)).result.return_bits));
}
void set_stream_ms_position(HSTREAM stream,int32_t value) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();MilesWire::Call c=streamCall(s,stream);
    c.value[0]=static_cast<uint32_t>(value);s.request(MilesWire::AIL_set_stream_ms_position,c);
}
void stream_ms_position(HSTREAM stream,int32_t *total,int32_t *current) {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();MilesWire::Call c=streamCall(s,stream);
    c.output_mask=MilesWire::pairMask(total,current);
    const StartupBridge::OwnedReply reply=s.request(MilesWire::AIL_stream_ms_position,c);
    const int32_t a=static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[0]));
    const int32_t b=static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[1]));
    if(total)*total=a;
    if(current)*current=b;
}

HSAMPLE allocate_sample_handle(HDIGDRIVER driver) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = driverCall(session, driver);
    // Both proxy allocation and container publication precede remote side effects.
    std::unique_ptr<SampleProxy> local(new SampleProxy);
    SampleProxy *sample = local.get();
    session.samples->proxies.push_back(std::move(local));
    StartupBridge::OwnedReply reply;
    try {
        reply = session.request(MilesWire::AIL_allocate_sample_handle, fields);
    } catch (...) {
        // Only an observed refusal permits discarding the unpublished proxy.
        if (!session.uncertain()) session.samples->proxies.pop_back();
        throw;
    }
    if (StartupBridge::nullHandle(reply.result.resource)) {
        session.samples->proxies.pop_back(); // never exposed, so cannot alias a stale handle
        return 0;
    }
    sample->wire = reply.result.resource;
    sample->live = true;
    return sampleToken(sample);
}

void sample_ms_position(HSAMPLE sample, int32_t *total, int32_t *current) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = {};
    fields.target = ownedSample(session, sample).wire;
    fields.output_mask = MilesWire::pairMask(total, current);
    StartupBridge::OwnedReply reply = session.request(MilesWire::AIL_sample_ms_position, fields);
    // Request topology was validated before admission settlement.
    const int32_t a = static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[0]));
    const int32_t b = static_cast<int32_t>(MilesStartup::signedValue(reply.result.value[1]));
    if (total) *total = a;
    if (current) *current = b;
}

void end_sample(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = {};
    fields.target = ownedSample(session, sample).wire;
    session.request(MilesWire::AIL_end_sample, fields);
}

void release_sample_handle(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    SampleProxy &owned = ownedSample(session, sample);
    MilesWire::Call fields = {};
    fields.target = owned.wire;
    session.request(MilesWire::AIL_release_sample_handle, fields);
    session.retireEos(fields.target);
    // Native HSAMPLE lifetime ends here. Caller reuse of that raw pointer is invalid.
    // Wire generations are a separate host/callback obligation, not pointer tokens.
    for (ClientMilesPipe::SampleState::Proxies::iterator i=session.samples->proxies.begin();
         i!=session.samples->proxies.end(); ++i) {
        if (i->get()==&owned) { session.samples->proxies.erase(i); break; }
    }
}

// Client control slice: five operations accept a live parent-owned borrowed proxy.
// Host resolves borrowed sample identity through its live parent stream.
static MilesWire::Call sampleCall(ClientMilesPipe::Session &session, HSAMPLE sample, bool borrowed=false) {
    MilesWire::Call fields = {};
    fields.target = controlSample(session, sample, borrowed).wire;
    return fields;
}
static void sampleFloatPair(uint32_t opcode, HSAMPLE sample, float *first, float *second) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample, opcode==MilesWire::AIL_sample_volume_levels);
    fields.output_mask = MilesWire::pairMask(first, second);
    const StartupBridge::OwnedReply reply = session.request(opcode, fields);
    float a, b;
    std::memcpy(&a, &reply.result.value[0], sizeof a);
    std::memcpy(&b, &reply.result.value[1], sizeof b);
    if (first) *first = a;
    if (second) *second = b;
}
void start_sample(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    session.request(MilesWire::AIL_start_sample, fields);
}
void stop_sample(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    session.request(MilesWire::AIL_stop_sample, fields);
}
uint32_t sample_status(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    return session.request(MilesWire::AIL_sample_status, fields).result.return_bits;
}
void set_sample_loop_count(HSAMPLE sample, int32_t count) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = static_cast<uint32_t>(count);
    session.request(MilesWire::AIL_set_sample_loop_count, fields);
}
void set_sample_loop_block(HSAMPLE sample, int32_t startByte, int32_t endByte) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = static_cast<uint32_t>(startByte);
    fields.value[1] = static_cast<uint32_t>(endByte);
    session.request(MilesWire::AIL_set_sample_loop_block, fields);
}
void set_sample_ms_position(HSAMPLE sample, int32_t milliseconds) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = static_cast<uint32_t>(milliseconds);
    session.request(MilesWire::AIL_set_sample_ms_position, fields);
}
void set_sample_position(HSAMPLE sample, uint32_t byteOffset) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = static_cast<uint32_t>(byteOffset);
    session.request(MilesWire::AIL_set_sample_position, fields);
}
uint32_t sample_position(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    return session.request(MilesWire::AIL_sample_position, fields).result.return_bits;
}
void set_sample_playback_rate(HSAMPLE sample, int32_t rate) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample, true);
    fields.value[0] = static_cast<uint32_t>(rate);
    session.request(MilesWire::AIL_set_sample_playback_rate, fields);
}
int32_t sample_playback_rate(HSAMPLE sample) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample, true);
    return static_cast<int32_t>(MilesStartup::signedValue(session.request(MilesWire::AIL_sample_playback_rate, fields).result.return_bits));
}
void set_sample_volume_levels(HSAMPLE sample, float left, float right) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample, true);
    fields.value[0] = floatBits(left);
    fields.value[1] = floatBits(right);
    session.request(MilesWire::AIL_set_sample_volume_levels, fields);
}
void sample_volume_levels(HSAMPLE sample, float * left, float * right) {
    sampleFloatPair(MilesWire::AIL_sample_volume_levels, sample, left, right);
}
void set_sample_reverb_levels(HSAMPLE sample, float dry, float wet) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample, true);
    fields.value[0] = floatBits(dry);
    fields.value[1] = floatBits(wet);
    session.request(MilesWire::AIL_set_sample_reverb_levels, fields);
}
void sample_reverb_levels(HSAMPLE sample, float * dry, float * wet) {
    sampleFloatPair(MilesWire::AIL_sample_reverb_levels, sample, dry, wet);
}
void set_sample_3D_position(HSAMPLE sample, float x, float y, float z) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = floatBits(x);
    fields.value[1] = floatBits(y);
    fields.value[2] = floatBits(z);
    session.request(MilesWire::AIL_set_sample_3D_position, fields);
}
void set_sample_3D_velocity_vector(HSAMPLE sample, float xPerMs, float yPerMs, float zPerMs) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = floatBits(xPerMs);
    fields.value[1] = floatBits(yPerMs);
    fields.value[2] = floatBits(zPerMs);
    session.request(MilesWire::AIL_set_sample_3D_velocity_vector, fields);
}
void set_sample_3D_distances(HSAMPLE sample, float maximum, float minimum, int32_t autoWetAttenuation) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = floatBits(maximum);
    fields.value[1] = floatBits(minimum);
    fields.value[2] = static_cast<uint32_t>(autoWetAttenuation);
    session.request(MilesWire::AIL_set_sample_3D_distances, fields);
}
void set_sample_occlusion(HSAMPLE sample, float value) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = floatBits(value);
    session.request(MilesWire::AIL_set_sample_occlusion, fields);
}
void set_sample_obstruction(HSAMPLE sample, float value) {
    ClientMilesPipe::Session &session = ClientMilesPipe::Session::selected();
    MilesWire::Call fields = sampleCall(session, sample);
    fields.value[0] = floatBits(value);
    session.request(MilesWire::AIL_set_sample_obstruction, fields);
}

int32_t set_named_sample_file(HSAMPLE sample,const char *suffix,const void *image,
    uint32_t bytes,int32_t block) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    const MilesWire::Handle target=ownedSample(session,sample).wire;
    return session.bindSampleImage(target,image,bytes,MilesWire::AIL_set_named_sample_file,block,suffix);
}
int32_t set_sample_file(HSAMPLE sample,const void *image,int32_t block) {
    ClientMilesPipe::Session &session=ClientMilesPipe::Session::selected();
    const MilesWire::Handle target=ownedSample(session,sample).wire;
    return session.bindSampleImage(target,image,0,MilesWire::AIL_set_sample_file,block,0);
}
} // namespace ClientMiles

namespace ClientMilesPipeCore57 {
void set_file_callbacks(FileOpenCallback open,FileCloseCallback close,FileSeekCallback seek,FileReadCallback read){
    ClientMilesPipe::Session::selected().installFiles(open,close,seek,read);
}
}
