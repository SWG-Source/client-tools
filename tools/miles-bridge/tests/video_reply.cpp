#include "../src/api/pipe/VideoState.h"
#include "../src/bink/bink_protocol.h"
#include <cstdio>
#include <cstdlib>
namespace {
unsigned checks=0;
void check(bool ok,const char *name) { ++checks;if(!ok){std::fprintf(stderr,"FAIL %s\n",name);std::exit(1);} }
}
int main() {
    using namespace MilesWire;
    ClientMilesPipe::VideoState s;
    s.driver=Handle{Driver,1,1};
    Call c={};c.target=s.driver;
    StartupBridge::OwnedReply r;
    r.result.resource=Handle{Video,2,1};
    check(s.validate(BinkOpen,c,r),"new video");
    s.rows.push_back(std::unique_ptr<ClientBink::VideoToken>(new ClientBink::VideoToken));
    s.rows.back()->wire=r.result.resource;s.rows.back()->live=true;
    check(!s.validate(BinkOpen,c,r),"live duplicate");
    s.rows.back()->live=false;
    check(!s.validate(BinkOpen,c,r),"retired replay");
    r.result.resource.generation=2;
    check(s.validate(BinkOpen,c,r),"next generation");
    c.target.generation=2;
    check(!s.validate(BinkOpen,c,r),"wrong driver");
    c.target=s.driver;r.result.resource=Handle();
    check(s.validate(BinkOpen,c,r),"native null open");
    s.copying=true;s.copyingVideo=Handle{Video,2,2};
    s.copyingInfo=ClientBink::Info{8,4,3,2,1,30,1};
    s.format=MilesBinkProtocol::Surface32A;s.total=128;s.offset=32;
    c=Call();c.target=s.copyingVideo;c.value[0]=s.format;r=StartupBridge::OwnedReply();
    r.result.value[0]=8;r.result.value[1]=4;r.result.value[2]=128;r.result.value[3]=2;r.result.value[4]=s.format;
    check(s.validate(BinkPixelsBegin,c,r),"begin native zero");
    r.result.return_bits=0xffffffffU;
    check(s.validate(BinkPixelsBegin,c,r),"begin signed native result");
    for(unsigned i=0;i<5;++i) { ++r.result.value[i];check(!s.validate(BinkPixelsBegin,c,r),"begin tuple changed");--r.result.value[i]; }
    ++c.value[0];check(!s.validate(BinkPixelsBegin,c,r),"requested format changed");--c.value[0];
    ++c.target.generation;check(!s.validate(BinkPixelsBegin,c,r),"begin target changed");--c.target.generation;
    s.copying=false;check(!s.validate(BinkPixelsBegin,c,r),"begin no transaction");s.copying=true;
    c.value[0]=32;c.value[1]=64;r=StartupBridge::OwnedReply();r.bytes.resize(64);
    check(s.validate(BinkPixelsChunk,c,r),"valid chunk");
    ++c.value[0];check(!s.validate(BinkPixelsChunk,c,r),"wrong offset");--c.value[0];
    ++c.value[1];check(!s.validate(BinkPixelsChunk,c,r),"wrong count");--c.value[1];
    r.bytes.pop_back();check(!s.validate(BinkPixelsChunk,c,r),"truncated bytes");r.bytes.push_back(0);
    c.value[1]=97;r.bytes.resize(97);check(!s.validate(BinkPixelsChunk,c,r),"beyond total");
    c.value[1]=0;r.bytes.clear();check(!s.validate(BinkPixelsChunk,c,r),"empty chunk");
    c.value[1]=64;r.bytes.resize(64);++c.target.generation;
    check(!s.validate(BinkPixelsChunk,c,r),"chunk target changed");--c.target.generation;
    s.copying=false;check(!s.validate(BinkPixelsChunk,c,r),"chunk no transaction");
    std::printf("PASS video reply %u checks\n",checks);return 0;
}
