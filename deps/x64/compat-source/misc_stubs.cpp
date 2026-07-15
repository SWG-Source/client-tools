// Static-library stubs:
//   * Logitech G15/G19 LCD SDK (lgLcd) - peripheral feature, never missed.
//   * pcre_malloc / pcre_free - sharedRegex was compiled against the
//     SOE-era pcre.h (plain `extern`, no dllimport) so its .obj references
//     bare `pcre_malloc` symbols. Modern vcpkg pcre.lib only exports the
//     dllimport-decorated `__imp_pcre_malloc` form. The SOE code only
//     reads/writes these to install a custom allocator, so providing
//     local globals here lets the link succeed; pcre.dll's own internal
//     copies stay defaulted to the CRT malloc/free, which is fine.
//
// libxml2 stubs were removed - vcpkg's libxml2.lib provides the real
// implementations.

#include <stddef.h>
#include <stdlib.h>

extern "C" {

void* (*pcre_malloc)(size_t) = malloc;
void  (*pcre_free)(void*)    = free;


unsigned long lgLcdInit(void) { return 0; }
unsigned long lgLcdDeInit(void) { return 0; }
unsigned long lgLcdConnectA(void*) { return 0; }
unsigned long lgLcdDisconnect(int) { return 0; }
unsigned long lgLcdEnumerate(int, int, void*) { return 0; }
unsigned long lgLcdOpen(void*) { return 0; }
unsigned long lgLcdClose(int) { return 0; }
unsigned long lgLcdReadSoftButtons(int, unsigned int*) { return 0; }
unsigned long lgLcdUpdateBitmap(int, const void*, unsigned int) { return 0; }
unsigned long lgLcdSetAsLCDForegroundApp(int, int) { return 0; }

} // extern "C"
