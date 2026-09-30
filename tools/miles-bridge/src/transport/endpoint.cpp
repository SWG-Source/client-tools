#include "endpoint.h"
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <exception>
namespace MilesPipe {
namespace {
uint32_t length(const unsigned char *p) { return uint32_t(p[12])|(uint32_t(p[13])<<8)|(uint32_t(p[14])<<16)|(uint32_t(p[15])<<24); }
}
Endpoint::Operation::Operation():pending(false) { std::memset(&ov,0,sizeof ov); }
Endpoint::Endpoint(HANDLE owned,size_t limit):pipe(owned),current(Open),reason(None),lastError(0),
 segment(limit),headerUsed(0),readUsed(0),writeUsed(0),ready(false),reads(0),writes(0) {
 if(pipe==INVALID_HANDLE_VALUE || !pipe || !segment || segment>MilesWire::MaxFrameBytes) {
  if(pipe && pipe!=INVALID_HANDLE_VALUE)CloseHandle(pipe);
  throw std::invalid_argument("invalid endpoint construction");
 }
 readOp.ov.hEvent=CreateEventA(0,TRUE,FALSE,0);
 writeOp.ov.hEvent=CreateEventA(0,TRUE,FALSE,0);
 if(!readOp.ov.hEvent || !writeOp.ov.hEvent) {
  if(readOp.ov.hEvent)CloseHandle(readOp.ov.hEvent);
  if(writeOp.ov.hEvent)CloseHandle(writeOp.ov.hEvent);
  CloseHandle(pipe);throw std::runtime_error("CreateEvent failed");
 }
}
Endpoint::~Endpoint() { cancel();if(!drain(INFINITE))std::terminate();CloseHandle(readOp.ov.hEvent);CloseHandle(writeOp.ov.hEvent); }
void Endpoint::cancelPending() {
 // ERROR_NOT_FOUND can mean completion won the race. Always collect afterwards.
 if(readOp.pending)CancelIoEx(pipe,&readOp.ov);
 if(writeOp.pending)CancelIoEx(pipe,&writeOp.ov);
}
void Endpoint::fault(Failure why,DWORD error) {
 if(current==Open) { reason=why;lastError=error;current=Faulted; }
 cancelPending();
}
void Endpoint::cancel() { if(current==Open)current=Stopping;cancelPending(); }
bool Endpoint::send(MilesTransport::Bytes frame) {
 if(current!=Open || !writeBuffer.empty() || !frame.data || frame.size<48 || frame.size>MilesWire::MaxFrameBytes)return false;
 if(length(frame.data)!=frame.size)return false;
 std::vector<unsigned char> copied(frame.data,frame.data+frame.size);
 writeBuffer.swap(copied);writeUsed=0;return true;
}
bool Endpoint::takeFrame(std::vector<unsigned char> &out,bool allowPeerClosed) {
 if(!ready || (current!=Open && !(allowPeerClosed && current==Faulted && reason==PeerClosed)))return false;
 std::vector<unsigned char> completed;completed.swap(readBuffer);out.swap(completed);
 headerUsed=readUsed=0;ready=false;return true;
}
void Endpoint::readCompleted(DWORD count) {
 ++reads;
 if(!count) { fault(PeerClosed,ERROR_BROKEN_PIPE);return; }
 if(headerUsed<48) {
  headerUsed+=count;
  if(headerUsed==48) {
   const uint32_t bytes=length(header);
   if(bytes<48 || bytes>MilesWire::MaxFrameBytes) { fault(InvalidLength,0);return; }
   try { readBuffer.resize(bytes); } catch(...) { fault(Storage,0);throw; }
   std::memcpy(&readBuffer[0],header,48);readUsed=48;
   if(bytes==48)ready=true;
  }
 } else { readUsed+=count;if(readUsed==readBuffer.size())ready=true; }
}
void Endpoint::writeCompleted(DWORD count) {
 ++writes;
 if(!count) { fault(PeerClosed,ERROR_BROKEN_PIPE);return; }
 writeUsed+=count;if(writeUsed==writeBuffer.size()) { writeBuffer.clear();writeUsed=0; }
}
bool Endpoint::collect(Operation &op,bool read) {
 DWORD count=0;
 if(!GetOverlappedResult(pipe,&op.ov,&count,FALSE)) {
  DWORD e=GetLastError();if(e==ERROR_IO_INCOMPLETE)return false;
  op.pending=false;
  if(current==Open)fault(e==ERROR_BROKEN_PIPE || e==ERROR_PIPE_NOT_CONNECTED?PeerClosed:WindowsIo,e);
  return true;
 }
 op.pending=false;
 if(current==Open) { if(read)readCompleted(count);else writeCompleted(count); }
 return true;
}
bool Endpoint::stepRead() {
 if(readOp.pending)return collect(readOp,true);
 if(current!=Open || ready)return false;
 unsigned char *p=header;size_t remaining=48-headerUsed;
 if(headerUsed<48)p+=headerUsed;
 else { p=&readBuffer[0]+readUsed;remaining=readBuffer.size()-readUsed; }
 DWORD count=0,wanted=static_cast<DWORD>((std::min)(segment,remaining));
 ResetEvent(readOp.ov.hEvent);
 if(ReadFile(pipe,p,wanted,&count,&readOp.ov)) { readCompleted(count);return true; }
 DWORD e=GetLastError();if(e==ERROR_IO_PENDING) { readOp.pending=true;return false; }
 fault(e==ERROR_BROKEN_PIPE || e==ERROR_PIPE_NOT_CONNECTED?PeerClosed:WindowsIo,e);return true;
}
bool Endpoint::stepWrite() {
 if(writeOp.pending)return collect(writeOp,false);
 if(current!=Open || writeBuffer.empty())return false;
 DWORD count=0,wanted=static_cast<DWORD>((std::min)(segment,writeBuffer.size()-writeUsed));
 ResetEvent(writeOp.ov.hEvent);
 if(WriteFile(pipe,&writeBuffer[0]+writeUsed,wanted,&count,&writeOp.ov)) { writeCompleted(count);return true; }
 DWORD e=GetLastError();if(e==ERROR_IO_PENDING) { writeOp.pending=true;return false; }
 fault(e==ERROR_BROKEN_PIPE || e==ERROR_PIPE_NOT_CONNECTED?PeerClosed:WindowsIo,e);return true;
}
void Endpoint::pump() {
 for(unsigned i=0;i<4;++i) { bool r=stepRead(),w=stepWrite();if(!r&&!w)break; }
}
bool Endpoint::drain(DWORD timeout) {
 cancel();ULONGLONG start=GetTickCount64();
 while(pending()) {
  pump();if(!pending())break;
  ULONGLONG elapsed=GetTickCount64()-start;
  if(timeout!=INFINITE && elapsed>=timeout)return false;
  HANDLE events[2];DWORD n=0;
  if(readOp.pending)events[n++]=readOp.ov.hEvent;
  if(writeOp.pending)events[n++]=writeOp.ov.hEvent;
  DWORD remaining=timeout==INFINITE?INFINITE:timeout-static_cast<DWORD>(elapsed);
  DWORD result=WaitForMultipleObjects(n,events,FALSE,remaining);
  if(result==WAIT_TIMEOUT || result==WAIT_FAILED)return false;
 }
 if(pipe!=INVALID_HANDLE_VALUE) { CloseHandle(pipe);pipe=INVALID_HANDLE_VALUE; }
 current=Closed;return true;
}
}
