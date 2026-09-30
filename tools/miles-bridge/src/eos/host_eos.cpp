#include "host_eos.h"
#include "../host-runtime/host_file_runtime.h"
#include <Mss.h>
#include <list>
namespace MilesHostEos {
namespace {
MilesHostRuntime50::Runtime *runtime=0;
SRWLOCK mapLock=SRWLOCK_INIT;
struct Row { void *native; MilesWire::Handle resource; unsigned active; };
std::list<Row> rows;
struct Guard { Guard(){AcquireSRWLockExclusive(&mapLock);} ~Guard(){ReleaseSRWLockExclusive(&mapLock);} };
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b){return a.kind==b.kind&&a.slot==b.slot&&a.generation==b.generation;}
void dispatch(void *native,uint64_t token) {
    try {
        Row *row=0;MilesWire::Handle resource={};
        {
            Guard guard;
            for(std::list<Row>::iterator i=rows.begin();i!=rows.end();++i)
                if(i->native==native&&i->resource.kind==static_cast<uint32_t>(token<=64?MilesWire::OwnedSample:MilesWire::Stream)){row=&*i;break;}
            if(!runtime||!row||row->active==~0u)MilesHostRuntime50::fatal();
            ++row->active;resource=row->resource;
        }
        runtime->invokeEos(resource,token);
        {Guard guard;if(!row->active)MilesHostRuntime50::fatal();--row->active;}
    }catch(...){MilesHostRuntime50::fatal();}
}
template<unsigned N> void AILCALLBACK sampleThunk(HSAMPLE sample){dispatch(sample,N+1);}
template<unsigned N> void AILCALLBACK streamThunk(HSTREAM stream){dispatch(stream,N+65);}
AILSAMPLECB samples[]={&sampleThunk<0>,&sampleThunk<1>,&sampleThunk<2>,&sampleThunk<3>,&sampleThunk<4>,&sampleThunk<5>,&sampleThunk<6>,&sampleThunk<7>,&sampleThunk<8>,&sampleThunk<9>,&sampleThunk<10>,&sampleThunk<11>,&sampleThunk<12>,&sampleThunk<13>,&sampleThunk<14>,&sampleThunk<15>,&sampleThunk<16>,&sampleThunk<17>,&sampleThunk<18>,&sampleThunk<19>,&sampleThunk<20>,&sampleThunk<21>,&sampleThunk<22>,&sampleThunk<23>,&sampleThunk<24>,&sampleThunk<25>,&sampleThunk<26>,&sampleThunk<27>,&sampleThunk<28>,&sampleThunk<29>,&sampleThunk<30>,&sampleThunk<31>,&sampleThunk<32>,&sampleThunk<33>,&sampleThunk<34>,&sampleThunk<35>,&sampleThunk<36>,&sampleThunk<37>,&sampleThunk<38>,&sampleThunk<39>,&sampleThunk<40>,&sampleThunk<41>,&sampleThunk<42>,&sampleThunk<43>,&sampleThunk<44>,&sampleThunk<45>,&sampleThunk<46>,&sampleThunk<47>,&sampleThunk<48>,&sampleThunk<49>,&sampleThunk<50>,&sampleThunk<51>,&sampleThunk<52>,&sampleThunk<53>,&sampleThunk<54>,&sampleThunk<55>,&sampleThunk<56>,&sampleThunk<57>,&sampleThunk<58>,&sampleThunk<59>,&sampleThunk<60>,&sampleThunk<61>,&sampleThunk<62>,&sampleThunk<63>};
AILSTREAMCB streams[]={&streamThunk<0>,&streamThunk<1>,&streamThunk<2>,&streamThunk<3>,&streamThunk<4>,&streamThunk<5>,&streamThunk<6>,&streamThunk<7>,&streamThunk<8>,&streamThunk<9>,&streamThunk<10>,&streamThunk<11>,&streamThunk<12>,&streamThunk<13>,&streamThunk<14>,&streamThunk<15>,&streamThunk<16>,&streamThunk<17>,&streamThunk<18>,&streamThunk<19>,&streamThunk<20>,&streamThunk<21>,&streamThunk<22>,&streamThunk<23>,&streamThunk<24>,&streamThunk<25>,&streamThunk<26>,&streamThunk<27>,&streamThunk<28>,&streamThunk<29>,&streamThunk<30>,&streamThunk<31>,&streamThunk<32>,&streamThunk<33>,&streamThunk<34>,&streamThunk<35>,&streamThunk<36>,&streamThunk<37>,&streamThunk<38>,&streamThunk<39>,&streamThunk<40>,&streamThunk<41>,&streamThunk<42>,&streamThunk<43>,&streamThunk<44>,&streamThunk<45>,&streamThunk<46>,&streamThunk<47>,&streamThunk<48>,&streamThunk<49>,&streamThunk<50>,&streamThunk<51>,&streamThunk<52>,&streamThunk<53>,&streamThunk<54>,&streamThunk<55>,&streamThunk<56>,&streamThunk<57>,&streamThunk<58>,&streamThunk<59>,&streamThunk<60>,&streamThunk<61>,&streamThunk<62>,&streamThunk<63>};
}
void initialize(MilesHostRuntime50::Runtime &owner){
    Guard guard;if(runtime)MilesHostRuntime50::fatal();runtime=&owner;
}
uint64_t registerCallback(void *native,const MilesWire::Handle &resource,uint64_t token){
    const bool sample=resource.kind==MilesWire::OwnedSample;
    if(!native||!resource.slot||!resource.generation||(!sample&&resource.kind!=MilesWire::Stream)||
       (token&&(sample?(token>64):(token<65||token>128))))MilesHostRuntime50::fatal();
    {
        Guard guard;if(!runtime)MilesHostRuntime50::fatal();
        bool found=false;
        for(std::list<Row>::iterator i=rows.begin();i!=rows.end();++i)if(i->native==native&&i->resource.kind==resource.kind){
            if(!same(i->resource,resource))MilesHostRuntime50::fatal();found=true;break;
        }
        if(!found){Row row={native,resource,0};rows.push_back(row);}
    }
    // No bridge lock spans SDK entry: callbacks may be synchronous and may wait.
    if(sample){
        const AILSAMPLECB prior=::AIL_register_EOS_callback(static_cast<HSAMPLE>(native),token?samples[token-1]:0);
        if(!prior)return 0;
        for(unsigned i=0;i<64;++i)if(prior==samples[i])return i+1;
    }else{
        const AILSTREAMCB prior=::AIL_register_stream_callback(static_cast<HSTREAM>(native),token?streams[token-65]:0);
        if(!prior)return 0;
        for(unsigned i=0;i<64;++i)if(prior==streams[i])return i+65;
    }
    MilesHostRuntime50::fatal(); // never disguise an unrepresentable native callback
}
void released(void *native,const MilesWire::Handle &resource){
    Guard guard;
    for(std::list<Row>::iterator i=rows.begin();i!=rows.end();++i)if(i->native==native&&same(i->resource,resource)){
        if(i->active)MilesHostRuntime50::fatal();rows.erase(i);return;
    }
}
void shutdownComplete(){
    Guard guard;for(std::list<Row>::const_iterator i=rows.begin();i!=rows.end();++i)if(i->active)MilesHostRuntime50::fatal();
    rows.clear();
}
}
