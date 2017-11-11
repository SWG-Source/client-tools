// ======================================================================
//
// SpaceQuest.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceQuest_H
#define INCLUDED_SpaceQuest_H

// ======================================================================

#include "StringTable.h"

// ======================================================================

class SpaceQuest
{
public:

	SpaceQuest();
	~SpaceQuest();

	bool load(char const * rootName, char const * missionTemplateType);
	bool save(char const * rootName, char const * missionTemplateType) const;

	CString const & getProperty(CString const & key) const;
	void setProperty(CString const & key, CString const & value);

	int getNumberOfStrings() const;
	CString const & getStringKey(int index) const;
	CString const & getString(CString const & key) const;
	bool hasStringKey(CString const & key) const;
	void setString(CString const & key, CString const & value);
	void removeString(CString const & key);

	CString const & getQuestCategory() const;
	void setQuestCategory(CString const & questCategory);

private:

	bool loadMissionDataTable(char const * const rootName, char const * const missionTemplateType);
	bool loadStringTable(char const * const rootName, char const * const missionTemplateType);
	bool loadQuestListDataTable(char const * const rootName, char const * const missionTemplateType);
	bool loadQuestTaskDataTable(char const * const rootName, char const * const missionTemplateType);

	bool saveMissionDataTable(char const * const rootName, char const * const missionTemplateType) const;
	bool saveStringTable(char const * const rootName, char const * const missionTemplateType) const;
	bool saveQuestListDataTable(char const * const rootName, char const * const missionTemplateType) const;
	bool saveQuestTaskDataTable(char const * const rootName, char const * const missionTemplateType) const;

private:

	StringTable m_propertyList;
	StringTable m_stringTable;
	CString m_questCategory;
};

// ======================================================================

#endif
