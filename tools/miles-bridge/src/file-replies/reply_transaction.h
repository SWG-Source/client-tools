#ifndef HOST_FILE_REPLY_TRANSACTION49_H
#define HOST_FILE_REPLY_TRANSACTION49_H
#include "file_tokens.h"
#include "../file-protocol/file_protocol.h"
namespace MilesHostFiles49 {
// One serialized host callback. Request/token/buffer preparation precedes send.
// FileTokens and vendor destination must outlive this transaction. The caller
// supplies authentic session context and a genuine validated outstanding Request.
class ReplyTransaction {
public:
    enum State { Prepared, Issued, Consumed, Acked, Cancelled, Failed };
    ReplyTransaction(FileTokens &,const MilesFileChannel26::Request &,uint64_t registration,
                     uint32_t existingToken,void *readDestination=0,size_t readCapacity=0);
    // Issued/Consumed/Failed destruction is forbidden: unknown effects/ACK uncertainty
    // require retention or the nonreturning host failure boundary, never rollback.
    ~ReplyTransaction();
    bool cancelBeforeSend() throw();
    bool beginSend() throw(); // Call BEFORE transport may expose any request byte.
    bool consume(MilesTransport::Bytes reply) throw(); // false => terminal host failure; no retry
    MilesTransport::Bytes ack() const throw(); // available only after exact consumption
    bool observeAckWriteComplete() throw(); // host I/O owner reports actual completed write
    bool result(uint32_t &returnBits,uint32_t &openedToken) const throw(); // only after Acked
    State state() const throw(){return current;}
private:
    FileTokens &tokens;
    MilesFileChannel26::Request request;
    uint32_t token,returnedBits,opened;
    void *destination;
    size_t capacity;
    unsigned char ackBytes[MilesFileProtocol48::ConsumptionAckBytes];
    size_t ackSize;
    State current;
    ReplyTransaction(const ReplyTransaction &);
    ReplyTransaction &operator=(const ReplyTransaction &);
};
}
#endif
