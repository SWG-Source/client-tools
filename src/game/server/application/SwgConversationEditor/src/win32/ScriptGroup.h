// ======================================================================
//
// ScriptGroup.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ScriptGroup_H
#define INCLUDED_ScriptGroup_H

// ======================================================================

class Iff;

// ======================================================================

class ScriptGroup
{
public:

	ScriptGroup ();
	~ScriptGroup ();

	void load (Iff & iff);
	void save (Iff & iff) const;
	void reset ();

	char const * getFamilyName (int familyId) const;
	void setFamilyName (int familyId, char const * familyName);
	char const * getFamilyText (int familyId) const;
	void setFamilyText (int familyId, char const * text);

	void addFamily (int familyId, char const * familyName);
	void removeFamily (int familyId);
	bool hasFamily (int familyId) const;
	bool findFamily (char const * name, int & familyId) const;

	int getNumberOfFamilies () const;
	int getFamilyId (int familyIndex) const;

private:

	//-- list of families
	class Family;
	typedef stdmap<int, Family *>::fwd FamilyMap;
	FamilyMap * const m_familyMap;

private:

	Family const * getFamily (int familyId) const;
	Family* getFamily (int familyId);
	Family const * getFamily (char const * familyName) const;
	Family* getFamily (char const * familyName);
	
	int createUniqueFamilyId () const;
	std::string createUniqueFamilyName (char const * baseName) const;

	void load_0000 (Iff & iff);

private:

	ScriptGroup (const ScriptGroup& rhs);
	ScriptGroup& operator= (const ScriptGroup& rhs);
};

// ======================================================================

#endif
