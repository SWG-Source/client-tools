// ======================================================================
//
// UIStlFwd.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef __UIStlFwd_H__
#define __UIStlFwd_H__

// ======================================================================
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace std
{
	template <class _Tp>                                                                struct less;
	template <class _Tp>                                                                struct equal_to;
	template <class _T1, class _T2>                                                     struct pair;
	template <class _T1>                                                                struct hash;
	template <class _Tp>                                                                class  allocator;
	template <class _CharT, class _Traits, class _Alloc>                                class  basic_string;
	template <class _CharT>                                                             struct  char_traits;

	template <size_t _Nb>                                                               class  bitset;
	template <class _Tp, class _Alloc>                                                  class  deque;
	template <class _Tp, class _Alloc>                                                  class  list;
	template <class _Tp, class _Alloc>                                                  class  queue;
	template <class _Tp, class _Alloc>                                                  class  vector;
	template <class _Tp, class _Sequence>                                               class  stack;
	template <class _Tp, class _Container, class _Compare>                              class  priority_queue;
	template <class _Key, class _Tp, class _Compare, class _Alloc>                      class  map;
	template <class _Key, class _Tp, class _HashFcn, class _Compare, class _Alloc>      class  hash_map;
	template <class _Key, class _Tp, class _Compare, class _Alloc>                      class  multimap;
	template <class _Key, class _Compare, class _Alloc>                                 class  set;
	template <class _Key, class _HashFcn, class _Compare, class _Alloc>                 class  hash_set;
	template <class _Key, class _Compare, class _Alloc>                                 class  multiset;

	typedef basic_string<char, char_traits<char>, allocator<char> >                     string;
}

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct ui_stddeque
{
	typedef std::deque<_Tp, _Alloc> fwd;
};

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct ui_stdlist
{
	typedef std::list<_Tp, _Alloc> fwd;
};

template <class _Key, class _Tp, class _Compare = std::less<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct ui_stdmap
{
	typedef std::map<_Key, _Tp, _Compare, _Alloc> fwd;
};

template <
	typename Key,
	typename Value,
	typename Hash = std::hash<Key>,
	typename Equal = std::equal_to<Key>,
	typename Alloc = std::allocator<std::pair<const Key, Value>>
>
using ui_stdhash_map = std::unordered_map<Key, Value, Hash, Equal, Alloc>;

template <class _Key, class _Tp, class _Compare = std::less<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct ui_stdmultimap
{
	typedef std::multimap<_Key, _Tp, _Compare, _Alloc> fwd;
};

template <class _Key, class _Compare = std::less<_Key>, class _Alloc = std::allocator<_Key> > struct ui_stdset
{
	typedef std::set<_Key, _Compare, _Alloc> fwd;
};

template <
	typename Key,
	typename Hash = std::hash<Key>,
	typename Equal = std::equal_to<Key>,
	typename Alloc = std::allocator<Key>
>
using ui_stdhash_set = std::unordered_set<Key, Hash, Equal, Alloc>;

template <class _Key, class _Compare = std::less<_Key>, class _Alloc = std::allocator<_Key> > struct ui_stdmultiset
{
	typedef std::multiset<_Key, _Compare, _Alloc> fwd;
};

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct ui_stdvector
{
	typedef std::vector<_Tp, _Alloc> fwd;
};

template <class _Tp, class _Container = ui_stdvector<_Tp>::fwd, class _Compare = std::less<typename _Container::value_type> > struct ui_stdpriority_queue
{
	typedef std::priority_queue<_Tp, _Container, _Compare> fwd;
};

template <class _Tp, class _Sequence = ui_stddeque<_Tp>::fwd > struct ui_stdqueue
{
	typedef std::queue<_Tp, _Sequence> fwd;
};

template <class _Tp, class _Sequence = ui_stddeque<_Tp>::fwd > struct ui_stdstack
{
	typedef std::stack<_Tp, _Sequence> fwd;
};

namespace Unicode
{
	using unicode_char_t = char16_t;
	using String = std::u16string;
}

// ======================================================================

#endif
