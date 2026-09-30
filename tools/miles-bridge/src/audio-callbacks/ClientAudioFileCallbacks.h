// Experimental client-local interface. Candidate only; not a wire structure.
#ifndef INCLUDED_ClientAudioFileCallbacks_H
#define INCLUDED_ClientAudioFileCallbacks_H

#include <stdint.h>

namespace ClientAudioFileCallbacks
{

// The original Audio.cpp map key, held only in this client process. Zero is a
// valid key. This wrapper is intentionally not convertible to bool or void *.
struct LocalFileHandle
{
	uintptr_t value;
};

struct OpenResult
{
	uint32_t callbackResult;
	LocalFileHandle handle; // Usable iff callbackResult != 0, even when value == 0.
};

enum SeekOrigin
{
	SeekBegin = 0,
	SeekCurrent = 1,
	SeekEnd = 2
};

// These ordinary C++ calls execute synchronously on the calling client thread.
// The coordinator must first establish engine/TLS lifetime, compatible thread
// context, serialization with ALL original file callbacks, and operation pins.
// This interface neither installs threads nor schedules callbacks. It must not
// be registered as an SDK callback table or called by an arbitrary I/O reactor.
//
// fileName is the original NUL-terminated byte name, live through the call.
// Read destination is caller-owned local storage of at least requestedBytes,
// live through the call. No native pointer or LocalFileHandle goes on the wire.
OpenResult open(char const *fileName);
void close(LocalFileHandle handle); // Original callback alone closes/deletes.
int32_t seek(LocalFileHandle handle, int32_t offset, uint32_t origin);
uint32_t read(LocalFileHandle handle, void *destination, uint32_t requestedBytes);

// seek returns the original signed result. read returns the original U32 bits,
// including the original int-to-U32 conversion; it does not clamp or normalize.
// Validation and transport failures belong outside this direct adapter.

} // namespace ClientAudioFileCallbacks

#endif
