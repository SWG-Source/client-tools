#ifndef __PRECOMPILE_H__
#define __PRECOMPILE_H__

#pragma warning( disable : 4702 )

#include <cstdlib>
#include "UIStlFwd.h"
#include "UiReport.h"
#include "UnicodeUtils.h"

// Reference an argument so that it won't generate a compiler warning, but do nothing with it
#define UI_UNREF(a)         (static_cast<void>(a))
#define UI_IGNORE_RETURN(a) (static_cast<void>(a))
#define UI_NOP              (static_cast<void>(0))

#ifdef _DEBUG
#define UI_ASSERT(a) assert (a)
#else
#define UI_ASSERT(a) UI_UNREF (a)
#endif

typedef unsigned char byte;

#endif // __PRECOMPILE_H__
