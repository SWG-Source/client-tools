#ifndef CLIENT_BINK_DEVELOPMENT_SURFACE_H
#define CLIENT_BINK_DEVELOPMENT_SURFACE_H
#include "ClientMiles.h"
namespace ClientBink {
// Private development adapter for the existing Video factory. No vendor
// structures or remote addresses escape this source boundary.
struct VideoToken;
typedef VideoToken *Handle;
struct Info { uint32_t width, height, frames, frame, lastFrame, rate, rateDiv; };
enum PixelFormat { Surface32A=5, Surface5551=8, Surface565=10 };
int32_t initialize(ClientMiles::HDIGDRIVER driver, uint32_t pixelBudget);
void shutdown();
Handle open(const char *filename);
void close(Handle);
Info info(Handle);
int32_t doFrame(Handle);
void nextFrame(Handle);
int32_t wait(Handle);
int32_t shouldSkip(Handle);
void service(Handle);
int32_t pause(Handle,bool);
int32_t setVideoOnOff(Handle,bool);
int32_t setSoundOnOff(Handle,bool);
void setVolume(Handle,uint32_t track,int32_t volume);
// Caller owns a valid writable destination of stride*height bytes. Copies only
// the movie rectangle at (0,0); native pixel conversion happens in the host.
int32_t copyToBuffer(Handle,void *destination,int32_t stride,uint32_t height,PixelFormat);
const char *lastError();
}
#endif
