// ======================================================================
//
// LocalizationData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "LocalizationData.h"

#include "DataChangeListener.h"
#include "LocalizedString.h"
#include "LocalizedStringTableReaderWriter.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <cassert>
#include <qmessagebox.h>
#include <cstdio>

// ======================================================================

LocalizationData * LocalizationData::ms_singleton = 0;

//-----------------------------------------------------------------

LocalizationData::LocalizationData () :
m_stringMap (),
m_listeners (),
m_nextIndex (1),
m_currentEditId (0),
m_hasTranslatedData (false)
{
	m_readonly [0] = m_readonly [1] = false;
	m_modified [0] = m_modified [1] = false;
}

//-----------------------------------------------------------------

void  LocalizationData::install ()
{
	assert (ms_singleton == 0);

	ms_singleton = new LocalizationData ();
}

//-----------------------------------------------------------------

void LocalizationData::remove ()
{
	assert (ms_singleton);

	delete ms_singleton;
	ms_singleton = 0;
}

//-----------------------------------------------------------------

const LocalizedStringPair * LocalizationData::getEntryById(LocalizedString::id_type id) const
{
	StringMap_t::const_iterator iter = m_stringMap.find (id);

	if (iter == m_stringMap.end ())
		return 0;
	
	return &(*iter).second;
}

//-----------------------------------------------------------------

const LocalizedStringPair *  LocalizationData::getEntryByName (const std::string & name) const
{
	for (StringMap_t::const_iterator it = m_stringMap.begin (); it != m_stringMap.end (); ++it)
	{
		if ((*it).second.getName () == name)
			return &(*it).second;
	}

	return 0;
}

//-----------------------------------------------------------------

void  LocalizationData::removeListener (DataChangeListener * listener)
{
	m_listeners.erase (listener);
}

//-----------------------------------------------------------------

void  LocalizationData::addListener (DataChangeListener * listener)
{
	m_listeners.insert (listener);
}

//-----------------------------------------------------------------

void  LocalizationData::notifyListeners() const
{
	for (Listeners_t::iterator iter = m_listeners.begin (); iter != m_listeners.end (); ++iter)
	{
		(*iter)->dataChanged ();
	}
}

//-----------------------------------------------------------------

void  LocalizationData::notifyListenersCurrentEditingChanged () const
{
	for (Listeners_t::iterator iter = m_listeners.begin (); iter != m_listeners.end (); ++iter)
	{
		(*iter)->currentEditingChanged ();
	}
}

//-----------------------------------------------------------------

void  LocalizationData::notifyListenersReadOnlyChanged () const
{
	for (Listeners_t::iterator iter = m_listeners.begin (); iter != m_listeners.end (); ++iter)
	{
		(*iter)->readOnlyChanged ();
	}
}

//-----------------------------------------------------------------

const LocalizedStringPair *     LocalizationData::createNewEntry ()
{
	// todo: handle lack of translated string
	LocalizedString src_locstr(static_cast<LocalizedString::id_type>(m_nextIndex), 1, Unicode::narrowToWide (""));
	LocalizedString trans_locstr(static_cast<LocalizedString::id_type>(m_nextIndex), 0, Unicode::narrowToWide (""));

	++m_nextIndex;

	LocalizedStringPair sp("default", src_locstr, m_hasTranslatedData ? &trans_locstr : 0);

	std::pair<StringMap_t::iterator, bool> retval = m_stringMap.insert (StringMap_t::value_type (src_locstr.getId (), sp));

	assert (retval.second == true);

	if (retval.second == false)
		return 0;

	notifyListeners ();

	return &(*retval.first).second;
}

//-----------------------------------------------------------------

void LocalizationData::deleteEntryById(LocalizedString::id_type id)
{
	StringMap_t::size_type retval = m_stringMap.erase(id);

	assert (retval);
	static_cast<void>(retval);

	m_modified [0] = true;
	m_modified [1] = true;

	notifyListeners ();
}

//-----------------------------------------------------------------

void  LocalizationData::modifyEntry (const LocalizedStringPair & rhs)
{
	modifyEntryNoUpdate(rhs);

	notifyListeners ();
}

//-----------------------------------------------------------------

void  LocalizationData::modifyEntryNoUpdate (const LocalizedStringPair & rhs)
{
	//-- make sure nothing exists with this name

	const LocalizedStringPair * const existing =  getEntryByName (rhs.getName ());

	if (existing && rhs.getId () != existing->getId ())
	{
		char buf [1024];
		_snprintf (buf, sizeof (buf), "An entry named '%s' already exists, at id %d", rhs.getName ().c_str (), existing->getId ());
		QMessageBox::warning (0, "Warning", buf);
		return;
	}

	//-- the assignment operator takes care of the modified flags

	const LocalizedStringPair & sp = (m_stringMap [rhs.getId ()] = rhs);

	m_modified [0] = m_modified [0] || sp.isModified (0);
	m_modified [1] = m_modified [1] || sp.isModified (1);
}
//-----------------------------------------------------------------

void LocalizationData::setCurrentEditId (int id)
{
	m_currentEditId = id;

	notifyListenersCurrentEditingChanged ();
}

//-----------------------------------------------------------------

void  LocalizationData::clear ()
{
	m_stringMap.clear ();
	m_nextIndex      = 1;
	notifyListeners ();
}

//-----------------------------------------------------------------

bool LocalizationData::populateStringTable (LocalizedStringTableRW & table, int which)
{
	std::string resultMsg;

	//-- the table must be empty

	//-- source table
	if (which == 0)
	{
		for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
		{
			LocalizedStringPair & sp = (*iter).second;
			
			table.addString (new LocalizedString (sp.getSourceString ()), sp.getName (), resultMsg);
		}

		return true;
	}

	//-- translated table
	else
	{
		for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
		{
			LocalizedStringPair & sp = (*iter).second;
			
			table.addString (new LocalizedString (sp.getTranslatedString ()), sp.getName (), resultMsg);
		}

		return true;
	}
}

//----------------------------------------------------------------------

bool LocalizationData::loadFromStringTableSource (LocalizedStringTableRW const & table)
{
	char buf [256];

	assert (m_stringMap.empty () && m_nextIndex == 1);
	
	const LocalizedStringTable::Map_t & smap = table.LocalizedStringTable::getMap ();

	unsigned long           last_src_id = 0;
	Unicode::String         last_src_str;
	std::string             last_src_name;

	for (LocalizedStringTable::Map_t::const_iterator iter = smap.begin (); iter != smap.end (); ++iter)
	{	
		const LocalizedString * const src_locstr = (*iter).second;
		assert (src_locstr);

		const unsigned long src_id = src_locstr->getId ();
		
		m_nextIndex = static_cast<int>(std::max (static_cast<LocalizedString::id_type>(m_nextIndex), src_id + 1));
		const Unicode::NarrowString * const name = table.getNameById (src_id);

		if (!name)
		{
			_snprintf (buf, sizeof (buf), 
				"LocalizationData::loadFromStringTableSource could not find a name for StringId [%d], str=[%s]\n"
				"The last StringId was id=[%d], name=[%s], str=[%s]\n",
				src_id,
				Unicode::wideToNarrow(src_locstr->getString ()).c_str(),
				last_src_id,
				last_src_name.c_str(),
				last_src_str.c_str ());

			QMessageBox::information(0, "Error", buf);
			return false;
		}
		
		const LocalizedStringPair sp(*name, *src_locstr, 0);
		
		std::pair<StringMap_t::iterator, bool> retval = m_stringMap.insert (std::make_pair (src_id, sp));
		
		assert (retval.second == true);
		
		if (retval.second == false)
		{
			_snprintf (buf, sizeof (buf), "LocalizationData::loadFromStringTableSource failed to insert id [%d] ([%s]:[%s])\n", 
				src_id, name->c_str (), Unicode::wideToNarrow (src_locstr->getString ()).c_str ());
			QMessageBox::information (0, "Error", buf);
			return false;
		}

		last_src_id     = src_locstr->getId ();
		last_src_str    = src_locstr->getString ();
		last_src_name   = *name;
	}

	return true;
}


//----------------------------------------------------------------------

bool LocalizationData::loadFromStringTableTranslated (LocalizedStringTableRW & table)
{
	LocalizedStringTable::Map_t & smap = table.getMap ();	
	
	int num_trans_added = 0;
	int num_source_added = 0;
	
	//-- first get all the translated strings which correspond to existing source strings
	{
		for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
		{
			LocalizedStringPair & sp = (*iter).second;

			LocalizedString const * const translatedString = table.getLocalizedStringByName(sp.getName());

			sp.setHasTranslatedString (true);
			
			if (!translatedString || sp.getHasTranslatedString () == false)
			{
				std::string fileName = table.getFileName();
				size_t const index = table.getFileName().find_last_of("\\/");
				if (index != std::string::npos)
					fileName = table.getFileName().c_str() + index + 1;

				std::string stringValue = fileName;
				stringValue += ":";
				stringValue += sp.getName();
				stringValue += " <NEEDS TRANSLATION>";
				LocalizedStringTableRW::str_setString(sp.getTranslatedString(), Unicode::narrowToWide(stringValue));
				++num_trans_added;
				sp.setHasTranslatedString(true);

				//-- We're resetting the translated string's source crc to NULL so the crcs will differ to get the orange bar to highlight the line.
				sp.getTranslatedString().setSourceCrc(LocalizedString::nullCrc);

				continue;
			}

			sp.getTranslatedString() = *translatedString;
			sp.setModified (1, false);
			
			smap.erase(translatedString->getId());
		}
	}
	
	if (num_trans_added)
	{
		m_modified [1] = true;

		//-- force an update to the data so the table will draw correctly
		notifyListeners ();		

		char buf [1024];
		_snprintf (buf, sizeof (buf), "%d entries were not found in the translated file, and were added.\n"
			"These entries will appear out-of-date in the list.\n"
			"The translated file will not change on disk until you Save it.", num_trans_added);
		QMessageBox::information (0, "Information", buf);
	}


	//-- now see if the translated file has strings that should be merged into the source file
	
	//-- do the reverse merge
	if (!smap.empty ())
	{
		char buf [1024];
		_snprintf (buf, sizeof (buf), 
			"The translated file contains %d entries which do not appear in the source file.\n"
			"You may choose to create new source entries for all the new translated entries,\n"
			"have me prompt you for confirmation of each string, or discard all new entries\n"
			"from the translated file.\n"
			"As usual, changes will not be committed to disk until you Save.", smap.size ());
		
		int warn_result = QMessageBox::warning (0, "Forward merge?",
			buf,
			"&Merge All", "&Prompt Me", "&Discard All", 0, 2);
		
		// don't discard
		if (warn_result != 2)
		{			
			
			for (LocalizedStringTable::Map_t::const_iterator iter = smap.begin (); iter != smap.end (); ++iter)
			{
				const LocalizedString * const trans_locstr = (*iter).second;
				assert (trans_locstr);
				
				m_nextIndex = static_cast<int>(std::max(static_cast<LocalizedString::id_type>(m_nextIndex), trans_locstr->getId () + 1));	
				const Unicode::NarrowString * const name = table.getNameById (trans_locstr->getId ());
				assert (name);
							
				//-- prompt user for info
				if (warn_result == 1)
				{
					char buf2[1024];
					_snprintf (buf2, sizeof(buf2), "Merge Translated entry '%s'?\n", name->c_str ());
					int merge_result = QMessageBox::warning (0, "Forward merge entry?",
						buf2,
						"&Merge", "&Don't Merge", "&Cancel", 0, 2);
					
					if (merge_result == 2)
						return false;
					
					//-- don't merge
					if (merge_result == 1)
						continue;
				}
				
				// make the source newer than the translated to it will appear out-of-date
				const LocalizedString src_locstr (trans_locstr->getId (), Unicode::narrowToWide ("<REVERSE-MERGED... UNKNOWN>"));
				
				LocalizedStringPair sp(*name, src_locstr, trans_locstr );
				
				std::pair<StringMap_t::iterator, bool> retval = m_stringMap.insert (StringMap_t::value_type (trans_locstr->getId (), sp));
				
				assert (retval.second == true);
				
				if (retval.second == false)
					return false;					
				
				//-- the source entry is now modified
				(*(retval.first)).second.setModified (0, true);

				++num_source_added;

				if (warn_result == 1)
				{
					m_modified [0] = true;
					//-- force an update to the data so the table will draw correctly
					notifyListeners ();
				}

			}

			if (num_source_added)
				m_modified [0] = true;

			if (warn_result != 1)
			{
				//-- force an update to the data so the table will draw correctly
				notifyListeners ();
			}

			char buf3[1024];
			_snprintf(buf3, sizeof (buf3), "%d entries were merged INTO the source file.\nChanges will not be committed to disk until you Save.", num_source_added);
			QMessageBox::information(0, "Information", buf3);
		}
	}

	return true;
}

//-----------------------------------------------------------------

bool LocalizationData::loadFromStringTable (LocalizedStringTableRW & table, int which)
{
	assert (m_currentEditId == 0);

	//-- source table

	if (which == 0)
	{
		if (!loadFromStringTableSource (table))
			return false;
	}

	//-- translated table
	
	else
	{
		if (!loadFromStringTableTranslated (table))
			return false;
	}
	
	fixupAllStringNames (true);

	notifyListeners ();
	return true;
}

//----------------------------------------------------------------------

int LocalizationData::fixupAllStringNames (bool showMessage)
{
	int count = 0;
	for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
	{
		LocalizedStringPair & sp = (*iter).second;

		if (!LocalizedStringTable::validateStringName (sp.getName ()))
		{
			std::string name = sp.getName ();
			LocalizedStringTable::fixupStringName (name);
			sp.setName (name);
			++count;
		}
	}
	
	if (count)
	{
		m_modified [0] = m_modified [1] = true;
		if (showMessage)
		{
			char buf [1024];
			_snprintf (buf, sizeof (buf), "%d entries had invalid names and were fixed.\nChanges will not be committed to disk until you Save.", count);
			QMessageBox::information (0, "Information", buf);
		}
	}

	return count;
}

//-----------------------------------------------------------------

/**
* Before calling this, the caller must make sure that the editing window(s) cancel their editing.
*/

void LocalizationData::setHasTranslatedData (bool b)
{
	if (b == m_hasTranslatedData)
		return;
	
	m_hasTranslatedData = b;

	for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
	{
		LocalizedStringPair & sp = (*iter).second;
		sp.setHasTranslatedString (b);
	}

	notifyListeners ();
}

//-----------------------------------------------------------------

void LocalizationData::clearModified (int index)
{
	assert (index == 0 || index == 1);

	for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
	{
		LocalizedStringPair & sp = (*iter).second;
		sp.setModified (index, false);
	}

	m_modified [index] = false;

	notifyListeners ();
}

//-----------------------------------------------------------------

void  LocalizationData::setReadOnly (int index, bool readonly)
{
	assert (index == 0 || index == 1);

	m_readonly [index] = readonly;

	notifyListenersReadOnlyChanged ();
}

//-----------------------------------------------------------------

void  LocalizationData::updateCrcs()
{
	for (StringMap_t::iterator iter = m_stringMap.begin (); iter != m_stringMap.end (); ++iter)
	{
		LocalizedStringPair & sp = (*iter).second;
		sp.updateCrc();
	}
}

// ======================================================================
