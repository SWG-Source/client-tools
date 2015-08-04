// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_STRING_H
#define SOEUTIL_STRING_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Specialized types of IString supported:
//
// class String;                // typical char string
// class StringFixed<size>;     // typical char string, with an embedded fixed size buffer
// class WideString;            // wchar_t string
// class WideStringFixed<size>; // wchar_t string, with an embedded fixed size buffer
//
// note: fixed variants turn into dynamic strings if they overflow the embedded buffer
// note: fixed variants are designed to be treated as-if there were non-fixed (ie. only distinguished at definition time)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <cstring>
#include "Types.h"
#include "StringUtil.h"
#include "Memory.h"

namespace SoeUtil
{

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // declarations/interface
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> class IString
{
    /////////////////////////////////////////////////////////////////
    // note: IString represents the interface used by all variants
    /////////////////////////////////////////////////////////////////

    public:
        // construction/destruction
        IString();
        IString(const IString<T> &source);
        IString(const T *source);
        virtual ~IString();

        // special function that allows you to set the string object to refer to an externally owned C string.
        // caller guarantees externalOwnedString will not change or be deleted during the String objects lifetime
        // note: len of -1 means to calculate length
        // note: if string is modified or made writable, a copy will be made
        // note: a direct String-to-String copy will not attempt to share a reference to this external string (this is a safety mechanism because app is only required to maintain string until String object they called SetExternalOwnership on has been deleted)
        void SetExternalOwnership(const T *externalOwnedString, int len = -1);

        // overloaded operators
        IString<T> &operator=(const IString<T> &other);
        IString<T> &operator=(const T *other);
        IString<T> &operator+=(const IString<T> &other);
        IString<T> &operator+=(const T *other);
        IString<T> &operator+=(T c);

        // note: in order to set these operators up as non-member functions, we would have to provide specializations of all of them
        // for StringFixed and WideStringFixed in all permutations, otherwise it would not be able to find the operator in all cases.
        // The reason for this is the compiler will not implicitly convert to a base class for the purpose of finding a matching operator overload.
        // The limitation is of course, that you can't do comparisons against a (char *) string unless it is on the right hand side.
        bool operator==(const IString<T> &s2) const;
        bool operator!=(const IString<T> &s2) const;
        bool operator<(const IString<T> &s2) const;
        bool operator<=(const IString<T> &s2) const;
        bool operator>(const IString<T> &s2) const;
        bool operator>=(const IString<T> &s2) const;
        bool operator==(const T *s2) const;
        bool operator!=(const T *s2) const;
        bool operator<(const T *s2) const;
        bool operator<=(const T *s2) const;
        bool operator>(const T *s2) const;
        bool operator>=(const T *s2) const;

        // basic manipulation
        const T *AsRead() const;        // gets a read-only pointer to string
        T *AsWrite(int *space = NULL);  // gets a writeable pointer to string (space = space available to write to)(must call FixupLength if length is changed -- not recommended, string is in an illegal state until FixedLength is called)(you lose buffer-safety nature of string class when you do this)
        int Length() const;             // returns length of string
        void Clear();                   // makes string empty (does not deallocate space it may have had allocated)
        void Reserve(int len);          // reserves enough space for a string of specified len
        void Compact();                 // minimize memory usage (use of this function is discouraged for performance and fragmentation reasons)
        void FixupLength();             // recalculates length of string (needed if length was changed via an AsWrite pointer -- not recommended, but useful if you are using existing code that wants a char* destination and you don't want to copy it to the stack first)
        T Get(int offset) const;        // returns character at specified offset (returns 0 for out of range)
        bool Set(int offset, T c);      // sets character at specified offset (returns false for out of range)(will make writable as needed)
        T GetFast(int offset) const;    // does no range checking, assumes offset is legal for performance (does assert in debug for out of range)
        void SetFast(int offset, T c);  // does no range checking, assumes offset is legal for performance (does assert in debug for out of range)(assumes it is writable, asserts if not)

        // string surgery
        void Insert(int dest, const T *source, int len = -1);       // len=-1=len of source string
        void Delete(int dest, int len = -1);                        // len=-1=remainder of string
        void Overwrite(int dest, const T *source, int len = -1);    // len=-1=len of source string (will expand string if needed)
        void Copy(const T *source);                                 // same as operator=
        void Copy(const IString<T> &source);                        // same as operator=
        void Copy(const T *source, int len);                        // set string to substring specified by source/len (asserts if source is not at least 'len' characters long)
        void CopyAndPreserveExternalOwnership(const IString<T> &source);  // same as operator=.  Preserving external ownership allows this object to inherit an externally owned string as externally owned to itself
        void Append(const T *source, int len);                      // appends len characters onto end of string (asserts if source is not at least 'len' characters long)
        void Append(const T *source);                               // same as operator+= (for a string)
        void Append(const T c);                                     // same as operator+= (for a single character)
        bool TrimLeft();                                            // deletes all leading whitespace
        bool TrimRight();                                           // deletes all trailing whitespace
        bool TrimAll();                                             // deletes all leading/trailing/multiple whitespace
        bool Strip(T c);                                            // deletes all instances of character 'c'
        int Format(const T *formatString, ...);                     // printf style formatting
        int FormatV(const T *formatString, va_list params);         // printf style formatting
        int AppendFormat(const T *formatString, ...);               // printf style formatting
        int AppendFormatV(const T *formatString, va_list params);   // printf style formatting

        // similar to printf style formatting, only it auto-picks the correct format string based on type of number
        // supports numeric and string types up to 8192 bytes in length.  Application can extend support to other types
        // by providing an appropriate StringFormatType specialization function (see StringUtil.h)
        template<typename T_VALUE> int FormatType(const T_VALUE &value);
        template<typename T_VALUE> int AppendFormatType(const T_VALUE &value);

        bool ToUpper();                                             // convert string to upper case
        bool ToLower();                                             // convert string to lower case


        // search/replace
        int Compare(const T *s2) const;                             // case sensitive compare
        int CompareI(const T *s2) const;                            // case insensitive compare
        int Compare(const T *s2, int maxLen) const;                 // case sensitive partial compare
        int CompareI(const T *s2, int maxLen) const;                // case insensitive partial compare
        int Find(const T *find) const;                              // returns offset or -1 if not found
        int FindI(const T *find) const;                             // returns offset or -1 if not found
        int FindRight(const T *find) const;                         // returns offset or -1 if not found
        int FindRightI(const T *find) const;                        // returns offset or -1 if not found
        bool Replace(const T *find, const T *replace);              // replaces all occurances of 'find' with 'replace'
        bool ReplaceI(const T *find, const T *replace);             // replaces all occurances of 'find' with 'replace' (case insensitive on find)
        int Find(int findChar) const;                               // returns offset or -1 if not found
        int FindRight(int findChar) const;                          // returns offset or -1 if not found

        // conversion operators (enabling not recommended as it can introduce hard to find subtle bugs for minimal convenience)
        #ifdef SOEUTIL_IMPLICIT_STRING_CONVERSION
        operator const T *() const;
        #endif

    protected:
        // functions/types
        typedef T RefType;     // making RefType same type as T will keep m_string aligned properly (especially in the case of wchar_t)

        // note: base implementation currently assumes that rawSpaceAllocated is greater than or equal to rawSpaceNeeded
        virtual byte *Alloc(int rawSpaceNeeded, int *rawSpaceAllocated, bool *shareable);
        virtual void Free(byte *data);

    private:
        void MakeWriteable(int spaceNeeded);
        void MakeWriteable_Internal(int spaceNeeded);
        int RefCount() const;
        void RefAdd();
        void RefRelease();
        bool RefIsShareable() const;


    private:
        // data
        T *m_string;           // points to actual string (there is a 'T' sized ref-count header on the allocation preceding this)
        int m_len;             // length of current string
        int m_space;           // amount of space available for the string (including 0 terminator character)
                               // note: if m_space is zero and m_string is not-NULL, then m_string represents a string constant owned elsewhere (set by SetExternalOwnership function)
};

template<typename T, int T_SIZE> class IStringFixed : public IString<T>
{
    public:
            // we really only need to override those virtual functions, but these don't inherit, so we must define them
        IStringFixed();
        ~IStringFixed();
        IStringFixed(const IStringFixed<T, T_SIZE> &source);
        IStringFixed(const T *source);
        IStringFixed(const IString<T> &source);
        IStringFixed<T, T_SIZE> &operator=(const IString<T> &other);
        IStringFixed<T, T_SIZE> &operator=(const IStringFixed<T, T_SIZE> &other);
        IStringFixed<T, T_SIZE> &operator=(const T *other);

    protected:
        virtual byte *Alloc(int rawSpaceNeeded, int *rawSpaceAllocated, bool *shareable);
        virtual void Free(byte *data);

    private:
        byte m_fixedData[(T_SIZE * sizeof(T)) + sizeof(RefType)];
};

//////////////////////////////////////////////////////////////////////////////////
// a version of the string object that won't share references internally
//////////////////////////////////////////////////////////////////////////////////
template<typename T> class IStringNoShare : public IString<T>
{
    public:
        IStringNoShare() : IString<T>() { }
        IStringNoShare(const IStringNoShare<T> &source) : IString<T>(source) { }
        IStringNoShare(const char *source) : IString<T>(source) { }
        IStringNoShare(const IString<T> &source) : IString<T>(source) { }

        IStringNoShare<T> &operator=(const IStringNoShare<T> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        IStringNoShare<T> &operator=(const IString<T> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        IStringNoShare<T> &operator=(const char *other)
        { 
            Copy(other); 
            return(*this); 
        }

    protected:
        virtual byte *Alloc(int rawSpaceNeeded, int *rawSpaceAllocated, bool *shareable)
        {
            bool dummy;
            *shareable = false;
            return IString<T>::Alloc(rawSpaceNeeded, rawSpaceAllocated, &dummy);
        }
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IString implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> IString<T>::IString()
{
    m_string = NULL;
    m_space = 0;
    m_len = 0;
}

template<typename T> IString<T>::IString(const IString<T> &source)
{
    m_string = NULL;
    m_space = 0;
    m_len = 0;

    Copy(source);
}

template<typename T> IString<T>::IString(const T *source)
{
    m_string = NULL;
    m_space = 0;
    m_len = 0;
    Copy(source);
}

template<typename T> IString<T>::~IString()
{
#ifdef SOEUTIL_DEBUG
    Clear();            // same as RefRelease only it also wipes the member-variables, something we normally would not need to do in release, but we do in Debug for extra safety
#else
    RefRelease();
#endif
}

template<typename T> void IString<T>::SetExternalOwnership(const T *externalOwnedString, int len)
{
    Clear();
    m_string = const_cast<T *>(externalOwnedString);
    if (len == -1)
    {
        m_len = externalOwnedString ? SoeUtil::StringLength(externalOwnedString) : 0;
    }
    else
    {
        m_len = len;
        SOEUTIL_ASSERT((externalOwnedString == NULL && len == 0) || m_len == SoeUtil::StringLength(externalOwnedString));        // you told me a len and it was wrong
    }
}

template<typename T> byte *IString<T>::Alloc(int rawSpaceNeeded, int *rawSpaceAllocated, bool *shareable)
{
    int allocSize = rawSpaceNeeded;
    if (m_space != 0)
    {
        // string previously existed and now wants more space, so implement a growth policy
        allocSize += (allocSize / 4);              // give it 25% extra room to grow
        allocSize = ((allocSize + 15) / 16) * 16;  // round to a multiple of 16 bytes
    }

    *shareable = true;
    *rawSpaceAllocated = allocSize;
    return((byte *)SoeUtil::Alloc(allocSize));
}

template<typename T> void IString<T>::Free(byte *data)
{
    SoeUtil::Free(data);
}

template<typename T> void IString<T>::MakeWriteable(int spaceNeeded)
{
        // if need more space or the current string needs to be copy-on-write
    if (spaceNeeded > m_space || RefCount() > 1)
    {
        MakeWriteable_Internal(spaceNeeded);        // bulk of work chained to another function to reduce inlining footprint of common case
    }
}

template<typename T> void IString<T>::MakeWriteable_Internal(int spaceNeeded)
{
        // we must make sure that the new space we are asking for is large enough to hold our existing string
        // the MakeWriteable function is not allowed to modify the existing string, so if it ends up having
        // to copy it, there must be at least this much space.  In practice, this does happen in some cases.  For
        // example, if you call Copy and the source string is shorter than the existing string, it will ask for less
        // writeable space than it already has.  Normally if there is already enough space, it wouldn't even make it into
        // here, but if there is more than 1 ref count, it may end up in here anyways.
    if (spaceNeeded < m_len + 1)
    {
        spaceNeeded = m_len + 1;
    }

        // allocate new space
    bool shareable;
    int rawSpace;
    int rawNeeded = (spaceNeeded * sizeof(T)) + sizeof(RefType);
    byte *data = Alloc(rawNeeded, &rawSpace, &shareable);
    SOEUTIL_MEMORY_INIT(data, rawSpace);
    *(RefType *)data = shareable ? 1 : 0;       // ref count of 1+ if shareable, 0 non-shareable
    T *newString = (T *)(data + sizeof(RefType));
    int newSpace = (rawSpace - sizeof(RefType)) / sizeof(T);
    int newLen = m_len;

        // copy old string into new space
        // note: technically there are cases where we know we are going to tromp the entirety of the new space with a new string
        // anyways, so this copy may not be necessary, but it will be fairly rare that you actually make it down into here in those
        // circumstances anyways, and the added complexity and overhead of passing in whether this copy can be avoided is probably
        // not worth it.
    std::memcpy(newString, this->AsRead(), (m_len + 1) * sizeof(T));

        // release reference to old string (sets us to the empty string too)
    RefRelease();

        // point members to new string
    m_string = newString;
    m_space = newSpace;
    m_len = newLen;
}

template<typename T> bool IString<T>::operator==(const IString<T> &s2) const
{
        // not implemented as a non-member because implicit conversions can't resolve anyways
    if (Length() == s2.Length())
    {
        return(std::memcmp(AsRead(), s2.AsRead(), Length() * sizeof(T)) == 0);
    }
    return(false);
}

template<typename T> bool IString<T>::operator!=(const IString<T> &s2) const
{
        // not implemented as a non-member because implicit conversions can't resolve anyways
    if (Length() == s2.Length())
    {
        return(std::memcmp(AsRead(), s2.AsRead(), Length() * sizeof(T)) != 0);
    }
    return(true);
}

template<typename T> bool IString<T>::operator<(const IString<T> &s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2.AsRead()) < 0);
}

template<typename T> bool IString<T>::operator<=(const IString<T> &s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2.AsRead()) <= 0);
}

template<typename T> bool IString<T>::operator>(const IString<T> &s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2.AsRead()) > 0);
}

template<typename T> bool IString<T>::operator>=(const IString<T> &s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2.AsRead()) >= 0);
}

template<typename T> bool IString<T>::operator==(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) == 0);
}

template<typename T> bool IString<T>::operator!=(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) != 0);
}

template<typename T> bool IString<T>::operator<(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) < 0);
}

template<typename T> bool IString<T>::operator<=(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) <= 0);
}

template<typename T> bool IString<T>::operator>(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) > 0);
}

template<typename T> bool IString<T>::operator>=(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2) >= 0);
}

template<typename T> IString<T> &IString<T>::operator=(const IString<T> &other)
{
    Copy(other);
    return(*this);
}

template<typename T> IString<T> &IString<T>::operator=(const T *other)
{
    Copy(other);
    return(*this);
}

template<typename T> IString<T> &IString<T>::operator+=(const IString<T> &other)
{
    int newLen = m_len + other.Length();
    if (newLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(newLen + 1);
        std::memmove(m_string + m_len, other.AsRead(), (other.Length() + 1) * sizeof(T));
        m_len = newLen;
    }
    return(*this);
}

template<typename T> IString<T> &IString<T>::operator+=(const T *other)
{
    Append(other);
    return(*this);
}

template<typename T> IString<T> &IString<T>::operator+=(T c)
{
    Append(c);
    return(*this);
}

#ifdef SOEUTIL_IMPLICIT_STRING_CONVERSION
    template<typename T> IString<T>::operator const T *() const
    {
        return(this->AsRead());
    }
#endif

template<typename T> T IString<T>::Get(int offset) const
{
    SOEUTIL_ASSERT(offset >= 0);
    if (offset < m_len)
    {
        return(m_string[offset]);       // can't be NULL if it made it in here
    }
    return(0);
}

template<typename T> bool IString<T>::Set(int offset, T c)
{
    SOEUTIL_ASSERT(offset >= 0);
    SOEUTIL_ASSERT(c != 0);              // not allowed to zero-terminate it manually like this
    if (offset < m_len)
    {
        MakeWriteable(m_len + 1);
        m_string[offset] = c;           // can't be NULL if it made it in here
        return(true);
    }
    return(false);
}

template<typename T> T IString<T>::GetFast(int offset) const
{
    SOEUTIL_ASSERT(offset >= 0);
    SOEUTIL_ASSERT(offset < m_len);      // out of range, even though you promised you were safe, how can I ever trust you again?
    return(m_string[offset]);           // can't be NULL (they promised us they were in-range)
}

template<typename T> void IString<T>::SetFast(int offset, T c)
{
    SOEUTIL_ASSERT(offset >= 0);
    SOEUTIL_ASSERT(c != 0);              // not allowed to zero-terminate it manually like this
    SOEUTIL_ASSERT(offset < m_len);      // out of range, even though you promised you were safe, how can I ever trust you again?
    SOEUTIL_ASSERT(RefCount() <= 1);     // SetFast requires that the string be writeable already (call AsWrite before using this functions)
    SOEUTIL_ASSERT(m_space != 0);        // can't be the const-exception string either
    m_string[offset] = c;                // can't be NULL (they promised us they were in-range)
}

template<typename T> int IString<T>::Length() const
{
    return(m_len);
}

template<typename T> const T *IString<T>::AsRead() const
{
    if (m_string == NULL)
    {
        static T s_emptyString = 0;
        return(&s_emptyString);
    }
    return(m_string);
}

template<typename T> T *IString<T>::AsWrite(int *space)
{
    MakeWriteable(m_len + 1);
    if (space != NULL)
    {
        *space = m_space;
    }
    return(m_string);
}

template<typename T> void IString<T>::Clear()
{
    RefRelease();
    m_string = NULL;
    m_space = 0;
    m_len = 0;
}

template<typename T> void IString<T>::Reserve(int len)
{
    len++;  // amount of space we reserve needs to be 1 byte longer than the len of string we are ask to be able to hold (to account for terminating zero)
    if (m_space < len)
    {
            // not enough space to meet reserve request, so ask for more space.
        MakeWriteable(len);
    }
}

template<typename T> void IString<T>::Compact()
{
        // do nothing for now...we don't support compacting yet - jeffp
}

template<typename T> void IString<T>::FixupLength()
{
    m_len = SoeUtil::StringLength(this->AsRead());
    SOEUTIL_ASSERT(m_space == 0 || m_len < m_space);     // overflowed buffer via AsWrite pointer at some point
}

template<typename T> void IString<T>::Insert(int dest, const T *source, int len)
{
    SOEUTIL_ASSERT(source != NULL);
    SOEUTIL_ASSERT(dest >= 0);           // negative destination illegal
    SOEUTIL_ASSERT(dest <= m_len);       // destination past end of current string illegal
    SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to copy from string that is a part of self (unless string is const exception)

    if (len == -1)
    {
        len = SoeUtil::StringLength(source);
    }
    else
    {
        SOEUTIL_ASSERT(StringLength(source, len) == len);         // 'source' string not as long as 'len' specified
    }

    int newLen = m_len + len;
    if (newLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(newLen + 1);
        std::memmove(m_string + dest + len, m_string + dest, (m_len - dest + 1) * sizeof(T));
        std::memcpy(m_string + dest, source, len * sizeof(T));
        m_len += len;
    }
}

template<typename T> void IString<T>::Delete(int dest, int len)
{
    SOEUTIL_ASSERT(dest >= 0);           // negative destination illegal
    SOEUTIL_ASSERT(dest <= m_len);       // destination past end of current string illegal

    if (len == -1)
    {
        MakeWriteable(dest + 1);
        m_len = dest;
        m_string[m_len] = 0;
    }
    else
    {
        SOEUTIL_ASSERT(dest + len <= m_len); // explicit length greater than actual string size

        MakeWriteable(m_len + 1);
        std::memmove(m_string + dest, m_string + (dest + len), (m_len - (dest + len) + 1) * sizeof(T));
        m_len -= len;
    }
}

template<typename T> void IString<T>::Overwrite(int dest, const T *source, int len)
{
    SOEUTIL_ASSERT(source != NULL);
    SOEUTIL_ASSERT(dest >= 0);           // negative destination illegal
    SOEUTIL_ASSERT(dest <= m_len);       // destination past end of current string illegal

    if (len == -1)
    {
        len = SoeUtil::StringLength(source);
    }
    else
    {
        SOEUTIL_ASSERT(StringLength(source, len) == len);         // 'source' string not as long as 'len' specified
    }
    
    SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to copy from string that is a part of self, since original string might be freed by the MakeWriteable call (or m_string is the const exception)
    int extent = dest + len;
    if (extent <= m_len)
    {
            // overwrite existing
        MakeWriteable(m_len + 1);       // need to call this in case string requires copy-on-write
        std::memcpy(m_string + dest, source, len * sizeof(T));
    }
    else
    {
            // overwrite extends past end of current string, ask for more space
        MakeWriteable(extent + 1);
        std::memcpy(m_string + dest, source, len * sizeof(T));
        m_string[extent] = 0;
        m_len = extent;
    }
}

template<typename T> void IString<T>::Copy(const T *source)
{
    if (source == NULL || *source == 0)
    {
        Clear();
    }
    else if (source != m_string || m_space == 0)     // do nothing if we are copying perfectly on top of ourself (and we are not a the special-const exception)
    {
        SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to copy from string that is a sub-part of self (unless string is the const exception)
        int len = SoeUtil::StringLength(source);
        MakeWriteable(len + 1);
        std::memcpy(m_string, source, (len + 1) * sizeof(T));
        m_len = len;
    }
}

template<typename T> void IString<T>::Copy(const T *source, int len)
{
    SOEUTIL_ASSERT(len >= 0);                                                 // negative not supported, use other Copy function to copy entire length of string
    SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to copy from string that is part of self (unless string is the const exception)
    SOEUTIL_ASSERT(StringLength(source, len) == len);                         // 'source' string not as long as 'len' specified
    if (len == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(len + 1);
        std::memcpy(m_string, source, len * sizeof(T));
        m_string[len] = 0;
        m_len = len;
    }
}

template<typename T> void IString<T>::Copy(const IString<T> &source)
{
    if (&source != this)
    {
        if (source.RefIsShareable())
        {
            // source shareable, so release current string and ref-count the sources copy
            RefRelease();
            m_string = source.m_string;
            m_len = source.m_len;
            m_space = source.m_space;
            RefAdd();
        }
        else
        {
            // source non-shareable
            if (source.m_len == 0)
            {
                // source is empty, so just set us to empty string
                Clear();
            }
            else
            {
                // make our own copy of source
                MakeWriteable(source.m_len + 1);
                m_len = source.m_len;
                std::memcpy(m_string, source.AsRead(), (m_len + 1) * sizeof(T));
            }
        }
    }
}

template<typename T> void IString<T>::CopyAndPreserveExternalOwnership(const IString<T> &source)
{
    if (source.m_space == 0 && source.m_len != 0)
    {
        // source contents are externally owned and we are being told we are allowed to inherit their
        // externally owned pointer, so make us refer to the same external data.  Copying in this way
        // requires the same contract as SetExternalOwnership requires, that is that the actual external string
        // must exist until we are destructed.
        SOEUTIL_ASSERT(source.m_string != NULL);        // source.m_len was non-zero, so source.m_string MUST be non-NULL
        m_string = source.m_string;
        m_len = source.m_len;
        m_space = source.m_space;
    }
    else
    {
        Copy(source);
    }
}

template<typename T> void IString<T>::Append(const T *source)
{
    SOEUTIL_ASSERT(source != NULL);
    SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to append from string that is a part of self (since making space might invalidate the original string)(unless string is the const exception)
    int olen = SoeUtil::StringLength(source);
    int newLen = m_len + olen;
    if (newLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(newLen + 1);
        std::memcpy(m_string + m_len, source, (olen + 1) * sizeof(T));
        m_len = newLen;
    }
}

template<typename T> void IString<T>::Append(const T *source, int len)
{
    SOEUTIL_ASSERT(source != NULL);
    SOEUTIL_ASSERT(len >= 0);                                                 // negative not supported
    SOEUTIL_ASSERT(m_space == 0 || source < m_string || source >= m_string + m_space);        // illegal to append from string that is a part of self (since making space might invalidate the original string)(unless string is the const exception)
    SOEUTIL_ASSERT(StringLength(source, len) == len);                         // 'source' string not as long as 'len' specified
    int newLen = m_len + len;
    if (newLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(newLen + 1);
        std::memcpy(m_string + m_len, source, len * sizeof(T));
        m_len = newLen;
        m_string[m_len] = 0;
    }
}

template<typename T> void IString<T>::Append(const T c)
{
    MakeWriteable(m_len + 2);
    m_string[m_len++] = c;
    m_string[m_len] = 0;
}

template<typename T> bool IString<T>::TrimLeft()
{
    bool ret = SoeUtil::StringTrimLeft(AsWrite());
    FixupLength();
    return(ret);
}

template<typename T> bool IString<T>::TrimRight()
{
    bool ret = SoeUtil::StringTrimRight(AsWrite());
    FixupLength();
    return(ret);
}

template<typename T> bool IString<T>::TrimAll()
{
    bool ret = SoeUtil::StringTrimAll(AsWrite());
    FixupLength();
    return(ret);
}

template<typename T> bool IString<T>::Strip(T c)
{
    bool ret = SoeUtil::StringStrip(AsWrite(), c);
    FixupLength();
    return(ret);
}

template<typename T> bool IString<T>::ToUpper()
{
    return(SoeUtil::StringToUpper(AsWrite()));
}

template<typename T> bool IString<T>::ToLower()
{
    return(SoeUtil::StringToLower(AsWrite()));
}

template<typename T> int IString<T>::FormatV(const T *formatString, va_list marker)
{
    int formatLen = SoeUtil::StringFormatCountV(formatString, marker);
    if (formatLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(formatLen + 1);
        SoeUtil::StringFormatV(m_string, formatLen + 1, formatString, marker);
        m_len = formatLen;
    }
    return(formatLen);
}

template<typename T> int IString<T>::Format(const T *formatString, ...)
{
    va_list marker;
    va_start(marker, formatString);
    int charactersWritten = FormatV(formatString, marker);
    va_end(marker);
    return(charactersWritten);
}

template<typename T> int IString<T>::AppendFormatV(const T *formatString, va_list marker)
{
    int formatLen = SoeUtil::StringFormatCountV(formatString, marker);
    int newLen = m_len + formatLen;
    if (newLen == 0)
    {
        Clear();
    }
    else
    {
        MakeWriteable(newLen + 1);
        SoeUtil::StringFormatV(m_string + m_len, formatLen + 1, formatString, marker);
        m_len = newLen;
    }
    return(formatLen);
}

template<typename T> int IString<T>::AppendFormat(const T *formatString, ...)
{
    va_list marker;
    va_start(marker, formatString);
    int charactersWritten = AppendFormatV(formatString, marker);
    va_end(marker);
    return(charactersWritten);
}

template<typename T> template<typename T_VALUE> int IString<T>::FormatType(const T_VALUE &value)
{
    ConvertType(value, this);
    return(Length());
}

template<typename T> template<typename T_VALUE> int IString<T>::AppendFormatType(const T_VALUE &value)
{
    IStringFixed<T, 8192> temp;
    ConvertType(value, (IString<T> *)&temp);
    Append(temp.AsRead());
    return(temp.Length());
}

template<typename T> int IString<T>::Compare(const T *s2) const
{
    return(SoeUtil::StringCompare(AsRead(), s2));
}

template<typename T> int IString<T>::CompareI(const T *s2) const
{
    return(SoeUtil::StringCompareI(AsRead(), s2));
}

template<typename T> int IString<T>::Compare(const T *s2, int maxLen) const
{
    return(SoeUtil::StringCompare(AsRead(), s2, maxLen));
}

template<typename T> int IString<T>::CompareI(const T *s2, int maxLen) const
{
    return(SoeUtil::StringCompareI(AsRead(), s2, maxLen));
}

template<typename T> int IString<T>::Find(const T *find) const
{
    const T *ptr = SoeUtil::StringFind(AsRead(), find);
    if (ptr != NULL)
    {
        return((int)(ptr - AsRead()));
    }
    return(-1);
}

template<typename T> int IString<T>::FindI(const T *find) const
{
    const T *ptr = SoeUtil::StringFindI(AsRead(), find);
    if (ptr != NULL)
    {
        return((int)(ptr - AsRead()));
    }
    return(-1);
}

template<typename T> int IString<T>::FindRight(const T *find) const
{
    const T *ptr = SoeUtil::StringFindRight(AsRead(), find);
    if (ptr != NULL)
    {
        return((int)(ptr - AsRead()));
    }
    return(-1);
}

template<typename T> int IString<T>::FindRightI(const T *find) const
{
    const T *ptr = SoeUtil::StringFindRightI(AsRead(), find);
    if (ptr != NULL)
    {
        return((int)(ptr - AsRead()));
    }
    return(-1);
}

template<typename T> int IString<T>::Find(int findChar) const
{
    const T *ptr = StringFind(AsRead(), findChar);
    if (ptr != NULL)
    {
        return((int)(ptr - AsRead()));
    }
    return(-1);
}

template<typename T> int IString<T>::FindRight(int findChar) const
{
    const T *start = AsRead();
    const T *walk = start + Length();
    while (walk != start && (int)*walk != findChar)
    {
        walk--;
    }
    return(((int)*walk != findChar) ? -1 : (int)(walk - start));
}

template<typename T> bool IString<T>::Replace(const T *find, const T *replace)
{
    int flen = SoeUtil::StringLength(find);
    SOEUTIL_ASSERT(flen > 0);       // if you find empty string and replace it, you will loop forever
    int rlen = SoeUtil::StringLength(replace);

    bool ret = false;
    const T *walk = AsRead();
    for (;;)
    {
        walk = SoeUtil::StringFind(walk, find);
        if (walk == NULL)
        {
            break;
        }

        ret = true;
        int offset = (int)(walk - AsRead());
        Delete(offset, flen);
        Insert(offset, replace, rlen);
        walk = AsRead() + offset + rlen;
    }
    return(ret);
}

template<typename T> bool IString<T>::ReplaceI(const T *find, const T *replace)
{
    int flen = SoeUtil::StringLength(find);
    SOEUTIL_ASSERT(flen > 0);       // if you find empty string and replace it, you will loop forever
    int rlen = SoeUtil::StringLength(replace);

    bool ret = false;
    const T *walk = AsRead();
    for (;;)
    {
        walk = SoeUtil::StringFindI(walk, find);
        if (walk == NULL)
        {
            break;
        }

        ret = true;
        int offset = (int)(walk - AsRead());
        Delete(offset, flen);
        Insert(offset, replace, rlen);
        walk = AsRead() + offset + rlen;
    }
    return(ret);
}

template<typename T> bool IString<T>::RefIsShareable() const
{
        // refcount of 0 mean unshareable.  Once a string has too many refs, it becomes unshareable as well.
    return(RefCount() != 0 && RefCount() < 127);        // will share a string a maximum of 127 times (technically speaking, this could be max-value of RefType, but this will handle all cases well enough)
}

template<typename T> int IString<T>::RefCount() const
{
    if (m_string == NULL || m_space == 0)       // empty string and const-exception string are both no-ref-count, which makes them unsharable
    {
        return(0);
    }
    byte *data = (byte *)m_string - sizeof(RefType);
    return((int)*(RefType *)data);
}

template<typename T> void IString<T>::RefAdd()
{
    SOEUTIL_ASSERT(m_string != NULL);        // should not be possible to RefAdd when string is NULL
    SOEUTIL_ASSERT(m_space != 0);            // should not be possible to RefAdd when string is the const exception
    byte *data = (byte *)m_string - sizeof(RefType);
    SOEUTIL_ASSERT(*(RefType *)data != 0);       // make sure we don't somehow try reffing something that is marked unshareable
    *(RefType *)data += 1;
}

template<typename T> void IString<T>::RefRelease()
{
    // note: RefRelease call MUST be immediately followed by a resetting of the m_string, m_len, and m_space members

    // if we are not a NULL string and we are not the const-exception
    if (m_string != NULL && m_space != 0)       
    {
        // then decrement our ref count (or free us)
        byte *data = (byte *)m_string - sizeof(RefType);
        if (*(RefType *)data <= 1)
        {
            SOEUTIL_MEMORY_INVALIDATE(data, (m_space * sizeof(T)) + sizeof(RefType));
            Free(data);
        }
        else
        {
            *(RefType *)data -= 1;
        }
    }
}


    ///////////////////////////////////////////////////////////
    // IStringFixed implementation
    ///////////////////////////////////////////////////////////
template<typename T, int T_SIZE> IStringFixed<T, T_SIZE>::IStringFixed() : IString<T>()
{
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE>::~IStringFixed()
{
    Clear();
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE>::IStringFixed(const IString<T> &source) : IString<T>()
{
        // must default construct first, then assign, otherwise the Alloc virtual function won't be mapped to the derived class yet
    Copy(source);
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE>::IStringFixed(const T *source) : IString<T>()
{
        // must default construct first, then assign, otherwise the Alloc virtual function won't be mapped to the derived class yet
    Copy(source);
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE>::IStringFixed(const IStringFixed<T, T_SIZE> &source) : IString<T>()
{
    Copy(source);
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE> &IStringFixed<T, T_SIZE>::operator=(const IStringFixed<T, T_SIZE> &other)
{
    Copy(other);
}

template<typename T, int T_SIZE> IStringFixed<T, T_SIZE> &IStringFixed<T, T_SIZE>::operator=(const IString<T> &other)
{
    Copy(other);
}

template<typename T, int T_SIZE> byte *IStringFixed<T, T_SIZE>::Alloc(int rawSpaceNeeded, int *rawSpaceAllocated, bool *shareable)
{
    if (rawSpaceNeeded <= sizeof(m_fixedData))
    {
        *shareable = false;
        *rawSpaceAllocated = sizeof(m_fixedData);
        return(m_fixedData);
    }
    return(IString<T>::Alloc(rawSpaceNeeded, rawSpaceAllocated, shareable));
}

template<typename T, int T_SIZE> void IStringFixed<T, T_SIZE>::Free(byte *data)
{
    if (data != m_fixedData)
    {
        IString<T>::Free(data);
    }
}


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // typedef aliases (and some trickery due to language limitation on making templatized typedefs)
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef IString<char> String;
typedef IString<wchar_t> WideString;
typedef IStringNoShare<char> StringNoShare;
typedef IStringNoShare<wchar_t> WideStringNoShare;

template<int T_SIZE> class StringFixed : public IStringFixed<char, T_SIZE>
{
    public:
        StringFixed() : IStringFixed<char, T_SIZE>() { }
        StringFixed(const StringFixed<T_SIZE> &source) : IStringFixed<char, T_SIZE>(source) { }
        StringFixed(const char *source) : IStringFixed<char, T_SIZE>(source) { }
        StringFixed(const IString<char> &source) : IStringFixed<char, T_SIZE>(source) { }

        StringFixed<T_SIZE> &operator=(const StringFixed<T_SIZE> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        StringFixed<T_SIZE> &operator=(const IString<char> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        StringFixed<T_SIZE> &operator=(const char *other)
        { 
            Copy(other); 
            return(*this); 
        }
};

template<int T_SIZE> class WideStringFixed : public IStringFixed<wchar_t, T_SIZE>
{
    public:
        WideStringFixed() : IStringFixed<wchar_t, T_SIZE>() { }
        WideStringFixed(const WideStringFixed<T_SIZE> &source) : IStringFixed<wchar_t, T_SIZE>(source) { }
        WideStringFixed(const wchar_t *source) : IStringFixed<wchar_t, T_SIZE>(source) { }
        WideStringFixed(const IString<wchar_t> &source) : IStringFixed<wchar_t, T_SIZE>(source) { }

        WideStringFixed<T_SIZE> &operator=(const WideStringFixed<T_SIZE> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        WideStringFixed<T_SIZE> &operator=(const IString<wchar_t> &other)
        { 
            Copy(other); 
            return(*this); 
        }

        WideStringFixed<T_SIZE> &operator=(const wchar_t *other)
        { 
            Copy(other); 
            return(*this); 
        }
};


////////////////////////////////////////////////////////////////////////////
// certain util functions need to be in here as StringUtil.h can't see String.h
////////////////////////////////////////////////////////////////////////////

template<typename T_VALUE> int StringFormatType(char *dest, int destLen, const T_VALUE &value)
{
    StringFixed<512> temp;
    ConvertType(value, &temp);
    return StringFormat(dest, destLen, "%s", temp.AsRead());
}

template<typename T_VALUE> int StringFormatType(wchar_t *dest, int destLen, const T_VALUE &value)
{
    WideStringFixed<512> temp;
    ConvertType(value, &temp);
    return StringFormat(dest, destLen, L"%s", temp.AsRead());
}


}    // namespace SoeUtil


#endif    // SOEUTIL_STRING_H

