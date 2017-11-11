// Copyright 2008 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_SYSTEMINTERNAL_H
#define SOEUTIL_SYSTEMINTERNAL_H

#include <stdlib.h>
#include <stdarg.h>


namespace SoeUtil
{
namespace SystemInternal
{


int StringFormatV_Internal(char *dest, int destLen, const char *formatString, va_list params);
int StringFormatV_Internal(wchar_t *dest, int destLen, const wchar_t *formatString, va_list params);
int StringFormatCountV_Internal(const char *formatString, va_list params);
int StringFormatCountV_Internal(const wchar_t *formatString, va_list params);
void DebugOutput_Internal(const char *formatString, va_list params);


}   // namespace SystemInternal
}   // namespace SoeUtil


#endif

