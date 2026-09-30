#ifndef MILES_HOST_UPLOAD106_STATE_H
#define MILES_HOST_UPLOAD106_STATE_H
#include "../backend/reply.h"
#include "../buffer/buffer_upload.h"
#include "../image/upload_policy.h"
#include <memory>
namespace MilesHostUpload106 {
// Private same-module calls. No SDK exports, transport, alternate registry or public ABI.
struct QueryResult { int32_t status; uint32_t value[7]; };
// ABI-private plain aggregate, returned only between functions in the same module.
typedef QueryResult (*NativeQuery)(uint32_t opcode,const void *,uint32_t);
typedef void (*Requirement)(bool, const char *);
class UploadState {
    MilesTransport::ResourceRegistry &registry;
public:
    uint32_t uploadBudgetBytes;
    uint64_t uploadChargedBytes;
    MilesTransport::ResourceRegistry::Reservation uploadReservation;
    std::unique_ptr<MilesHost::BufferUpload> upload;
    std::vector<unsigned char> sealedImage;
    MilesWire::Handle uploadId;
    bool uploadClassified, uploadTerminal, uploadNativePending;
    UploadState(MilesTransport::ResourceRegistry &owner,uint32_t budget)
        : registry(owner),uploadBudgetBytes(budget),uploadChargedBytes(0),uploadId(),
          uploadClassified(false),uploadTerminal(false),uploadNativePending(false) {}
    bool active() const { return !!upload; }
    bool failed() const { return uploadTerminal || uploadNativePending; }
    void observeRefusal(uint32_t status) {
        if(upload && status!=StartupBridge::Success)uploadTerminal=true;
    }
    // Returns false only for an ordinary command while idle; Backend continues its
    // existing dispatch. All upload effects and their terminal guards live here.
    bool intercept(uint32_t opcode,const MilesWire::Call &c,
                   const std::vector<unsigned char> &frame,bool started,bool shutdown,
                   NativeQuery query,Requirement require,StartupBridge::OwnedReply &out) {
        require(!failed(),"uncertain image operation cannot resume");
        if(opcode==MilesWire::BufferBegin || opcode==MilesWire::BufferChunk ||
           opcode==MilesWire::BufferSeal || opcode==MilesWire::BufferRelease ||
           opcode==MilesWire::AIL_file_type || opcode==MilesWire::AIL_WAV_info) {
            try {
                out=executeUpload(opcode,c,frame,started,shutdown,query,require);
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
    StartupBridge::OwnedReply executeUpload(uint32_t opcode, const MilesWire::Call &c,
                                            const std::vector<unsigned char> &frame, bool started, bool shutdown,
                                            NativeQuery queryImage, Requirement require) {
        using namespace StartupBridge;
        OwnedReply out;out.result.transport_status=InvalidFields;
        const bool begin=opcode==MilesWire::BufferBegin;
        const bool chunk=opcode==MilesWire::BufferChunk;
        const bool wav=opcode==MilesWire::AIL_WAV_info;
        const bool classify=opcode==MilesWire::AIL_file_type || wav;
        if(c.callback || c.reserved || c.output_mask!=(wav ? 1u : 0u) || c.text.offset || c.text.length)return out;
        for(unsigned i=(begin || chunk || classify) ? 1u : 0u;i<8;++i)
            if(c.value[i])return out;
        if(chunk) {
            if(!c.bytes.length || c.bytes.length>static_cast<uint32_t>(MilesImage93::ChunkBytes) ||
               c.bytes.offset>frame.size() || c.bytes.length>frame.size()-c.bytes.offset)return out;
        } else if(c.bytes.offset || c.bytes.length)return out;
        if(begin) {
            if(!nullHandle(c.target) || !nullHandle(c.resource))return out;
        } else if(classify) {
            if(!nullHandle(c.target))return out;
        } else if(!nullHandle(c.resource))return out;
        if(!started || shutdown) {out.result.transport_status=LifecycleRefused;return out;}
        if(begin) {
            if(upload || uploadChargedBytes) {out.result.transport_status=LifecycleRefused;return out;}
            const uint32_t size=c.value[0];
            if(!MilesImage93::allows(uploadBudgetBytes,size) ||
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
                uploadNativePending=true;
                const QueryResult queried=queryImage(opcode,sealedImage.data(),c.value[0]);
                const int32_t value=queried.status;
                if(wav && value)for(unsigned n=0;n<7;++n)out.result.value[n]=queried.value[n];
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
