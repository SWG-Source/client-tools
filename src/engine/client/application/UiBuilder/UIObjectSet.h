#ifndef INCLUDED_UIObjectSet_H
#define INCLUDED_UIObjectSet_H

#include "UITypes.h"

#include <list>
#include <map>

#pragma warning(disable:4284)

class UIBaseObject;

// =========================================================================
// UIObjectSet is a custom container designed to support fast (logN) insert,
// find, and erase operations while also allowing iteration IN THE ORDER
// THAT THE OBJECTS WERE ADDED. (insert and push_back perform roughly the
// same operation).
//
// This container was created for two reasons:
// 1) Often times the order of a user's selections is important.  However,
// the containers holding the selections need to support 'find' and 'erase'.
// 2) Often times a simple set of objects is needed.  Good (and deterministic)
// software NEVER performs operations in pointer-sorted order. This container
// will iterator in the insertion order, making operations insensitive to pointer
// values.
//
// Because the contained value is also a key value, iterators are not allowed
// to return non-const references.
//
// It also supports optional ref-counting of the contained UI objects.
// =========================================================================

class UIObjectSet
{
public:

	typedef UIBaseObject *value_type;
	typedef UIBaseObject *const const_value_type;

	typedef value_type &reference;
	typedef const_value_type &const_reference;

	// =============================================================

	typedef std::list<int>                                       ObjectListDummy;
	typedef ObjectListDummy::iterator                            ObjectListDummyIterator;
	typedef std::map<UIBaseObject *, ObjectListDummyIterator>    PointerLookup;

	// =============================================================

	class ObjectEntry
	{
	public:
		ObjectEntry(UIBaseObject &o) : object(&o) {}

		UIBaseObject            *object;
		PointerLookup::iterator  pointerMapEntry;
	};

	typedef std::list<ObjectEntry> ObjectList;

	// =============================================================

	template <class ITER>
	class iterator_wrapper
	{
	public:
		friend class UIObjectSet;
		iterator_wrapper() {}
		iterator_wrapper(const ITER &i) : iter(i) {}
		iterator_wrapper(const iterator_wrapper &i) : iter(i.iter) {}

		const_reference operator*() const { return iter->object; }

		const_value_type *operator->() const { return &iter->object; }

		bool operator==(const iterator_wrapper &o) const { return iter==o.iter; }
		bool operator!=(const iterator_wrapper &o) const { return iter!=o.iter; }
		iterator_wrapper &operator++()    { ++iter; return *this; }
		iterator_wrapper &operator--()    { --iter; return *this; }
		iterator_wrapper  operator++(int) { iterator_wrapper returnValue(*this); ++iter; return returnValue; }
		iterator_wrapper  operator--(int) { iterator_wrapper returnValue(*this); --iter; return returnValue; }
	protected:
		ITER iter;
	};

	typedef iterator_wrapper<ObjectList::iterator>         iterator;
	typedef iterator_wrapper<ObjectList::const_iterator>   const_iterator;
	typedef iterator_wrapper<ObjectList::reverse_iterator> reverse_iterator;

	// =============================================================

	UIObjectSet(bool doRefCounting) : m_doRefCounting(doRefCounting) {}

	// =============================================================

	iterator         begin()       { return iterator(entries.begin()); }
	iterator         end()         { return iterator(entries.end()); }

	const_iterator   begin() const { return const_iterator(entries.begin()); }
	const_iterator   end()   const { return const_iterator(entries.end()); }

	reverse_iterator rbegin()      { return reverse_iterator(entries.rbegin()); }
	reverse_iterator rend()        { return reverse_iterator(entries.rend()); }

	// =============================================================

	const_reference front() const { return entries.front().object; }
	const_reference back()  const { return entries.back().object; }

	// =============================================================
	void clear();

	std::pair<iterator, bool> insert(UIBaseObject *);
	bool push_back(UIBaseObject *);
	bool erase(UIBaseObject *);
	bool erase(const iterator &);

	// =============================================================

	bool empty()                  const { return entries.empty(); }
	int  size()                   const { return pointerLookup.size(); }
	bool contains(UIBaseObject *) const;

	// =============================================================

	bool                   m_doRefCounting;
	ObjectList             entries;
	PointerLookup          pointerLookup;
};


#endif
