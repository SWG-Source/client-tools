#include "reply_transaction.h"
#include <exception>
#include <stdexcept>
namespace MilesHostFiles49 {
namespace {
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
}
ReplyTransaction::ReplyTransaction(FileTokens &table,const MilesFileChannel26::Request &original,
    uint64_t registration,uint32_t existingToken,void *buffer,size_t bufferCapacity)
    :tokens(table),request(original),token(existingToken),returnedBits(0),opened(0),
     destination(buffer),capacity(bufferCapacity),ackSize(0),current(Prepared) {
    MilesFileProtocol48::FileAckExpected expected;
    if(!MilesFileProtocol48::expectFileAck(request,registration,expected) ||
       !MilesFileProtocol48::encodeFileConsumptionAck(expected,ackBytes,sizeof ackBytes,ackSize))
        throw std::invalid_argument("validated current-version file request and installed registration required");
    if(request.opcode()==MilesWire::FileRead) {
        if(capacity<request.count() || (request.count() && !destination))
            throw std::invalid_argument("read destination must cover complete requested extent");
    } else if(destination || capacity)throw std::invalid_argument("buffer only belongs to read");
    if(request.opcode()==MilesWire::FileOpen) {
        if(token)throw std::invalid_argument("open has no existing SDK token");
        // Last fallible preparation: reserve all registry ownership before send.
        if(!tokens.reserveOpen(token))throw std::runtime_error("host file capacity or token space exhausted before send");
    } else {
        MilesWire::Handle remote={};
        if(!tokens.resolve(token,remote) || !same(remote,request.target()))
            throw std::invalid_argument("SDK token must resolve to exact client wire File identity");
    }
}
ReplyTransaction::~ReplyTransaction() {
    if(current==Prepared) {if(!cancelBeforeSend())std::terminate();}
    else if(current!=Acked && current!=Cancelled)std::terminate();
}
bool ReplyTransaction::cancelBeforeSend() throw() {
    if(current!=Prepared)return false;
    if(request.opcode()==MilesWire::FileOpen && !tokens.cancelOpen(token))return false;
    current=Cancelled;return true;
}
bool ReplyTransaction::beginSend() throw() {
    if(current!=Prepared)return false;
    if(request.opcode()==MilesWire::FileClose && !tokens.beginClose(token))return false;
    current=Issued;return true;
}
bool ReplyTransaction::consume(MilesTransport::Bytes frame) throw() {
    if(current!=Issued){current=Failed;return false;}
    current=Failed; // Sticky before validation: malformed/uncertain results cannot be retried.
    try {
        MilesFileChannel26::OwnedReply reply;
        if(MilesFileChannel26::decodeReply(frame,request,reply)!=MilesFileChannel26::Valid)return false;
        if(request.opcode()==MilesWire::FileOpen) {
            if(reply.returnBits) {
                if(!tokens.publishOpen(token,reply.file))return false;
                opened=token;
            } else if(!tokens.cancelOpen(token))return false;
        } else if(request.opcode()==MilesWire::FileRead) {
            // Real decoder checked frame bounds/payload extent; copyRead checks
            // destination capacity/nullness and returned-count equality again.
            if(MilesFileChannel26::copyRead(reply,request.count(),destination,capacity)!=MilesFileChannel26::Valid)return false;
        } else if(request.opcode()==MilesWire::FileClose) {
            if(!tokens.finishClose(token))return false;
        }
        // Seek bits and all status fields came through the exact real decoder.
        returnedBits=reply.returnBits;current=Consumed;return true;
    } catch(...) {return false;} // Allocation/validation uncertainty is never EOF or retry permission.
}
MilesTransport::Bytes ReplyTransaction::ack() const throw() {
    return current==Consumed?MilesTransport::Bytes(ackBytes,ackSize):MilesTransport::Bytes();
}
bool ReplyTransaction::observeAckWriteComplete() throw() {
    if(current!=Consumed)return false;
    current=Acked;return true;
}
bool ReplyTransaction::result(uint32_t &bits,uint32_t &openedToken) const throw() {
    if(current!=Acked)return false;
    bits=returnedBits;openedToken=opened;return true;
}
}
