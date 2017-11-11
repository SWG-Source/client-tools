//======================================================================
//
// UIPropertyRegister.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPropertyRegister_H
#define INCLUDED_UIPropertyRegister_H

//======================================================================

#include "UIPropertyRegisterEntry.h"

#include <map>

class UIPropertyRegister;

//----------------------------------------------------------------------
class EntryContainerBase
{
public:
	virtual ~EntryContainerBase () = 0 {}
};

template <typename T> class EntryContainer : public EntryContainerBase
{
public:
	T * entry;
	
	EntryContainer<T> (T * _entry) : entry (_entry) {}
	
	~EntryContainer<T> ()
	{
		delete entry;
		entry = 0;
	}
};

//----------------------------------------------------------------------

template<typename T> class StaticPropertyRegister
{
	typedef UIPropertyRegisterEntry<T>              EntryType;
	typedef std::map<std::string, EntryType *>      Entries;
	typedef std::map<UIPropertyRegister *, Entries> RegisterEntries;

	static RegisterEntries  s_registerEntries;

	virtual void   clearEntries (UIPropertyRegister * reg)
	{
		Entries * const entries = findEntries (reg);
		
		if (entries)
		{
			for (Entries::iterator eit = entries->begin (); eit != entries->end (); ++eit)
			{
				delete (*eit).second;
			}
			
			s_registerEntries.erase (entries);
		}
	}

	bool setProperty (UIPropertyRegister * reg, const std::string & name, Unicode::String & )
	{
		EntryType * const entry = findEntry (reg, name);

		if (entry)
		{
			
		}
	}

private:
	Entries * findEntries (UIPropertyRegister * reg)
	{
		RegisterEntries::iterator it = s_registerEntries.find (reg);

		if (it != s_registerEntries.end ())
			return (*it).second;
	}

	EntryType * findEntry (UIPropertyRegister * reg, const std::string & name)
	{
		Entries * const entries = findEntries (reg);
		if (entries)
		{
			const Entries::iterator it = entries->find (name);
			if (it != entries->end ())
				return (*it).second;
		}

		return 0;
	}
};

//----------------------------------------------------------------------

class UIPropertyRegister
{
public:


	template <typename T> void addPropertyRegisterEntry (T * entry)
	{
		const char * const name = t->GetName ();
		assert (name);
		const std::string lowerName (Unicode::toLower (name));

		m_propertyEntries->insert(std::make_pair (lowerName, new EntryContainer (entry)));
	}

	~UIPropertyRegister ()
	{
		for (PropertyEntryMap::iterator it = m_propertyEntries.begin (); it != m_propertyEntries.end (); ++it)
		{
			EntryContainerBase * const entry = (*it).second;
			delete entry;
		}
	}

	template <typename T> bool setProperty (const std::string & name, Unicode::String & value)
	{
		StaticPropertyRegister<T>::setProperty (this, name, value);
	}

	typedef std::map<std::string, EntryContainerBase *> PropertyEntryMap;
	PropertyEntryMap m_propertyEntries;
};

//======================================================================

#endif
