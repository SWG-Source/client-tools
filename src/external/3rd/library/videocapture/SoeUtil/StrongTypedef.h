// Copyright 2007 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#ifndef SOEUTIL_STRONGTYPEDEF_H
#define SOEUTIL_STRONGTYPEDEF_H

namespace SoeUtil
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class/macro gives you roughly the equivalent of a strong typedef, which allows you to do things like create
// new types of integral values that can only be passed and assigned to others of the same type.  Usage is as follows:
//
//   SOEUTIL_STRONG_TYPEDEF(int, PlayerId);       // similar to a traditional typedef, only strongly typed
//   SOEUTIL_STRONG_TYPEDEF(int, UserId);
//
//   PlayerId myid(0);                            // works
//   UserId uid;                                  // works (value uninitialized)
//   PlayerId test = myid;                        // works
//   PlayerId test2 = PlayerId(123);              // works
//   int test3 = myid;                            // error, no implicit conversion to base type
//   PlayerId test4 = 123;                        // error, no implicit conversion from base type
//   PlayerId test5 = uid;                        // error, no conversion, different types
//   int test6 = myid.AsBase();                   // works
//   PlayerId test7 = PlayerId(uid.AsBase());     // works (probably a bug, but user was explicit so it compiles)
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> class StrongType
{
    public:
        typedef T BaseType;
        typedef StrongType<T, T_SIGNATURE, T_ZERO_INIT> SelfType;

        StrongType(const SelfType &source);
        StrongType();
        explicit StrongType(const T &value);

        SelfType &operator=(const SelfType &rhs);
        bool operator==(const SelfType &rhs) const;
        bool operator!=(const SelfType &rhs) const;
        bool operator<(const SelfType &rhs) const;
        bool operator<=(const SelfType &rhs) const;
        bool operator>(const SelfType &rhs) const;
        bool operator>=(const SelfType &rhs) const;

        T &AsBase();
        const T &AsBase() const;

    protected:
        T m_value;
};

template <typename T, typename T_SIGNATURE> class StrongType<T, T_SIGNATURE, true> : public StrongType<T, T_SIGNATURE, false>
{
    public:
        StrongType(const SelfType &source) : StrongType<T, T_SIGNATURE, false>(source) { }
        StrongType() : StrongType<T, T_SIGNATURE, false>(0) { }
        StrongType(const T &value) : StrongType<T, T_SIGNATURE, false>(value) { }
};



//////////////////////////////////////////////////////////////////////////////////////////
// inline implementations
//////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> StrongType<T, T_SIGNATURE, T_ZERO_INIT>::StrongType(const SelfType &source)
{
    m_value = source.m_value;
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> StrongType<T, T_SIGNATURE, T_ZERO_INIT>::StrongType()
{
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> StrongType<T, T_SIGNATURE, T_ZERO_INIT>::StrongType(const T &value)
{
    m_value = value;
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> typename StrongType<T, T_SIGNATURE, T_ZERO_INIT>::SelfType &StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator=(const SelfType &rhs)
{
    m_value = rhs.m_value;
    return(*this);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator==(const SelfType &rhs) const
{
    return(m_value == rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator!=(const SelfType &rhs) const
{
    return(m_value != rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator<(const SelfType &rhs) const
{
    return(m_value < rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator<=(const SelfType &rhs) const
{
    return(m_value <= rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator>(const SelfType &rhs) const
{
    return(m_value > rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> bool StrongType<T, T_SIGNATURE, T_ZERO_INIT>::operator>=(const SelfType &rhs) const
{
    return(m_value >= rhs.m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> T &StrongType<T, T_SIGNATURE, T_ZERO_INIT>::AsBase()
{
    return(m_value);
}

template <typename T, typename T_SIGNATURE, bool T_ZERO_INIT> const T &StrongType<T, T_SIGNATURE, T_ZERO_INIT>::AsBase() const
{
    return(m_value);
}


}   // namespace SoeUtil


#define SOEUTIL_STRONG_TYPEDEF(a, b) class SoeUtilSignature##b; typedef SoeUtil::StrongType<a, SoeUtilSignature##b, false> b
#define SOEUTIL_STRONG_TYPEDEF_ZERO(a, b) class SoeUtilSignature##b; typedef SoeUtil::StrongType<a, SoeUtilSignature##b, true> b

#endif