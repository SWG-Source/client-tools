#ifndef	_Archive_H
#define	_Archive_H

//---------------------------------------------------------------------

#include "ByteStream.h"
#include <string>
#include <map>
#include <deque>
#include <set>

//---------------------------------------------------------------------

namespace Archive {
	
//---------------------------------------------------------------------
	

inline void get(ReadIterator & source, double & target)
{
	source.get(&target, 8);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, float & target)
{
	source.get(&target, 4);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, unsigned long int & target)
{
	source.get(&target, 4);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, signed long int & target)
{
	source.get(&target, 4);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, unsigned int & target)
{
	source.get(&target, 4);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, signed int & target)
{
	source.get(&target, 4);
}

//---------------------------------------------------------------------
// 64-bit overloads. On MSVC `unsigned long long` and `unsigned __int64`
// (the engine's `uint64` typedef) are the same type, and on x64 `size_t`
// is also `unsigned __int64` - so this single overload covers all three
// and makes AutoDeltaMap/Set/Vector etc. compile on x64. NetworkIdArchive
// used to define these out-of-line for `uint64`/`int64`; those were
// removed to avoid a duplicate-symbol clash at link time.

inline void get(ReadIterator & source, unsigned long long & target)
{
	source.get(&target, 8);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, signed long long & target)
{
	source.get(&target, 8);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, unsigned short int & target)
{
	source.get(&target, 2);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, signed short int & target)
{
	source.get(&target, 2);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, unsigned char & target)
{
	source.get(&target, 1);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, signed char & target)
{
	source.get(&target, 1);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, std::string & target)
{
	unsigned short len;
	unsigned int size;
	get(source, len);
	if (len < 65535)
		size = len;
	else
		get(source, size);
	// Sanity-bail: claimed size exceeds remaining buffer. Without this,
	// std::string(c, size) memcpy's past valid memory and AVs.
	if (size > source.getSize())
	{
		throw ReadException("std::string get: size exceeds remaining buffer");
	}
	const char * c = reinterpret_cast<const char * const>(source.getBuffer());
	target = std::string(c, size);
	source.advance(size);
}

//---------------------------------------------------------------------

inline void get(ReadIterator & source, bool & target)
{
	source.get(&target, 1);
}

//-----------------------------------------------------------------------

inline void get(ReadIterator & source, ByteStream & target)
{
	unsigned int s;
	get(source, s);
	// ReadIterator::getSize() already returns the remaining (unread) bytes.
	const unsigned int remaining = source.getSize();
	if (s > remaining)
	{
		// Sanity-bail: claimed size exceeds remaining buffer, almost
		// certainly malformed input. Throwing ReadException unwinds the
		// AutoByteStream::unpack chain so the message gets dropped instead
		// of allocating garbage and corrupting the heap.
		throw ReadException("ByteStream get: size exceeds remaining buffer");
	}
	target.put(source.getBuffer(), s);
	source.advance(s);
}

//----------------------------------------------------------------------

template<typename A, typename B> inline void get(ReadIterator & source, std::pair<A,B> & target)
{
	get(source, target.first);
	get(source, target.second);
}

//---------------------------------------------------------------------

template<typename A> inline void get(ReadIterator & source, std::vector<A>& target)
{
	target.clear();
	signed int length = 0;
	source.get(&length, 4);
	A temp;
	for(int i = 0; i < length; ++i)
	{
		get(source, temp);
		target.push_back(temp);
	}
}

//-----------------------------------------------------------------------

template<typename A> inline void get(ReadIterator & source, std::set<A>& target)
{
	target.clear();
	signed int length = 0;
	source.get(&length, 4);
	A temp;
	for(int i = 0; i < length; ++i)
	{
		get(source, temp);
		target.insert(temp);
	}
}

//-----------------------------------------------------------------------

template<typename A> inline void get(ReadIterator & source, std::deque<A>& target)
{
	target.clear();
	signed int length = 0;
	source.get(&length, 4);
	A temp;
	for(int i = 0; i < length; ++i)
	{
		get(source, temp);
		target.push_back(temp);
	}
}

//-----------------------------------------------------------------------

template<typename A> inline void get_ptr(ReadIterator & source, std::vector<const A *>& target)
{
	target.clear();
	signed int length = 0;
	source.get(&length, 4);
//	A temp;
	for(int i = 0; i < length; ++i)
	{
		A * temp = new A;
		get(source, temp);
		target.push_back(temp);
	}
}

//-----------------------------------------------------------------------

template<typename Key, typename Value> inline void get(ReadIterator & source, std::map<Key, Value> & target)
{
	// Wire format is a 4-byte length, matching vector/set/deque. Using `int`
	// here (instead of size_t) keeps the get/put overload selection unambiguous
	// on x64, where size_t is 8 bytes and would need a separate uint64 overload.
	int numKeys;
	get(source, numKeys);
	for(int i = 0; i < numKeys; ++i)
	{
		Key k;
		get(source, k);
		Value v;
		get(source, v);
		target[k] = v;
	}
}

//---------------------------------------------------------------------

template<typename A> inline void get(ReadIterator & source, A * target, int length)
{
	for(int i = 0; i < length; ++i)
	{
		get(source, target[i]);
	}
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const double & source)
{
	target.put(&source, 8);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const float & source)
{
	target.put(&source, 4);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const unsigned long int & source)
{
	target.put(&source, 4);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const signed long int & source)
{
	target.put(&source, 4);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const unsigned int & source)
{
	target.put(&source, 4);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const signed int & source)
{
	target.put(&source, 4);
}

//---------------------------------------------------------------------
// 64-bit put overloads - matched pair to the get() side above.

inline void put(ByteStream & target, const unsigned long long & source)
{
	target.put(&source, 8);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const signed long long & source)
{
	target.put(&source, 8);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const unsigned short int & source)
{
	target.put(&source, 2);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const signed short int & source)
{
	target.put(&source, 2);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const unsigned char & source)
{
	target.put(&source, 1);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const signed char & source)
{
	target.put(&source, 1);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const char & source)
{
	target.put(&source, 1);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const std::string & source)
{
	if (source.size() < 65535)
	{
		unsigned short len = static_cast<unsigned short>(source.size());
		put(target, len);
	}
	else
	{
		unsigned short len = static_cast<unsigned short>(65535);
		put(target, len);
		unsigned int size = source.size();
		put(target, size);
	}
	target.put(source.data(), source.size());
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, const bool & source)
{
	target.put(&source, 1);
}

//---------------------------------------------------------------------

inline void put(ByteStream & target, Archive::ReadIterator & source)
{
	put(target, source.getSize());
	target.put(source.getBuffer(), source.getSize());
	source.advance(source.getSize());
}

//-----------------------------------------------------------------------

inline void put(ByteStream & target, const ByteStream & source)
{
	put(target, source.begin().getSize());
	target.put(source.begin().getBuffer(), source.begin().getSize());
}

//----------------------------------------------------------------------

template<typename A, typename B> inline void put(ByteStream & target, const std::pair<A,B> & source)
{
	put(target, source.first);
	put(target, source.second);
}

//---------------------------------------------------------------------

template<typename A> inline void put(ByteStream & target, const std::vector<A> & source)
{
	signed int length = source.size();
	target.put(&length, 4);
	for(int i = 0; i < length; ++i)
	{
		put(target, source[i]);
	}
}

//-----------------------------------------------------------------------

template<typename A> inline void put(ByteStream & target, const std::set<A> & source)
{
	signed int length = source.size();
	target.put(&length, 4);
	for (typename std::set<A>::const_iterator i = source.begin(); i != source.end(); ++i)
		put(target, *i);
}

//-----------------------------------------------------------------------

template<typename A> inline void put(ByteStream & target, const std::deque<A> & source)
{
	signed int length = source.size();
	target.put(&length, 4);
	for(int i = 0; i < length; ++i)
	{
		put(target, source[i]);
	}
}

//-----------------------------------------------------------------------

template<typename Key, typename Value> inline void put(ByteStream & target, const std::map<Key, Value> & source)
{
	int numKeys = static_cast<int>(source.size());
	put(target, numKeys);
	for (typename std::map<Key, Value>::const_iterator i = source.begin(); i != source.end(); ++i)
	{
		put(target, i->first);
		put(target, i->second);
	}
}

//---------------------------------------------------------------------

template<typename A> inline void put(ByteStream & target, const A * source, int length)
{
	for(int i = 0; i < length; ++i)
	{
		put(target, source[i]);
	}
}

//---------------------------------------------------------------------

}//namespace Archive

//---------------------------------------------------------------------

#endif	// _Archive_H
