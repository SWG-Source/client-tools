// ======================================================================
//
// UIStlFwd.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef __UIStlFwd_H__
#define __UIStlFwd_H__

#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>

// ======================================================================

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

template <class _Key, class _Tp, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct ui_stdunordered_map
{
	typedef std::unordered_map<_Key, _Tp, _HashFcn, _Compare, _Alloc> fwd;
};

template <class _Key, class _Tp, class _Compare = std::less<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct ui_stdmultimap
{
	typedef std::multimap<_Key, _Tp, _Compare, _Alloc> fwd;
};

template <class _Key, class _Compare = std::less<_Key>, class _Alloc = std::allocator<_Key> > struct ui_stdset
{
	typedef std::set<_Key, _Compare, _Alloc> fwd;
};

template <class _Key, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator<_Key> > struct ui_stdunordered_set
{
	typedef std::unordered_set<_Key, _HashFcn, _Compare, _Alloc> fwd;
};

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
	typedef unsigned short unicode_char_t;
	typedef std::basic_string<unicode_char_t, std::char_traits<unicode_char_t>, std::allocator<unicode_char_t> > String;
}

// ======================================================================

#endif
