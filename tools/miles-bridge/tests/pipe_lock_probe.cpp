// Development process only. File mode starts engine Thread/TLS and a worker;
// this never initializes Audio or runs global engine teardown.
#include "../src/api/ClientMiles.h"
#include "../src/api/pipe/LiveChannel.h"
#include "../src/failure/failure_boundary.h"
#include <windows.h>
#include <cstdio>
#include <exception>
#include <memory>
#include <process.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include "engine_worker_context.h"

namespace {
void finish(unsigned code, const char *message)
{
    std::fprintf(code ? stderr : stdout, "%s\n", message);
    std::fflush(stdout);
    std::fflush(stderr);
    // Session::close has no paired-close implementation. Retain all roots until
    // process death; this does not claim callback/worker/allocator teardown.
    ::ExitProcess(code);
}
void fatalReporter(uint32_t reason, const char *message)
{
    std::fprintf(stderr, "Miles fatal reason %lu: ", static_cast<unsigned long>(reason));
    finish(10, message ? message : "no diagnostic");
}
bool rejectedLockFields(ClientMilesPipe::Session &session, uint32_t opcode)
{
    MilesWire::Call fields = {};
    fields.value[0] = 1;
    try { session.request(opcode, fields); }
    catch (const ClientMilesPipeCore57::Failure &error) {
        return error.reason() == ClientMilesPipeCore57::FailureReason::InvalidArgument;
    }
    return false;
}
unsigned __stdcall wrongCaller(void *context)
{
    bool &rejected = *static_cast<bool *>(context);
    try { ClientMilesPipeCore57::get_preference(ClientMiles::MixFragmentCount); }
    catch (const ClientMilesPipeCore57::Failure &error) {
        rejected = error.reason() == ClientMilesPipeCore57::FailureReason::WrongState;
    }
    catch (...) {}
    return 0;
}
void put16(std::vector<unsigned char> &v,size_t at,unsigned value)
{ v[at]=static_cast<unsigned char>(value);v[at+1]=static_cast<unsigned char>(value>>8); }
void put32(std::vector<unsigned char> &v,size_t at,unsigned value)
{ put16(v,at,value);put16(v,at+2,value>>16); }
std::vector<unsigned char> wave()
{
    // Generated mono PCM silence; no licensed game media required.
    std::vector<unsigned char> bytes(44+8192*2,0);
    std::memcpy(bytes.data(),"RIFF",4);put32(bytes,4,static_cast<unsigned>(bytes.size()-8));
    std::memcpy(bytes.data()+8,"WAVEfmt ",8);put32(bytes,16,16);
    put16(bytes,20,1);put16(bytes,22,1);put32(bytes,24,22050);
    put32(bytes,28,44100);put16(bytes,32,2);put16(bytes,34,16);
    std::memcpy(bytes.data()+36,"data",4);put32(bytes,40,8192*2);
    return bytes;
}
void sampleBindings()
{
    ClientMiles::HDIGDRIVER driver=ClientMiles::open_digital_driver(22050,16,
        ClientMiles::StereoSpeakerConfiguration,0);
    if(!driver)finish(20,"FAIL: genuine digital driver");
    ClientMiles::HSAMPLE sample=ClientMiles::allocate_sample_handle(driver);
    if(!sample)finish(21,"FAIL: sample allocation");
    const std::vector<unsigned char> reference=wave();
    std::vector<unsigned char> image(reference);
    try {
        ClientMilesPipeCore57::set_sample_file(sample,image.data(),0);
        finish(22,"FAIL: size-less binding accepted without source extent");
    } catch(const ClientMilesPipeCore57::Failure &error) {
        if(error.reason()!=ClientMilesPipeCore57::FailureReason::InvalidArgument)throw;
    }
    int32_t expected=0;
    for(unsigned cycle=0;cycle<65;++cycle) {
        image=reference;
        int32_t result=0;
        if(cycle%2) {
            ClientMilesPipe::ScopedSourceImage extent(image.data(),static_cast<uint32_t>(image.size()));
            result=ClientMiles::set_sample_file(sample,image.data(),0);
        } else result=ClientMiles::set_named_sample_file(sample,".wav",image.data(),
            static_cast<uint32_t>(image.size()),0);
        if(!result)finish(23,"FAIL: real WAV bind/rebind");
        // The host must own its image after the command's upload is released.
        std::fill(image.begin(),image.end(),0);
        int32_t total=0,current=0;
        ClientMiles::sample_ms_position(sample,&total,&current);
        if(cycle==0)expected=total;
        if(total<=0 || total!=expected || current!=0)finish(24,"FAIL: retained sample duration");
        ClientMiles::start_sample(sample);
        ClientMiles::end_sample(sample);
    }
    std::vector<unsigned char> invalid(reference.size(),0);
    if(ClientMiles::set_named_sample_file(sample,".wav",invalid.data(),
        static_cast<uint32_t>(invalid.size()),0)!=0)finish(25,"FAIL: invalid WAV native result");
    if(!ClientMiles::set_named_sample_file(sample,".wav",reference.data(),
        static_cast<uint32_t>(reference.size()),0))finish(26,"FAIL: successful replacement after failed bind");
    ClientMiles::release_sample_handle(sample);
    std::puts("PASS: 65 named/unnamed rebinds; source extent and retained duration");
    std::puts("PASS: native failed bind, successful replacement, genuine release");
}
volatile LONG fileOpens=0,fileCloses=0,fileReads=0,fileSeeks=0,fileFaults=0;
bool fileThread()
{
    if(engineProbeThreadReady())return true;
    InterlockedIncrement(&fileFaults);return false;
}
uint32_t __stdcall openFile(const char *name,ClientMiles::FileHandle *out)
{
    if(!fileThread() || !name || !out || std::strcmp(name,"bridge-probe.wav"))return 0;
    HANDLE file=CreateFileA(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
    if(file==INVALID_HANDLE_VALUE)return 0;
    *out=reinterpret_cast<uintptr_t>(file);InterlockedIncrement(&fileOpens);return 1;
}
void __stdcall closeFile(ClientMiles::FileHandle file)
{
    if(!fileThread() || !CloseHandle(reinterpret_cast<HANDLE>(file))) {
        InterlockedIncrement(&fileFaults);return;
    }
    InterlockedIncrement(&fileCloses);
}
int32_t __stdcall seekFile(ClientMiles::FileHandle file,int32_t offset,uint32_t origin)
{
    if(!fileThread() || origin>FILE_END)return -1;
    LARGE_INTEGER distance,position;distance.QuadPart=offset;
    if(!SetFilePointerEx(reinterpret_cast<HANDLE>(file),distance,&position,origin) ||
       position.QuadPart<0 || position.QuadPart>INT32_MAX) {
        InterlockedIncrement(&fileFaults);return -1;
    }
    InterlockedIncrement(&fileSeeks);return static_cast<int32_t>(position.QuadPart);
}
uint32_t __stdcall readFile(ClientMiles::FileHandle file,void *buffer,uint32_t bytes)
{
    DWORD read=0;
    if(!fileThread() || !ReadFile(reinterpret_cast<HANDLE>(file),buffer,bytes,&read,0)) {
        InterlockedIncrement(&fileFaults);return 0;
    }
    InterlockedIncrement(&fileReads);return read;
}
void fileCallbacks()
{
    const std::vector<unsigned char> bytes=wave();
    HANDLE file=CreateFileA("bridge-probe.wav",GENERIC_WRITE,0,0,CREATE_NEW,0,0);
    DWORD written=0;
    if(file==INVALID_HANDLE_VALUE || !WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&written,0) ||
       written!=bytes.size() || !CloseHandle(file))finish(30,"FAIL: generated file creation");
    ClientMiles::set_file_callbacks(openFile,closeFile,seekFile,readFile);
    ClientMiles::HDIGDRIVER driver=ClientMiles::open_digital_driver(22050,16,
        ClientMiles::StereoSpeakerConfiguration,0);
    if(!driver)finish(31,"FAIL: file probe digital driver");
    ClientMiles::HSTREAM stream=ClientMiles::open_stream(driver,"bridge-probe.wav",0);
    if(!stream)finish(32,"FAIL: genuine open_stream through engine file worker");
    int32_t total=0,current=0;
    ClientMiles::stream_ms_position(stream,&total,&current);
    if(total<=0 || current!=0 || !ClientMiles::stream_sample_handle(stream))
        finish(33,"FAIL: streamed WAV position or borrowed sample");
    ClientMiles::close_stream(stream);
    const LONG opened=InterlockedCompareExchange(&fileOpens,0,0);
    const LONG closed=InterlockedCompareExchange(&fileCloses,0,0);
    const LONG reads=InterlockedCompareExchange(&fileReads,0,0);
    const LONG seeks=InterlockedCompareExchange(&fileSeeks,0,0);
    if(opened<1 || closed!=opened || reads<1 || seeks<1 || InterlockedCompareExchange(&fileFaults,0,0))
        finish(34,"FAIL: file callback effects or engine TLS");
    if(!DeleteFileA("bridge-probe.wav"))finish(35,"FAIL: generated file cleanup");
    std::puts("PASS: real engine file callbacks opened/read/sought/closed generated WAV");
    std::puts("PASS: file callbacks ran with engine TLS; native stream and borrowed sample returned");
}
}

int main(int argc, char **argv)
{
    const bool bindingMode=argc==4 && !std::strcmp(argv[3],"--sample-bindings");
    const bool fileMode=argc==4 && !std::strcmp(argv[3],"--file-callbacks");
    if (argc != 3 && !bindingMode && !fileMode)
        finish(2, "usage: miles-pipe-probe.exe <x86 host.exe> <original Mss32.dll> [--sample-bindings|--file-callbacks]");
    try {
        ClientMilesPrivate52::bindFatalReporter(fatalReporter);
        if(fileMode) {
            if(setupEngineProbe() || runEngineWorkerProbe())finish(36,"FAIL: engine worker prerequisite");
            std::puts("PASS: engine worker FIFO, TLS, drain/join and destruction");
        }
        // The code and engine modules are statically linked and process resident.
        // File mode above bootstraps Thread/TLS; other modes leave them dormant.
        // These tokens retain process-resident engine/callback code, not teardown.
        std::shared_ptr<void> engineLifetime(new int(1));
        std::shared_ptr<void> callbackLifetime(new int(1));
        ClientMilesPipe::Session *session = ClientMilesPipe::connectSession(
            argv[1], argv[2], engineLifetime, callbackLifetime, 1024 * 1024);
        if (!session || !ClientMiles::startup())
            finish(3, "FAIL: session/startup");
        intptr_t const expected = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        if (!rejectedLockFields(*session, MilesWire::AIL_lock))
            finish(11, "FAIL: malformed lock not rejected as InvalidArgument");
        ClientMiles::lock();
        ClientMiles::lock();
        intptr_t const nested = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        if (!rejectedLockFields(*session, MilesWire::AIL_unlock))
            finish(12, "FAIL: malformed unlock not rejected as InvalidArgument");
        ClientMiles::unlock();
        intptr_t const outer = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        ClientMiles::unlock();
        intptr_t const after = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        bool rejected = false;
        HANDLE caller = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, wrongCaller, &rejected, 0, 0));
        if (!caller || WaitForSingleObject(caller, 10000) != WAIT_OBJECT_0)
            finish(13, "FAIL: secondary caller did not join");
        CloseHandle(caller);
        if (!rejected)
            finish(14, "FAIL: secondary caller not rejected as WrongState");
        if (ClientMiles::get_preference(ClientMiles::MixFragmentCount) != expected)
            finish(15, "FAIL: owner preference after secondary caller");
        if(bindingMode)sampleBindings();
        if(fileMode)fileCallbacks();
        ClientMiles::shutdown();
        if (nested != expected || outer != expected || after != expected)
            finish(4, "FAIL: preference changed during nested lock/unlock");
        std::puts("PASS: malformed lock/unlock rejected; nested sequence completed");
        std::puts("PASS: secondary caller rejected; owner sequence completed");
        std::puts("PASS: startup, nested lock/unlock, ordinary preference, shutdown");
        finish(0, "PASS: pipe lock transport probe; test-only process exit, no teardown claim");
    } catch (const std::exception &error) {
        finish(5, error.what());
    } catch (...) {
        finish(6, "FAIL: unknown exception");
    }
    return 7;
}
