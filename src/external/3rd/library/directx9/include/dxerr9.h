/*==========================================================================;
 *
 *  File:   dxerr9.h
 *  Content: Compatibility shim
 *
 *  The original SOE-era SWG codebase included this header for the
 *  legacy `DXGetErrorString9A`/`DXGetErrorDescription9A`/`DXTrace*` set
 *  of helpers. Microsoft's DirectX SDK June 2010 dropped the "9"
 *  suffix and replaced them with `DXGetErrorString[AW]` /
 *  `DXGetErrorDescription[AW]` / `DXTrace[AW]` in `DxErr.lib`.
 *
 *  This shim forwards to the modern names so existing callers compile
 *  unchanged. Pair this header with linking `DxErr.lib` from the June
 *  2010 SDK (already in DXSDK_DIR\Lib\x64\DxErr.lib).
 *
 ****************************************************************************/

#ifndef _DXERR9_H_
#define _DXERR9_H_

#include <DxErr.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DXGetErrorString9A      DXGetErrorStringA
#define DXGetErrorString9W      DXGetErrorStringW
#define DXGetErrorDescription9A DXGetErrorDescriptionA
#define DXGetErrorDescription9W DXGetErrorDescriptionW

#ifdef UNICODE
#define DXGetErrorString9        DXGetErrorString9W
#define DXGetErrorDescription9   DXGetErrorDescription9W
#else
#define DXGetErrorString9        DXGetErrorString9A
#define DXGetErrorDescription9   DXGetErrorDescription9A
#endif

#ifdef __cplusplus
}
#endif

#endif // _DXERR9_H_
