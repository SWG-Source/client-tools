#include "FirstUiBuilder.h"
#include "UIObjectSet.h"
#include "UIBaseObject.h"

namespace UIObjectSetNamespace
{
	inline 
	UIObjectSet::ObjectList::iterator &_iter(
		UIObjectSet::ObjectListDummyIterator &i
	)
	{ 
		return *reinterpret_cast<UIObjectSet::ObjectList::iterator *>(&i); 
	}

	inline 
	UIObjectSet::ObjectListDummyIterator &_dummy(
		UIObjectSet::ObjectList::iterator &i
	)
	{ 
		return *reinterpret_cast<UIObjectSet::ObjectListDummyIterator *>(&i); 
	}
}
using namespace UIObjectSetNamespace;

////////////////////////////////////////////////////////////////////////////////////////////////

void UIObjectSet::clear()
{
	while (!empty())
	{
		erase(front());
	}
}

std::pair<UIObjectSet::iterator, bool> UIObjectSet::insert(UIBaseObject *o)
{
	PointerLookup::iterator pli = pointerLookup.lower_bound(o);

	if (pli!=pointerLookup.end() && !pointerLookup.key_comp()(o, pli->first))
	{
		// object is already inserted.
		return std::pair<iterator, bool>(_iter(pli->second), false);
	}
	else // insert new object.
	{
		if (m_doRefCounting && o)
		{
			o->Attach(0);
		}

		// create a new item in the sequential list.
		ObjectList::iterator oi = entries.insert(entries.end(), ObjectEntry(*o));

		// create new item in the pointer-lookup map.
		oi->pointerMapEntry = pointerLookup.insert(pli, std::pair<UIBaseObject *const, ObjectListDummyIterator>(o,_dummy(oi)));

		// return a new iterator.
		return std::pair<iterator, bool>(iterator(oi), true);
	}
}

bool UIObjectSet::push_back(UIBaseObject *o)
{
	PointerLookup::iterator pli = pointerLookup.lower_bound(o);

	// if the iterator is at end or our group is LT the iterator, we need to add.
	if (pli==pointerLookup.end() || pointerLookup.key_comp()(o, pli->first))
	{
		if (m_doRefCounting && o)
		{
			o->Attach(0);
		}

		// create a new item in the sequential list.
		ObjectList::iterator oi = entries.insert(entries.end(), ObjectEntry(*o));

		// create new item in the pointer-lookup map.
		oi->pointerMapEntry = pointerLookup.insert(pli, std::pair<UIBaseObject *const, ObjectListDummyIterator>(o,_dummy(oi)));

		return true;
	}
	else
	{
		return false;
	}
}

bool UIObjectSet::erase(UIBaseObject *o)
{
	PointerLookup::iterator pli = pointerLookup.find(o);
	if (pli!=pointerLookup.end())
	{
		ObjectList::iterator oi = _iter(pli->second);
		pointerLookup.erase(pli);
		entries.erase(oi);

		if (m_doRefCounting && o)
		{
			o->Detach(0);
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool UIObjectSet::erase(const iterator &o)
{
	UIBaseObject *obj = o.iter->object;
	PointerLookup::iterator pli = o.iter->pointerMapEntry;
	entries.erase(o.iter);
	pointerLookup.erase(pli);

	if (m_doRefCounting && obj)
	{
		obj->Detach(0);
	}
	return true;
}

bool UIObjectSet::contains(UIBaseObject *o) const
{
	return pointerLookup.find(o)!=pointerLookup.end();
}

