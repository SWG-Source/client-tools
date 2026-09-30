#ifndef HOST_FILE_TOKENS49_H
#define HOST_FILE_TOKENS49_H
#include "../wire/resource_registry.h"
#include <memory>
namespace MilesHostFiles49 {
// Private x86 SDK token adapter over the existing resource registry. Serialized
// by the host's callback producer slot. No vendor/client handles are pointers here.
class FileTokens {
public:
    explicit FileTokens(uint32_t liveCapacity);
    ~FileTokens();
    // May allocate registry storage. Must succeed before sending FileOpen.
    bool reserveOpen(uint32_t &opaqueToken);
    // Only for a request never sent, or a validated ordinary failed-open result.
    bool cancelOpen(uint32_t opaqueToken) throw();
    // No allocation. remoteFile is the CLIENT's wire identity, not registry identity.
    bool publishOpen(uint32_t opaqueToken,const MilesWire::Handle &remoteFile) throw();
    bool resolve(uint32_t opaqueToken,MilesWire::Handle &remoteFile) const throw();
    bool beginClose(uint32_t opaqueToken) throw();
    bool finishClose(uint32_t opaqueToken) throw();
private:
    enum State { Free, Reserved, Live, Closing };
    struct Record {
        State state;
        uint32_t token;
        MilesWire::Handle hostIdentity, remoteFile;
        MilesTransport::ResourceRegistry::Reservation reservation;
        Record():state(Free),token(0),hostIdentity(),remoteFile() {}
    private:
        Record(const Record &);
        Record &operator=(const Record &);
    };
    // Registry must outlive record Reservations during destruction.
    MilesTransport::ResourceRegistry registry;
    std::unique_ptr<Record[]> records;
    const uint32_t capacity;
    uint32_t lastToken;
    Record *find(uint32_t) throw();
    const Record *find(uint32_t) const throw();
    FileTokens(const FileTokens &);
    FileTokens &operator=(const FileTokens &);
};
}
#endif
