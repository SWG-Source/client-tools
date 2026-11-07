#ifndef LIVE_BRIDGE_COMMON_H
#define LIVE_BRIDGE_COMMON_H
#include "../transport/endpoint.h"
#include <sddl.h>
#include <wincrypt.h>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <cstring>
#pragma comment(lib,"advapi32.lib")
using MilesPipe::Endpoint;
inline void require(bool v,const char* what){if(!v){printf("FAIL %s error=%lu\n",what,GetLastError());throw std::runtime_error(what);}}
inline MilesTransport::Bytes bytes(const std::vector<unsigned char>& v){return MilesTransport::Bytes(v.empty()?0:&v[0],v.size());}
inline void pauseForIo(Endpoint &a,Endpoint *b) {
 HANDLE events[4];DWORD n=0;
 if(a.readPending())events[n++]=a.readEvent();if(a.writePending())events[n++]=a.writeEvent();
 if(b) { if(b->readPending())events[n++]=b->readEvent();if(b->writePending())events[n++]=b->writeEvent(); }
 if(n)require(WaitForMultipleObjects(n,events,FALSE,2)!=WAIT_FAILED,"wait io");
 else SwitchToThread();
}
inline void healthy(Endpoint& endpoint, bool orderedPeerClose = false) {
 require(endpoint.state() == Endpoint::Open ||
     (orderedPeerClose && endpoint.failure() == Endpoint::PeerClosed), "session channel fault");
}
inline std::vector<unsigned char> receive(Endpoint &a,Endpoint *b=0, bool finalReply=false) {
 ULONGLONG begin=GetTickCount64();std::vector<unsigned char> out;
 while(GetTickCount64()-begin<10000) {
  a.pump();if(b)b->pump();
  // Inspect the idle channel before accepting a ready command frame.
  if(b)healthy(*b,finalReply);
  healthy(a);
  if(a.takeFrame(out)) { require(a.bodyCapacity()==0,"receive transfers storage");return out; }
  pauseForIo(a,b);
 }
 throw std::runtime_error("receive watchdog");
}
inline void drainSession(Endpoint& a, Endpoint& b, bool orderedClose) {
 // drain closes storage; its success alone does not certify channel health.
 a.pump();b.pump();healthy(a,orderedClose);healthy(b,orderedClose);
 require(a.drain(3000) && b.drain(3000), "session drain");
 require(a.failure()==Endpoint::None || (orderedClose && a.failure()==Endpoint::PeerClosed), "command fault retained");
 require(b.failure()==Endpoint::None || (orderedClose && b.failure()==Endpoint::PeerClosed), "callback fault retained");
}
inline void sent(Endpoint &a) {
 ULONGLONG begin=GetTickCount64();
 while(a.sendBusy() && GetTickCount64()-begin<10000) { a.pump();require(a.state()==Endpoint::Open,"send channel fault");pauseForIo(a,0); }
 require(!a.sendBusy(),"send watchdog");
}
struct ChildProcess {
 PROCESS_INFORMATION info;
 HANDLE job;
 ChildProcess():job(0) { std::memset(&info,0,sizeof info); }
 void close() {
  if(info.hProcess) {
   DWORD code=0;
   if(!GetExitCodeProcess(info.hProcess,&code) || code==STILL_ACTIVE) {
    if(!TerminateProcess(info.hProcess,90))std::terminate();
    if(WaitForSingleObject(info.hProcess,INFINITE)!=WAIT_OBJECT_0)std::terminate();
   }
  }
  if(info.hThread)CloseHandle(info.hThread);
  if(info.hProcess)CloseHandle(info.hProcess);
  if(job)CloseHandle(job);
  std::memset(&info,0,sizeof info);job=0;
 }
 ~ChildProcess(){close();}
 void create(const char* executable, char* command) {
  require(!info.hProcess && !job,"fresh child owner");
  job=CreateJobObjectA(0,0);require(job!=0,"job");
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  require(SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof limits)!=0,"job limit");
  STARTUPINFOA si={};si.cb=sizeof si;
  // The private audio worker must not create a second UI or steal game focus.
  require(CreateProcessA(executable,command,0,0,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,0,0,&si,&info)!=0,"launch child");
 }
 void assignAndResume(HANDLE destinationJob) {
  require(AssignProcessToJobObject(destinationJob,info.hProcess)!=0,"assign child");
  require(ResumeThread(info.hThread)!=static_cast<DWORD>(-1),"resume child");
  CloseHandle(info.hThread);info.hThread=0;
 }
private:
 ChildProcess(const ChildProcess&);
 ChildProcess& operator=(const ChildProcess&);
};
struct ServerPipe {
 HANDLE pipe,event;OVERLAPPED ov;bool pending;
 ServerPipe(const std::string &name,SECURITY_ATTRIBUTES &sa):pipe(INVALID_HANDLE_VALUE),event(0),pending(false) {
  std::memset(&ov,0,sizeof ov);
  pipe=CreateNamedPipeA(name.c_str(),PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE,
    PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,1,4096,4096,0,&sa);
  require(pipe!=INVALID_HANDLE_VALUE,"CreateNamedPipe");event=CreateEventA(0,TRUE,FALSE,0);if(!event) { CloseHandle(pipe);pipe=INVALID_HANDLE_VALUE;throw std::runtime_error("connect event"); }ov.hEvent=event;
  if(!ConnectNamedPipe(pipe,&ov)) { DWORD e=GetLastError();require(e==ERROR_IO_PENDING || e==ERROR_PIPE_CONNECTED,"ConnectNamedPipe");pending=e==ERROR_IO_PENDING; }
 }
 void connected() { if(pending) { require(WaitForSingleObject(event,10000)==WAIT_OBJECT_0,"connect timeout");DWORD n=0;require(GetOverlappedResult(pipe,&ov,&n,FALSE)!=0,"connect result");pending=false; } }
 HANDLE take() { HANDLE h=pipe;pipe=INVALID_HANDLE_VALUE;return h; }
 ~ServerPipe() { if(pending) { CancelIoEx(pipe,&ov);DWORD n;GetOverlappedResult(pipe,&ov,&n,TRUE); }if(pipe!=INVALID_HANDLE_VALUE)CloseHandle(pipe);if(event)CloseHandle(event); }
};
inline std::string nonce() {
 HCRYPTPROV provider=0;require(CryptAcquireContextA(&provider,0,0,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT)!=0,"random provider");
 unsigned char random[16];BOOL ok=CryptGenRandom(provider,sizeof random,random);CryptReleaseContext(provider,0);require(ok!=0,"random bytes");
 char text[33];const char hex[]="0123456789abcdef";for(unsigned i=0;i<16;++i) { text[i*2]=hex[random[i]>>4];text[i*2+1]=hex[random[i]&15]; }text[32]=0;return text;
}
inline PSECURITY_DESCRIPTOR userDescriptor() {
 HANDLE token=0;require(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)!=0,"token");DWORD needed=0;
 GetTokenInformation(token,TokenUser,0,0,&needed);std::vector<unsigned char> storage(needed);
 require(GetTokenInformation(token,TokenUser,&storage[0],needed,&needed)!=0,"token user");CloseHandle(token);
 LPSTR sid=0;require(ConvertSidToStringSidA(reinterpret_cast<TOKEN_USER *>(&storage[0])->User.Sid,&sid)!=0,"sid text");
 std::string sddl="D:P(A;;GA;;;";sddl+=sid;sddl+=")";LocalFree(sid);PSECURITY_DESCRIPTOR descriptor=0;
 require(ConvertStringSecurityDescriptorToSecurityDescriptorA(sddl.c_str(),SDDL_REVISION_1,&descriptor,0)!=0,"private acl");return descriptor;
}

#endif
