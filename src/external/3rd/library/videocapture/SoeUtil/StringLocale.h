// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_STRINGLOCALE_H
#define SOEUTIL_STRINGLOCALE_H

#include <cwctype>

namespace SoeUtil
{
namespace Locale
{

    ////////////////////////////////////////////////////////////////////////
    // character classification/conversion functions
    ////////////////////////////////////////////////////////////////////////

int ToUpper(int c);          // takes ascii/unicode character and returns uppercase equivalent
int ToLower(int c);          // takes ascii/unicode character and returns lowercase equivalent
bool IsSpace(int c);         // is character considered whitespace
bool IsUpper(int c);         // is character considered upper-case
bool IsLower(int c);         // is character considered lower-case
bool IsPunctuation(int c);   // is character considered a punctuation
bool IsDigit(int c);         // is character considered a digit
bool IsAlpha(int c);         // is character considered a letter


    ////////////////////////////////////////////////////////////////////////
    // inline implementations
    ////////////////////////////////////////////////////////////////////////

inline int ToUpper(int c)
{
    return(std::towupper((wint_t)c));
}

inline int ToLower(int c)
{
    return(std::towlower((wint_t)c));
}

inline bool IsSpace(int c)
{
    return(iswctype((wint_t)c, wctype("space")) != 0);
    // return(std::iswspace((wint_t)c) != 0);
}

inline bool IsUpper(int c)
{
    return(std::iswupper((wint_t)c) != 0);
}

inline bool IsLower(int c)
{
    return(std::iswlower((wint_t)c) != 0);
}

inline bool IsPunctuation(int c)
{
    return(std::iswpunct((wint_t)c) != 0);
}

inline bool IsDigit(int c)
{
    return(iswctype((wint_t)c, wctype("digit")) != 0);
    // return(std::iswdigit((wint_t)c) != 0);
}

inline bool IsAlpha(int c)
{
    return(iswctype((wint_t)c, wctype("alpha")) != 0);
    // return(std::iswalpha((wint_t)c) != 0);
}


}   // namespace Locale
}   // namespace SoeUtil


#endif  // SOEUTIL_STRINGLOCALE_H
