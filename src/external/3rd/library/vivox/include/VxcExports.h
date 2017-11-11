#ifndef __VXCEXPORTS_H
#define __VXCEXPORTS_H
#if defined(VIVOXDOC) || defined(SWIG)
#define VIVOXSDK_DLLEXPORT
#else
#ifdef _MSC_VER
  #ifdef BUILDING_VIVOXSDK
    #define VIVOXSDK_DLLEXPORT __declspec(dllexport)
  #else
    #define VIVOXSDK_DLLEXPORT __declspec(dllimport)
  #endif
#else
  #define VIVOXSDK_DLLEXPORT __attribute__ ((visibility("default")))
#endif
#endif

#endif
