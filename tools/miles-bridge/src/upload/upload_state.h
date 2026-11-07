#ifndef MILES_HOST_UPLOAD106_STATE_H
#define MILES_HOST_UPLOAD106_STATE_H
#include "../backend/reply.h"
#include "../buffer/buffer_upload.h"
#include "../image/upload_policy.h"
#include <memory>
#include <list>
namespace MilesHostUpload106 {
// Private same-module calls. No SDK exports, transport, alternate registry or public ABI.
struct QueryResult { int32_t status; uint32_t value[7]; };
// ABI-private plain aggregate, returned only between functions in the same module.
typedef QueryResult (*NativeQuery)(uint32_t opcode,const void *,uint32_t);
typedef int32_t (*NativeBind)(uint32_t,void *,const char *,const void *,uint32_t,int32_t);
typedef void (*Requirement)(bool, const char *);
class UploadState {
    MilesTransport::ResourceRegistry &registry;
    struct Binding {
        MilesWire::Handle sample;
        std::vector<unsigned char> image, suffix;
        explicit Binding(const MilesWire::Handle &id) : sample(id) {}
        uint64_t bytes() const { return static_cast<uint64_t>(image.size())+suffix.size(); }
    };
    // Stable rows: neither vector storage moves while the SDK can retain it.
    std::list<Binding> bindings;
    uint64_t retainedBytes_;
public:
    uint32_t uploadBudgetBytes;
    uint64_t uploadChargedBytes;
    MilesTransport::ResourceRegistry::Reservation uploadReservation;
    std::unique_ptr<MilesHost::BufferUpload> upload;
    std::vector<unsigned char> sealedImage;
    MilesWire::Handle uploadId;
    bool uploadClassified, uploadTerminal, uploadNativePending;
    UploadState(MilesTransport::ResourceRegistry &owner,uint32_t budget)
        : registry(owner),retainedBytes_(0),uploadBudgetBytes(budget),uploadChargedBytes(0),uploadId(),
          uploadClassified(false),uploadTerminal(false),uploadNativePending(false) {}
    bool active() const { return !!upload; }
    bool failed() const { return uploadTerminal || uploadNativePending; }
    uint64_t retainedBytes() const { return retainedBytes_; }
    // Only called after genuine native release/shutdown has returned.
    void releasedSample(const MilesWire::Handle &sample) { discardBindings(sample,0); }
    void shutdownComplete() { bindings.clear();retainedBytes_=0; }
    void observeRefusal(uint32_t status) {
        if(upload && status!=StartupBridge::Success)uploadTerminal=true;
    }
    // Returns false only for an ordinary command while idle; Backend continues its
    // existing dispatch. All upload effects and their terminal guards live here.
    bool intercept(uint32_t opcode,const MilesWire::Call &c,
                   const std::vector<unsigned char> &frame,bool started,bool shutdown,
                   NativeQuery query,Requirement require,StartupBridge::OwnedReply &out,
                   NativeBind bind=0) {
        require(!failed(),"uncertain image operation cannot resume");
        if(opcode==MilesWire::BufferBegin || opcode==MilesWire::BufferChunk ||
           opcode==MilesWire::BufferSeal || opcode==MilesWire::BufferRelease ||
           opcode==MilesWire::AIL_file_type || opcode==MilesWire::AIL_WAV_info ||
           opcode==MilesWire::AIL_set_sample_file || opcode==MilesWire::AIL_set_named_sample_file) {
            try {
                out=executeUpload(opcode,c,frame,started,shutdown,query,bind,require);
                observeRefusal(out.result.transport_status);
                return true;
            } catch(...) {uploadTerminal=true;throw;}
        }
        if(upload) {
            uploadTerminal=true;
            out=StartupBridge::OwnedReply();out.result.transport_status=StartupBridge::LifecycleRefused;
            return true;
        }
        return false;
    }
private:
    UploadState(const UploadState &);
    UploadState &operator=(const UploadState &);
    void discardBindings(const MilesWire::Handle &sample,const Binding *keep) {
        for(std::list<Binding>::iterator i=bindings.begin();i!=bindings.end();) {
            if(&*i!=keep && i->sample.kind==sample.kind && i->sample.slot==sample.slot &&
               i->sample.generation==sample.generation) {
                retainedBytes_-=i->bytes();i=bindings.erase(i);
            } else ++i;
        }
    }
    StartupBridge::OwnedReply executeUpload(uint32_t opcode, const MilesWire::Call &c,
                                            const std::vector<unsigned char> &frame, bool started, bool shutdown,
                                            NativeQuery queryImage,NativeBind bind, Requirement require) {
        using namespace StartupBridge;
        OwnedReply out;out.result.transport_status=InvalidFields;
        const bool begin=opcode==MilesWire::BufferBegin;
        const bool chunk=opcode==MilesWire::BufferChunk;
        const bool wav=opcode==MilesWire::AIL_WAV_info;
        const bool named=opcode==MilesWire::AIL_set_named_sample_file;
        const bool binding=named || opcode==MilesWire::AIL_set_sample_file;
        const bool classify=opcode==MilesWire::AIL_file_type || wav || binding;
        if(c.callback || c.reserved || c.output_mask!=(wav ? 1u : 0u))return out;
        if(named && c.text.length) {
            if(c.text.length>MilesStartup::RequestTextLimit || c.text.offset>frame.size() ||
               c.text.length>frame.size()-c.text.offset)return out;
            const unsigned char *text=frame.data()+c.text.offset;
            if(text[c.text.length-1] || std::memchr(text,0,c.text.length-1))return out;
        } else if(c.text.offset || c.text.length)return out;
        for(unsigned i=binding ? 2u : (begin || chunk || classify) ? 1u : 0u;i<8;++i)
            if(c.value[i])return out;
        if(chunk) {
            if(!c.bytes.length || c.bytes.length>static_cast<uint32_t>(MilesImage93::ChunkBytes) ||
               c.bytes.offset>frame.size() || c.bytes.length>frame.size()-c.bytes.offset)return out;
        } else if(c.bytes.offset || c.bytes.length)return out;
        if(begin) {
            if(!nullHandle(c.target) || !nullHandle(c.resource))return out;
        } else if(classify) {
            if(!binding && !nullHandle(c.target))return out;
        } else if(!nullHandle(c.resource))return out;
        if(!started || shutdown) {out.result.transport_status=LifecycleRefused;return out;}
        if(begin) {
            if(upload || uploadChargedBytes) {out.result.transport_status=LifecycleRefused;return out;}
            const uint32_t size=c.value[0];
            if(!MilesImage93::allows(uploadBudgetBytes,size) ||
               retainedBytes_+static_cast<uint64_t>(size)*2>uploadBudgetBytes ||
               static_cast<uint64_t>(size)>static_cast<uint64_t>(sealedImage.max_size())) {
                out.result.transport_status=InputBudgetExceeded;return out;
            }
            // Existing registry reservation and budget charge precede allocation;
            // Backend is heap-retained if allocation/publication/reply becomes uncertain.
            require(registry.reserve(MilesWire::Buffer,MilesWire::Handle(),uploadReservation),
                    "image registry capacity before allocation");
            uploadChargedBytes=static_cast<uint64_t>(size)*2;
            upload.reset(new MilesHost::BufferUpload(size,uploadBudgetBytes/2));
            require(registry.publish(uploadReservation,upload.get(),uploadId),"publish image owner");
            out.result.resource=uploadId;
        } else {
            const MilesWire::Handle &id=classify ? c.resource : c.target;
            void *local=0;
            if(!upload || id.kind!=uploadId.kind || id.slot!=uploadId.slot ||
               id.generation!=uploadId.generation || !registry.resolve(id,MilesWire::Buffer,local) ||
               local!=upload.get()) {out.result.transport_status=InvalidResource;return out;}
            if(chunk) {
                if(!upload->append(c.value[0],frame.data()+c.bytes.offset,c.bytes.length))return out;
            } else if(opcode==MilesWire::BufferSeal) {
                if(upload->sealed() || !upload->seal())return out;
                require(upload->copySealed(sealedImage),"independent sealed image ownership");
            } else if(classify) {
                if(!upload->sealed() || uploadClassified || c.value[0]!=upload->size() ||
                   sealedImage.size()!=upload->size())return out;
                int32_t value=0;
                if(binding) {
                    void *sample=0;
                    if(!registry.resolve(c.target,MilesWire::OwnedSample,sample)) {
                        out.result.transport_status=InvalidResource;return out;
                    }
                    if(retainedBytes_+uploadChargedBytes+c.text.length>uploadBudgetBytes) {
                        out.result.transport_status=InputBudgetExceeded;return out;
                    }
                    require(bind!=0,"native sample binding required");
                    bindings.emplace_back(c.target);
                    Binding &held=bindings.back();
                    if(c.text.length)held.suffix.assign(frame.begin()+c.text.offset,
                                                       frame.begin()+c.text.offset+c.text.length);
                    // All ownership/allocation work precedes the native effect.
                    held.image.swap(sealedImage);
                    retainedBytes_+=held.bytes();uploadChargedBytes-=held.image.size();
                    int32_t block=0;std::memcpy(&block,&c.value[1],sizeof(block));
                    uploadNativePending=true;
                    value=bind(opcode,sample,held.suffix.empty() ? 0 :
                        reinterpret_cast<const char *>(held.suffix.data()),held.image.data(),c.value[0],block);
                    // The supported Miles setters initialize the sample before
                    // a successful replacement. Zero is not a rollback promise:
                    // keep old and attempted inputs until success or release.
                    if(value)discardBindings(c.target,&held);
                } else {
                    uploadNativePending=true;
                    const QueryResult queried=queryImage(opcode,sealedImage.data(),c.value[0]);
                    value=queried.status;
                    if(wav && value)for(unsigned n=0;n<7;++n)out.result.value[n]=queried.value[n];
                }
                // Native failure/unknown classification remains a native S32 result.
                static_assert(sizeof(value)==sizeof(out.result.return_bits),"file type result width");
                std::memcpy(&out.result.return_bits,&value,sizeof(value));
                uploadClassified=true;uploadNativePending=false;
            } else if(opcode==MilesWire::BufferRelease) {
                // Also usable for a healthy partial upload. Never used as client
                // recovery after refusal/uncertainty, which is terminal in93.
                require(registry.beginClose(id),"close image identity");
                require(registry.retire(id),"retire readonly image identity");
                upload.reset();std::vector<unsigned char>().swap(sealedImage);
                uploadId=MilesWire::Handle();uploadChargedBytes=0;uploadClassified=false;
            } else return out;
        }
        out.result.transport_status=Success;return out;
    }
};
}
#endif
