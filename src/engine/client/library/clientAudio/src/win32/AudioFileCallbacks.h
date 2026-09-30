#ifndef INCLUDED_AudioFileCallbacks_H
#define INCLUDED_AudioFileCallbacks_H

#include <stdint.h>

// Internal native-shaped file callbacks for an engine-admitted worker whose
// sharedThread entry point already owns PerThreadData installation/removal.
// These callbacks do not install TLS or synchronize Audio's shared file map.
// The caller must serialize file operations and keep Audio/file state alive
// until all operations finish. The direct Miles registration remains separate.
namespace AudioFileCallbacks
{
	// Open status is independent of the output handle: zero is a valid handle.
	uint32_t __stdcall openAdmitted(char const *fileName, uintptr_t *fileHandle);
	void __stdcall closeAdmitted(uintptr_t fileHandle);
	int32_t __stdcall seekAdmitted(uintptr_t fileHandle, int32_t offset, uint32_t type);
	uint32_t __stdcall readAdmitted(uintptr_t fileHandle, void *buffer, uint32_t bytes);
}

#endif
