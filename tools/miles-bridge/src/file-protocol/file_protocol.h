#ifndef PRIVATE_FILE_PROTOCOL48_H
#define PRIVATE_FILE_PROTOCOL48_H
#include "../file-channel/file_channel.h"
namespace MilesFileProtocol48 {
// Existing transport status values; only these apply to initial table installation.
enum InstallStatus { Installed=0, Unsupported=1, InvalidFields=3, LifecycleRefused=0x1001 };
enum { InstallCallBytes=136, InstallReplyBytes=128, ConsumptionAckBytes=136 };
struct InstallRequest {
    MilesWire::Header header;
    uint64_t registration;
    InstallRequest():header(),registration(0) {}
};
// Header is an admitted ordinary command: Request/opcode28, nonzero request/lane,
// causal=lease=0. Does not authenticate, enforce replay, or call the SDK.
bool encodeInstall(const MilesWire::Header &,uint64_t registration,
                   unsigned char *,size_t capacity,size_t &written);
bool decodeInstall(MilesTransport::Bytes,const MilesWire::Header &expected,InstallRequest &out);
// Installed may be supplied ONLY after actual SDK installation returned; the
// helper encodes a report, not proof that SDK installation happened.
bool encodeInstallReply(const InstallRequest &,InstallStatus,
                        unsigned char *,size_t capacity,size_t &written);
bool decodeInstallReply(MilesTransport::Bytes,const InstallRequest &,InstallStatus &out);

// Fixed retained expectation: captured before file side effects, no filename or
// client-local intake. Authenticated session and consumed-once state remain outer.
struct FileAckExpected {
    MilesWire::Header original;
    uint64_t registration;
    FileAckExpected():original(),registration(0) {}
};
bool expectFileAck(const MilesFileChannel26::Request &,uint64_t installedRegistration,FileAckExpected &out);
// Host's sole callback transport owner calls this only AFTER reply validation,
// read-copy/open-token publication/close bookkeeping. Not an ACK of send alone.
// No allocation and no raw native-struct copying. Rejection leaves output intact.
bool encodeFileConsumptionAck(const FileAckExpected &,unsigned char *,size_t capacity,size_t &written);
bool validateFileConsumptionAck(MilesTransport::Bytes,const FileAckExpected &);
// Route this distinct opcode before reverse-file monotonic intake; after full
// validation call mapper.acknowledgeConsumed(authenticatedSession, original.request).
// This stateless codec does NOT consume replay state or authorize duplicate ACKs.

// v3 retains v2 opcode59 success parent echo, including null alias. Outer code
// retains refusal handling and stable alias ownership; this validates success only.
bool decodeStreamAliasSuccess(MilesTransport::Bytes,const MilesWire::Header &expected,
                              const MilesWire::Handle &expectedParent,MilesWire::Handle &alias);
}
#endif
