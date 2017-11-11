// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_STRINGUTIL_H
#define SOEUTIL_STRINGUTIL_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#include <limits>

#include "Types.h"
#include "StringLocale.h"
#include "SystemInternal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard C-style string functions
// note: templatized functions are intended to work on both char and wchar_t types (and not other types)
// note: non-safe ansi-C closest equivalents noted in comments (if applicable)
// note: the String object has a similar set of functions when dealing with a String objects.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace SoeUtil
{


// basic manipulation
template<typename T> int StringLength(const T *s);                                                      // strlen
template<typename T> int StringLength(const T *s, int maxCheckLen);                                     // strlen, only it stops after maxCheckLen and returns maxCheckLen if it reaches that far
template<typename T> T *StringCopy(T *dest, int destLen, const T *source);                              // strncpy
template<typename T> T *StringAppend(T *dest, int destLen, const T *source);                            // strcat
template<typename T> T *StringInsert(T *dest, int destLen, const T *source);
template<typename T> T *StringDelete(T *dest, int deleteLen);

// comparing/searching
template<typename T> int StringCompare(const T *s1, const T *s2);                                       // strcmp
template<typename T> int StringCompareI(const T *s1, const T *s2);                                      // stricmp
template<typename T> int StringCompare(const T *s1, const T *s2, int maxLen);                           // strncmp
template<typename T> int StringCompareI(const T *s1, const T *s2, int maxLen);                          // strnicmp
template<typename T> const T *StringFind(const T *source, const T *find);                               // strstr
template<typename T> const T *StringFindI(const T *source, const T *find);                              // stristr
template<typename T> const T *StringFindRight(const T *source, const T *find);
template<typename T> const T *StringFindRightI(const T *source, const T *find);
template<typename T> T *StringFind(T *source, const T *find);                                           // strstr (non-const)
template<typename T> T *StringFindI(T *source, const T *find);                                          // stristr (non-const)
template<typename T> T *StringFindRight(T *source, const T *find);
template<typename T> T *StringFindRightI(T *source, const T *find);
template<typename T> bool StringReplace(T *dest, int destLen, const T *find, const T *replace);         // retuns true if something changed
template<typename T> bool StringReplaceI(T *dest, int destLen, const T *find, const T *replace);        // retuns true if something changed

template<typename T> const T *StringFind(const T *source, int findChar);                                // strchr
template<typename T> const T *StringFindRight(const T *source, int findChar);                           // strrchr
template<typename T> T *StringFind(T *source, int findChar);                                            // strchr (non-const)
template<typename T> T *StringFindRight(T *source, int findChar);                                       // strrchr (non-const)

// compares a string against a wildcard specification, *=match 0 or more characters, ?=match 1 character
template<typename T> bool StringWildcardMatch(const T *source, const T *matchSpecification);            // case sensitive
template<typename T> bool StringWildcardMatchI(const T *source, const T *matchSpecification);           // case insensitive version

// printf style formatting
template<typename T> int StringFormat(T *dest, int destLen, const T *formatString, ...);               // snprintf
template<typename T> int StringFormatV(T *dest, int destLen, const T *formatString, va_list params);   // vsnprintf
template<typename T> int StringFormatCount(const T *formatString, ...);                                // scprintf
template<typename T> int StringFormatCountV(const T *formatString, va_list params);                    // vscprintf

// formats type T_VALUE to a string (built-in and SoeUtil types supported; application defined types can be specialized)
// note: these exist for completeness, the preferred method is to use ConvertType with a String object as the destination
template<typename T_VALUE> int StringFormatType(char *dest, int destLen, const T_VALUE &value);
template<typename T_VALUE> int StringFormatType(wchar_t *dest, int destLen, const T_VALUE &value);

// character stripping
template<typename T> bool StringTrimLeft(T *dest);       // trim all whitespace on left, returns true if something changed
template<typename T> bool StringTrimRight(T *dest);      // trim all whitespace on right, returns true if something changed
template<typename T> bool StringTrimAll(T *dest);        // trim left/right and all multiple-whitespace in a row, returns true if something changed
template<typename T> bool StringStrip(T *dest, T c);     // strip all instances of character 'c', returns true if something changed

// case conversion functions
template<typename T> bool StringToUpper(T *s);            // convert string to upper case, returns true if something changed
template<typename T> bool StringToLower(T *s);            // convert string to lower case, returns true if something changed

// hashing
// note: hashing-functions below are guaranteed to return 0 for an empty string
template<typename T> int StringHash(T *s);              // calculate a hash value for the specified string
template<typename T> int StringHash(const T *s, int maxLen);        // calculate a hash value for the specified string up to maxLen characters
template<typename T> int StringUpperHash(const T *s, int maxLen);   // calculate a hash value for the specified string up to maxLen characters
template<typename T> int StringUpperHash(T *s);         // calculate a hash value for the specified string in a case-insensitive manner
template<typename T> int StringHashLiteral(T *s);       // same result as StringHash, but works on code-literals and calculates the hash at compile time (in release builds)
                                                        // note: limited to 48 letters max

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // inline implementations
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

   // trick to limit template types supported (just to get additional compiler checking against likely misuse)
inline void StringEnforceType(char *) { }
inline void StringEnforceType(wchar_t *) { }

template<typename T> int StringLength(const T *s)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    const T *w = s;
    while (*w != 0)
    {
        ++w;
    }
    return((int)(w - s));
}

template<typename T> int StringLength(const T *s, int maxCheckLen)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    const T *w = s;
    const T *end = w + maxCheckLen;
    while (w != end && *w != 0)
    {
        ++w;
    }
    return((int)(w - s));
}

template<typename T> T *StringCopy(T *dest, int destLen, const T *source)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    SOEUTIL_ASSERT(destLen > 0);
    T *walk = dest;
    T *end = dest + destLen - 1;
    while (walk != end && *source != 0)
    {
        *walk++ = *source++;
    }
    *walk = 0;
    return(dest);
}

template<typename T> T *StringAppend(T *dest, int destLen, const T *source)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(dest);
    SOEUTIL_ASSERT(len < destLen);   // existing string longer than dest space available somehow
    StringCopy(dest + len, destLen - len, source);
    return(dest);
}

template<typename T> T *StringInsert(T *dest, int destLen, const T *source)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(dest);       // measuring length of strings in advance is not the optimal way to implement this function, but it will be fast enough
    int slen = StringLength(source);
    SOEUTIL_ASSERT(len < destLen);   // existing string longer than dest space available somehow

        // move existing string to make space first
    if (slen + len >= destLen)
    {
            // going to overflow
        if (slen >= destLen)
        {
                // source alone overflows, so just string copy it onto the end and be done
            std::memcpy(dest, source, (destLen - 1) * sizeof(T));
            dest[destLen - 1] = 0;
        }
        else
        {
                // takes both to overflow
            std::memmove(dest + slen, dest, (destLen - slen - 1) * sizeof(T));
            std::memcpy(dest, source, slen * sizeof(T));
            dest[destLen - 1] = 0;
        }
    }
    else
    {
            // all fits
        std::memmove(dest + slen, dest, (len + 1) * sizeof(T));
        std::memcpy(dest, source, slen * sizeof(T));
    }

    return(dest);
}

template<typename T> T *StringDelete(T *dest, int deleteLen)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(dest);
    if (len < deleteLen)
    {
        *dest = 0;
    }
    else
    {
        memmove(dest, dest + deleteLen, (len - deleteLen + 1) * sizeof(T));
    }
    return(dest);
}

template<typename T> int StringCompare(const T *s1, const T *s2)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    while (*s1 != 0 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }
    return((int)*s1 - (int)*s2);
}

template<typename T> int StringCompare(const T *s1, const T *s2, int maxLen)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    SOEUTIL_ASSERT(maxLen >= 0);

    const T *end = s1 + maxLen;
    while (s1 != end && *s1 != 0 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }
    return((s1 == end) ? 0 : ((int)*s1 - (int)*s2));
}

template<typename T> int StringCompareI(const T *s1, const T *s2)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    while (*s1 != 0 && Locale::ToUpper(*s1) == Locale::ToUpper(*s2))
    {
        ++s1;
        ++s2;
    }
    return((int)*s1 - (int)*s2);
}

template<typename T> int StringCompareI(const T *s1, const T *s2, int maxLen)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    SOEUTIL_ASSERT(maxLen >= 0);

    const T *end = s1 + maxLen;
    while (s1 != end && *s1 != 0 && Locale::ToUpper(*s1) == Locale::ToUpper(*s2))
    {
        ++s1;
        ++s2;
    }
    return((s1 == end) ? 0 : (Locale::ToUpper(*s1) - Locale::ToUpper(*s2)));
}

template<typename T> const T *StringFind(const T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(find);
    while (*source != 0)
    {
        if (StringCompare(find, source, len) == 0)
        {
            return(source);
        }
        source++;
    }
    return(NULL);
}

template<typename T> const T *StringFindI(const T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(find);
    while (*source != 0)
    {
        if (StringCompareI(find, source, len) == 0)
        {
            return(source);
        }
        source++;
    }
    return(NULL);
}

template<typename T> T *StringFind(T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    return((T *)StringFind((const T *)source, find));
}

template<typename T> T *StringFindI(T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    return((T *)StringFindI((const T *)source, find));
}

template<typename T> const T *StringFindRight(const T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(find);
    int slen = StringLength(source);
    const T *walk = source + (slen - len);
    while (walk >= source)
    {
        if (StringCompare(find, walk, len) == 0)
        {
            return(walk);
        }
        --walk;
    }
    return(NULL);
}

template<typename T> const T *StringFindRightI(const T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(find);
    int slen = StringLength(source);
    const T *walk = source + (slen - len);
    while (walk >= source)
    {
        if (StringCompareI(find, walk, len) == 0)
        {
            return(walk);
        }
        --walk;
    }
    return(NULL);
}

template<typename T> T *StringFindRight(T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    return((T *)StringFindRight((const T *)source, find));
}

template<typename T> T *StringFindRightI(T *source, const T *find)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    return((T *)StringFindRightI((const T *)source, find));
}

template<typename T> const T *StringFind(const T *source, int findChar)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    while (*source != 0 && (int)*source != findChar)
    {
        source++;
    }
    return(((int)*source != findChar) ? NULL : source);
}

template<typename T> const T *StringFindRight(const T *source, int findChar)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    const T *walk = source + StringLength(source);
    while (walk != source && (int)*walk != findChar)
    {
        walk--;
    }
    return(((int)*walk != findChar) ? NULL : walk);
}

template<typename T> T *StringFind(T *source, int findChar)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    while (*source != 0 && (int)*source != findChar)
    {
        source++;
    }
    return(((int)*source != findChar) ? NULL : source);
}

template<typename T> T *StringFindRight(T *source, int findChar)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    T *walk = source + StringLength(source);
    while (walk != source && (int)*walk != findChar)
    {
        walk--;
    }
    return(((int)*walk != findChar) ? NULL : walk);
}

template<typename T> bool StringReplace(T *dest, int destLen, const T *find, const T *replace)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type

    T *walk = dest;
    int flen = StringLength(find);
    SOEUTIL_ASSERT(flen > 0);       // if you find empty string and replace it, you will loop forever
    int rlen = StringLength(replace);

    bool ret = false;
    for (;;)
    {
        walk = StringFind(walk, find);
        if (walk == NULL)
        {
            break;
        }

        ret = true;
        StringDelete(walk, flen);
        StringInsert(walk, destLen - (int)(walk - dest), replace);
        walk += rlen;
    }
    return(ret);
}

template<typename T> bool StringReplaceI(T *dest, int destLen, const T *find, const T *replace)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type

    T *walk = dest;
    int flen = StringLength(find);
    SOEUTIL_ASSERT(flen > 0);       // if you find empty string and replace it, you will loop forever
    int rlen = StringLength(replace);

    bool ret = false;
    for (;;)
    {
        walk = StringFindI(walk, find);
        if (walk == NULL)
        {
            break;
        }

        ret = true;
        StringDelete(walk, flen);
        StringInsert(walk, destLen - (int)(walk - dest), replace);
        walk += rlen;
    }
    return(ret);
}

template<typename T> int StringFormatV(T *dest, int destLen, const T *formatString, va_list params)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    // no default implmentation, must be specialized for supported types
    return 0;
}

// NOTE: destLen is the number of characters NOT the number of bytes.  Returns the number of characters written to the buffer
inline int StringFormatV(char *dest, int destLen, const char *formatString, va_list params)
{
    return SystemInternal::StringFormatV_Internal(dest, destLen, formatString, params);
}

// NOTE: destLen is the number of characters NOT the number of bytes.  Returns the number of characters written to the buffer
inline int StringFormatV(wchar_t *dest, int destLen, const wchar_t *formatString, va_list params)
{
    return SystemInternal::StringFormatV_Internal(dest, destLen, formatString, params);
}

inline int StringFormatCountV(const char *formatString, va_list params)
{
    return SystemInternal::StringFormatCountV_Internal(formatString, params);
}

inline int StringFormatCountV(const wchar_t *formatString, va_list params)
{
    return SystemInternal::StringFormatCountV_Internal(formatString, params);
}

// NOTE: destLen is the number of characters NOT the number of bytes.
// Returns the number of characters written to the buffer
template<typename T> int StringFormat(T *dest, int destLen, const T *formatString, ...)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    va_list marker;
    va_start(marker, formatString);
    int numCharactersWritten = StringFormatV( dest, destLen, formatString, marker );
    va_end(marker);
    return numCharactersWritten;
}

// Returns the number of characters that would need to be written (not including terminating zero)
template<typename T> int StringFormatCount(const T *formatString, ...)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    va_list marker;
    va_start(marker, formatString);
    int numCharactersWritten = StringFormatCountV( formatString, marker );
    va_end(marker);
    return numCharactersWritten;
}

template<typename T> bool StringTrimLeft(T *dest)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    T *walk = dest;
    while (Locale::IsSpace(*walk))
    {
        ++walk;
    }

    if (walk != dest)
    {
        int len = StringLength(walk);
        memmove(dest, walk, (len + 1) * sizeof(T));
        return(true);
    }
    return(false);
}

template<typename T> bool StringTrimRight(T *dest)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    int len = StringLength(dest);
    T *walk = dest + len - 1;
    T *end = walk;
    while (walk >= dest && Locale::IsSpace(*walk))
    {
        *walk-- = 0;
    }
    return(walk != end);
}

template<typename T> bool StringTrimAll(T *dest)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    T *s = dest;
    T *d = dest;
        // trim left
    while (*s != 0)
    {
        if (!Locale::IsSpace(*s))
        {
            break;
        }
        ++s;
    }

        // trim multiple
    while (*s != 0)
    {
        if (!Locale::IsSpace(*s) || !Locale::IsSpace(*(s + 1)))
        {
            *d++ = *s;
        }
        ++s;
    }
    *d = 0;

        // trim right (at this point will be at most 1 character
    if (d > dest && Locale::IsSpace(*(d - 1)))
    {
        --d;
        *d = 0;
    }
    return(s != d);
}

template<typename T> bool StringStrip(T *dest, T c)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    T *s = dest;
    T *d = dest;
    while (*s != 0)
    {
        if (*s != c)
        {
            *d++ = *s;
        }
        ++s;
    }
    *d = 0;
    return(s != d);
}

template<typename T> bool StringToUpper(T *s)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    bool ret = false;
    T *w = s;
    while(*w != 0)
    {
        T hold = (T)Locale::ToUpper(*w);
        if (*w != hold)
        {
            *w = hold;
            ret = true;
        }
        w++;
    }
    return(ret);
}

template<typename T> bool StringToLower(T *s)
{
    StringEnforceType((T *)0);    // compiles to nothing, just validates T is of a supported type
    bool ret = false;
    T *w = s;
    while(*w != 0)
    {
        T hold = (T)Locale::ToLower(*w);
        if (*w != hold)
        {
            *w = hold;
            ret = true;
        }
        w++;
    }
    return(ret);
}

template<typename T> bool StringWildcardMatch(const T *source, const T *matchSpecification)
{
    while (*matchSpecification != 0)
    {
        if (*matchSpecification == '*')
        {
            matchSpecification++;
            if (*matchSpecification == 0)
            {
                return true;
            }

            while (*source != 0)
            {
                if (StringWildcardMatch(source, matchSpecification))
                {
                    return true;
                }
                source++;
            }

            return false;
        }
        else if (*matchSpecification == *source || (*matchSpecification == '?' && *source != 0))
        {
            matchSpecification++;
            source++;
        }
        else
        {
            return false;
        }
    }

    return (*source == 0);
}

template<typename T> bool StringWildcardMatchI(const T *source, const T *matchSpecification)
{
    while (*matchSpecification != 0)
    {
        if (*matchSpecification == '*')
        {
            matchSpecification++;
            if (*matchSpecification == 0)
            {
                return true;
            }

            while (*source != 0)
            {
                if (StringWildcardMatchI(source, matchSpecification))
                {
                    return true;
                }
                source++;
            }

            return false;
        }
        else if (Locale::ToUpper(*matchSpecification) == Locale::ToUpper(*source) || (*matchSpecification == '?' && *source != 0))
        {
            matchSpecification++;
            source++;
        }
        else
        {
            return false;
        }
    }

    return (*source == 0);
}

template<typename T> int StringHash(T *s)
{
    // Jenkins One-at-a-time hash function
    int hash = 0;
    while (*s != 0)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

template<typename T> int StringUpperHash(T *s)
{
    // Jenkins One-at-a-time hash function
    int hash = 0;
    while (*s != 0)
    {
        hash += Locale::ToUpper(*s);
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

template<typename T> int StringHash(const T *s, int maxLen)
{
    // Jenkins One-at-a-time hash function
    int hash = 0;
    while ((*s != 0) && (maxLen-- > 0))
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

template<typename T> int StringUpperHash(const T *s, int maxLen)
{
    // Jenkins One-at-a-time hash function
    int hash = 0;
    while ((*s != 0) && (maxLen-- > 0))
    {
        hash += Locale::ToUpper(*s);
        hash += (hash << 10);
        hash ^= (hash >> 6);
        s++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

SOEUTIL_FORCEINLINE int StringHashCompileOperation(int v, int c)
{
    // Jenkins One-at-a-time hash function
    v += c;
    v += (v << 10);
    v ^= (v >> 6);
    return(v);
}

template<typename T> SOEUTIL_FORCEINLINE int StringHashLiteral(T *s)
{
#ifdef SOEUTIL_DEBUG
    return StringHash(s);
#else
    // Jenkins One-at-a-time hash function (max of 48 letters in string will be processed, longer strings will end up calling StringHash)
    int v = 0;
    if (s[0] != 0)
    {
        v = StringHashCompileOperation(v, s[0]);
        if (s[1] != 0)
        {
            v = StringHashCompileOperation(v, s[1]);
            if (s[2] != 0)
            {
                v = StringHashCompileOperation(v, s[2]);
                if (s[3] != 0)
                {
                    v = StringHashCompileOperation(v, s[3]);
                    if (s[4] != 0)
                    {
                        v = StringHashCompileOperation(v, s[4]);
                        if (s[5] != 0)
                        {
                            v = StringHashCompileOperation(v, s[5]);
                            if (s[6] != 0)
                            {
                                v = StringHashCompileOperation(v, s[6]);
                                if (s[7] != 0)
                                {
                                    v = StringHashCompileOperation(v, s[7]);
                                    if (s[8] != 0)
                                    {
                                        v = StringHashCompileOperation(v, s[8]);
                                        if (s[9] != 0)
                                        {
                                            v = StringHashCompileOperation(v, s[9]);
                                            if (s[10] != 0)
                                            {
                                                v = StringHashCompileOperation(v, s[10]);
                                                if (s[11] != 0)
                                                {
                                                    v = StringHashCompileOperation(v, s[11]);
                                                    if (s[12] != 0)
                                                    {
                                                        v = StringHashCompileOperation(v, s[12]);
                                                        if (s[13] != 0)
                                                        {
                                                            v = StringHashCompileOperation(v, s[13]);
                                                            if (s[14] != 0)
                                                            {
                                                                v = StringHashCompileOperation(v, s[14]);
                                                                if (s[15] != 0)
                                                                {
                                                                    v = StringHashCompileOperation(v, s[15]);
                                                                    if (s[16] != 0)
                                                                    {
                                                                        v = StringHashCompileOperation(v, s[16]);
                                                                        if (s[17] != 0)
                                                                        {
                                                                            v = StringHashCompileOperation(v, s[17]);
                                                                            if (s[18] != 0)
                                                                            {
                                                                                v = StringHashCompileOperation(v, s[18]);
                                                                                if (s[19] != 0)
                                                                                {
                                                                                    v = StringHashCompileOperation(v, s[19]);
                                                                                    if (s[20] != 0)
                                                                                    {
                                                                                        v = StringHashCompileOperation(v, s[20]);
                                                                                        if (s[21] != 0)
                                                                                        {
                                                                                            v = StringHashCompileOperation(v, s[21]);
                                                                                            if (s[22] != 0)
                                                                                            {
                                                                                                v = StringHashCompileOperation(v, s[22]);
                                                                                                if (s[23] != 0)
                                                                                                {
                                                                                                    v = StringHashCompileOperation(v, s[23]);
                                                                                                    if (s[24] != 0)
                                                                                                    {
                                                                                                        v = StringHashCompileOperation(v, s[24]);
                                                                                                        if (s[25] != 0)
                                                                                                        {
                                                                                                            v = StringHashCompileOperation(v, s[25]);
                                                                                                            if (s[26] != 0)
                                                                                                            {
                                                                                                                v = StringHashCompileOperation(v, s[26]);
                                                                                                                if (s[27] != 0)
                                                                                                                {
                                                                                                                    v = StringHashCompileOperation(v, s[27]);
                                                                                                                    if (s[28] != 0)
                                                                                                                    {
                                                                                                                        v = StringHashCompileOperation(v, s[28]);
                                                                                                                        if (s[29] != 0)
                                                                                                                        {
                                                                                                                            v = StringHashCompileOperation(v, s[29]);
                                                                                                                            if (s[30] != 0)
                                                                                                                            {
                                                                                                                                v = StringHashCompileOperation(v, s[30]);
                                                                                                                                if (s[31] != 0)
                                                                                                                                {
                                                                                                                                    v = StringHashCompileOperation(v, s[31]);
                                                                                                                                    if (s[32] != 0)
                                                                                                                                    {
                                                                                                                                        v = StringHashCompileOperation(v, s[32]);
                                                                                                                                        if (s[33] != 0)
                                                                                                                                        {
                                                                                                                                            v = StringHashCompileOperation(v, s[33]);
                                                                                                                                            if (s[34] != 0)
                                                                                                                                            {
                                                                                                                                                v = StringHashCompileOperation(v, s[34]);
                                                                                                                                                if (s[35] != 0)
                                                                                                                                                {
                                                                                                                                                    v = StringHashCompileOperation(v, s[35]);
                                                                                                                                                    if (s[36] != 0)
                                                                                                                                                    {
                                                                                                                                                        v = StringHashCompileOperation(v, s[36]);
                                                                                                                                                        if (s[37] != 0)
                                                                                                                                                        {
                                                                                                                                                            v = StringHashCompileOperation(v, s[37]);
                                                                                                                                                            if (s[38] != 0)
                                                                                                                                                            {
                                                                                                                                                                v = StringHashCompileOperation(v, s[38]);
                                                                                                                                                                if (s[39] != 0)
                                                                                                                                                                {
                                                                                                                                                                    v = StringHashCompileOperation(v, s[39]);
                                                                                                                                                                    if (s[40] != 0)
                                                                                                                                                                    {
                                                                                                                                                                        v = StringHashCompileOperation(v, s[40]);
                                                                                                                                                                        if (s[41] != 0)
                                                                                                                                                                        {
                                                                                                                                                                            v = StringHashCompileOperation(v, s[41]);
                                                                                                                                                                            if (s[42] != 0)
                                                                                                                                                                            {
                                                                                                                                                                                v = StringHashCompileOperation(v, s[42]);
                                                                                                                                                                                if (s[43] != 0)
                                                                                                                                                                                {
                                                                                                                                                                                    v = StringHashCompileOperation(v, s[43]);
                                                                                                                                                                                    if (s[44] != 0)
                                                                                                                                                                                    {
                                                                                                                                                                                        v = StringHashCompileOperation(v, s[44]);
                                                                                                                                                                                        if (s[45] != 0)
                                                                                                                                                                                        {
                                                                                                                                                                                            v = StringHashCompileOperation(v, s[45]);
                                                                                                                                                                                            if (s[46] != 0)
                                                                                                                                                                                            {
                                                                                                                                                                                                v = StringHashCompileOperation(v, s[46]);
                                                                                                                                                                                                if (s[47] != 0)
                                                                                                                                                                                                {
                                                                                                                                                                                                    v = StringHashCompileOperation(v, s[47]);
                                                                                                                                                                                                    if (s[48] != 0)
                                                                                                                                                                                                    {
                                                                                                                                                                                                        v = StringHashCompileOperation(v, s[48]);
                                                                                                                                                                                                        if (s[49] != 0)
                                                                                                                                                                                                        {
                                                                                                                                                                                                            return(StringHash(s));
                                                                                                                                                                                                        }
                                                                                                                                                                                                    }
                                                                                                                                                                                                }
                                                                                                                                                                                            }
                                                                                                                                                                                        }
                                                                                                                                                                                    }
                                                                                                                                                                                }
                                                                                                                                                                            }
                                                                                                                                                                        }
                                                                                                                                                                    }
                                                                                                                                                                }
                                                                                                                                                            }
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                            }
                                                                                                                                        }
                                                                                                                                    }
                                                                                                                                }
                                                                                                                            }
                                                                                                                        }
                                                                                                                    }
                                                                                                                }
                                                                                                            }
                                                                                                        }
                                                                                                    }
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    v += (v << 3);
    v ^= (v >> 11);
    v += (v << 15);
    return(v);
#endif
}

#if 0       // unfortunately, "pig"[1] is not considered a constant by the compiler, so this doesn't work, but it came soooo close
// literal string hash (can be used in case statements, but requires letters of string be separated by commas)
#define SOEUTIL_ISH(v,c) ((c == 0) ? v : (((v+c) + ((v+c) << 10)) ^ (((v+c) + ((v+c) << 10)) >> 6)))
#define SOEUTIL_ISHPOST(v) (((v + (v << 3)) ^ ((v + (v << 3)) >> 11)) + (((v + (v << 3)) ^ ((v + (v << 3)) >> 11)) << 15))
#define SOEUTIL_STRINGHASH1(v) SOEUTIL_ISHPOST(SOEUTIL_ISH(0, #v[0]))
#define SOEUTIL_STRINGHASH2(v) SOEUTIL_ISHPOST(SOEUTIL_ISH(SOEUTIL_ISH(0, #v[0]), #v[1]))
#define SOEUTIL_STRINGHASH3(v) SOEUTIL_ISHPOST(SOEUTIL_ISH(SOEUTIL_ISH(SOEUTIL_ISH(0, #v[0]), #v[1]), #v[2]))
#define SOEUTIL_STRINGHASH4(v) SOEUTIL_ISHPOST(SOEUTIL_ISH(SOEUTIL_ISH(SOEUTIL_ISH(SOEUTIL_ISH(0, #v[0]), #v[1]), #v[2]), #v[3]))
#define SOEUTIL_STRINGHASH(v) (#v[0]==0) ? 0 : ((#v[1]==0) ? SOEUTIL_STRINGHASH1(##v) : ((#v[2]==0) ? SOEUTIL_STRINGHASH2(##v) : ((#v[3]==0) ? SOEUTIL_STRINGHASH3(##v) : ((#v[4]==0) ? SOEUTIL_STRINGHASH4(##v) : (-1)))))
#endif


}   // namespace SoeUtil


#endif
