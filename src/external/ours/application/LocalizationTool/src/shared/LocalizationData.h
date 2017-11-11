// ======================================================================
//
// LocalizationData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizationData_H
#define INCLUDED_LocalizationData_H

class LocalizedStringTableRW;
#include "LocalizedStringPair.h"

#include "LocalizedString.h"

#include <unordered_map>
#include <set>

struct DataChangeListener;

class LocalizedStringTableRW;

// ======================================================================
class LocalizationData
{
public:

	typedef std::unordered_map<LocalizedString::id_type, LocalizedStringPair> StringMap_t;

	typedef StringMap_t::iterator iterator;
	typedef StringMap_t::const_iterator const_iterator;

	static void                  install              ();
	static void                  remove               ();

	static LocalizationData &    getData              ();

	const_iterator               getConstIterators    (const_iterator & end);

	const LocalizedStringPair *  getEntryById         (LocalizedString::id_type id) const;
	const LocalizedStringPair *  getEntryByName       (const std::string & name) const;

	void                         removeListener       (DataChangeListener * listener);
	void                         addListener          (DataChangeListener * listener);

	const LocalizedStringPair *  createNewEntry       ();

	void                         deleteEntryById      (LocalizedString::id_type id);

	void                         modifyEntry          (const LocalizedStringPair & rhs);
	void                         modifyEntryNoUpdate  (const LocalizedStringPair & rhs);

	int                          getCurrentEditId     () const;
	void                         setCurrentEditId     (int id);

	void                         clear                ();

	bool                         populateStringTable  (LocalizedStringTableRW & table, int which);
	bool                         loadFromStringTable  (LocalizedStringTableRW & table, int which);

	bool                         getHasTranslatedData () const;
	void                         setHasTranslatedData (bool b);

	bool                         isModified           (int index) const;
	void                         clearModified        (int index);

	void                         setReadOnly          (int index, bool readonly);
	bool                         getReadOnly          (int index) const;

	void                         notifyListeners      () const;

	int                          fixupAllStringNames  (bool showMessage);

	void                         updateCrcs();
private:
	                             LocalizationData     ();
	                             LocalizationData     (const LocalizationData & rhs);
	LocalizationData &           operator=            (const LocalizationData & rhs);

	void                         notifyListenersCurrentEditingChanged () const;
	void                         notifyListenersReadOnlyChanged       () const;

	bool                         loadFromStringTableSource            (LocalizedStringTableRW const & table);
	bool                         loadFromStringTableTranslated        (LocalizedStringTableRW & table);

private:

	static LocalizationData *    ms_singleton;

	StringMap_t                  m_stringMap;

	typedef std::set<DataChangeListener *> Listeners_t;

	Listeners_t                  m_listeners;

	int                          m_nextIndex;

	int                          m_currentEditId;

	bool                         m_hasTranslatedData;

	bool                         m_modified [2];

	bool                         m_readonly [2];
};

//-----------------------------------------------------------------

inline LocalizationData & LocalizationData::getData ()
{
	return *ms_singleton;
}

//-----------------------------------------------------------------

inline LocalizationData::const_iterator LocalizationData::getConstIterators (const_iterator & end)
{
	end = m_stringMap.end ();
	return m_stringMap.begin ();

}

//-----------------------------------------------------------------

inline int LocalizationData::getCurrentEditId () const
{
	return m_currentEditId;
}

//-----------------------------------------------------------------

inline bool LocalizationData::getHasTranslatedData () const
{
	return m_hasTranslatedData;
}

//-----------------------------------------------------------------

inline bool LocalizationData::isModified (int index) const
{
	return m_modified [index];
}

//-----------------------------------------------------------------

inline bool LocalizationData::getReadOnly (int index) const
{
	return m_readonly [index];
}

// ======================================================================

#endif
