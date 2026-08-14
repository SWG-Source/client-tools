/* we need to all the OT support into one file to get efficient inlining */

#if defined(__GNUC__)
#define inline __inline__
#else
#define inline
#endif

#include "ftglue.c"
#include "ftxopen.c"
#include "ftxgdef.c"
#include "ftxgpos.c"
#include "ftxgsub.c"
#include "otlbuffer.c"
