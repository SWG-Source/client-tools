// ATLMFC build numbers (YYMMDD)

// Avoid "pragma once" since Incredibuild generates warnings when atlmfc
// files are included in a precompiled header and in the source code.
// Instead we will rely upon old-fashioned guards.
//
//#pragma once

#ifndef __ATLBUILD_H__
#define __ATLBUILD_H__

#define _LIBS_BUILD 50727
#define _LIBS_USER_BUILD "50727"

#define _LIBS_PRODUCT_VERSION_MAJOR 8
#define _LIBS_USER_PRODUCT_VERSION_MAJOR "8"
#define _LIBS_PRODUCT_VERSION_MINOR 0
#define _LIBS_USER_PRODUCT_VERSION_MINOR "00"

#define _LIBS_FILE_VERSION_MAJOR 8
#define _LIBS_USER_FILE_VERSION_MAJOR "8"
#define _LIBS_FILE_VERSION_MINOR 0
#define _LIBS_USER_FILE_VERSION_MINOR "00"
#define _LIBS_USER_FULL_VER "8.00.50727"

#ifndef _LIBS_RBLD
#define _LIBS_RBLD 42
#define _LIBS_USER_RBLD "42"
#endif


#endif // __ATLBUILD_H__
