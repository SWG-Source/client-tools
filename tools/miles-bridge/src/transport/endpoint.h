#ifndef MILES_PIPE_ENDPOINT_H
#define MILES_PIPE_ENDPOINT_H
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <vector>
#include "../wire/codec.h"
namespace MilesPipe {
class Endpoint {
public:
 enum State { Open, Faulted, Stopping, Closed };
 enum Failure { None, WindowsIo, InvalidLength, PeerClosed, Storage };
 explicit Endpoint(HANDLE ownedConnectedPipe,size_t segmentLimit=1048576);
 // May block waiting for OS cancellation completion. Normal use drains explicitly.
 ~Endpoint();
 bool send(MilesTransport::Bytes frame);
 bool takeFrame(std::vector<unsigned char> &out);
 void pump();
 void cancel();
 bool drain(DWORD timeoutMs); // Timeout leaves storage and handles alive.
 State state() const { return current; }
 Failure failure() const { return reason; }
 DWORD windowsError() const { return lastError; }
 bool readPending() const { return readOp.pending; }
 HANDLE readEvent() const { return readOp.ov.hEvent; }
 HANDLE writeEvent() const { return writeOp.ov.hEvent; }
 bool writePending() const { return writeOp.pending; }
 bool pending() const { return readOp.pending || writeOp.pending; }
 bool sendBusy() const { return !writeBuffer.empty(); }
 size_t bodyCapacity() const { return readBuffer.capacity(); }
 unsigned readCompletions() const { return reads; }
 unsigned writeCompletions() const { return writes; }
private:
 struct Operation { OVERLAPPED ov; bool pending; Operation(); };
 HANDLE pipe;
 Operation readOp,writeOp;
 State current;
 Failure reason;
 DWORD lastError;
 size_t segment,headerUsed,readUsed,writeUsed;
 unsigned char header[48];
 std::vector<unsigned char> readBuffer,writeBuffer;
 bool ready;
 unsigned reads,writes;
 void fault(Failure why,DWORD error);
 bool stepRead();
 bool stepWrite();
 void readCompleted(DWORD bytes);
 void writeCompleted(DWORD bytes);
 void cancelPending();
 bool collect(Operation &op,bool read);
 Endpoint(const Endpoint &);
 Endpoint &operator=(const Endpoint &);
};
}
#endif
