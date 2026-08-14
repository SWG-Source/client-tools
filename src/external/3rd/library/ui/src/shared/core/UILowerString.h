//======================================================================
//
// UILowerString.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UILowerString_H
#define INCLUDED_UILowerString_H

//======================================================================
#include <unordered_map>

class UILowerString
{
	friend class UILowerStringCacheMapIntializer;

public:
	UILowerString();
	UILowerString(UILowerString const & rhs);


	explicit UILowerString(const char * const str);
	explicit UILowerString(const std::string & str);

	bool operator<(const UILowerString & rhs) const;
	bool operator==(const UILowerString & rhs) const;
	bool operator!=(const UILowerString & rhs) const;
	UILowerString const & operator=(const UILowerString & rhs);

	static const UILowerString null;
	static const std::string nullstring;
	
	bool empty() const;

	bool const startsWith(const char c) const;
	bool const equals(const char * const str, size_t n) const;
	std::string const & get() const;
	char const * const c_str() const;
	inline size_t const getHash() const { return m_hashEqu; }

	void set(const std::string & str);
	void updateHash(char const * const str);

private:

#ifdef _DEBUG
	std::string m_debugStr;
#endif

	size_t m_hashQuick;
	size_t m_hashEqu;

	typedef std::unordered_map<size_t, std::string> UILowerStringHashMap;
	static UILowerStringHashMap* ms_hashMap;
};

static class UILowerStringCacheMapIntializer
{
public:
	UILowerStringCacheMapIntializer();
	~UILowerStringCacheMapIntializer();
private:
	static int refCount;
} CacheMapInitializer;


//----------------------------------------------------------------------

inline UILowerString::UILowerString() :
#ifdef _DEBUG
m_debugStr(),
#endif
m_hashQuick(0),
m_hashEqu(0)
{
}

//----------------------------------------------------------------------

inline UILowerString::UILowerString(const char * const str) :
#ifdef _DEBUG
m_debugStr(),
#endif
m_hashQuick(0),
m_hashEqu(0)
{
	updateHash(str);
}

//----------------------------------------------------------------------
inline UILowerString::UILowerString(UILowerString const & rhs) :
#ifdef _DEBUG
m_debugStr(),
#endif
m_hashQuick(rhs.m_hashQuick),
m_hashEqu(rhs.m_hashEqu)
{
#ifdef _DEBUG
	m_debugStr = rhs.m_debugStr;
#endif
}

//----------------------------------------------------------------------

inline UILowerString::UILowerString(std::string const & str) :
#ifdef _DEBUG
m_debugStr(),
#endif
m_hashQuick(0),
m_hashEqu(0)
{
	updateHash(str.c_str());
}

//----------------------------------------------------------------------

inline bool UILowerString::operator<(UILowerString const & rhs) const
{
	if (m_hashQuick < rhs.m_hashQuick) 
	{
		return true;
	}
	else if (m_hashQuick > rhs.m_hashQuick)
	{
		return false;
	}
	return (m_hashEqu != rhs.m_hashEqu) && (_stricmp(get().c_str(), rhs.get().c_str()) < 0);
}

//----------------------------------------------------------------------

inline bool UILowerString::operator==(UILowerString const & rhs) const
{
	return m_hashQuick == rhs.m_hashQuick && m_hashEqu == rhs.m_hashEqu;
}

//----------------------------------------------------------------------

inline bool UILowerString::operator!=(UILowerString const & rhs) const
{
	return !(*this == rhs);
}

//----------------------------------------------------------------------

inline UILowerString const & UILowerString::operator=(UILowerString const & rhs)
{
	if (this != &rhs) 
	{
		m_hashQuick = rhs.m_hashQuick;
		m_hashEqu = rhs.m_hashEqu;
		
#ifdef _DEBUG
		m_debugStr = rhs.m_debugStr;
#endif
	}

	return *this;
}

//----------------------------------------------------------------------

inline bool UILowerString::empty() const
{
	return !m_hashQuick;
}

//----------------------------------------------------------------------

inline const bool UILowerString::startsWith(char const c) const 
{ 
	return (!c && !m_hashQuick) || (m_hashQuick > 0 && tolower(get()[0]) == tolower(c)); 
}

//----------------------------------------------------------------------

inline char const * const UILowerString::c_str() const 
{ 
	return get().c_str(); 
}

//----------------------------------------------------------------------

inline bool const UILowerString::equals(const char * const str, size_t n) const 
{
	return (!m_hashQuick && (!str || !*str)) || !_strnicmp(get().c_str(), str, n); 
}


//======================================================================

#endif
