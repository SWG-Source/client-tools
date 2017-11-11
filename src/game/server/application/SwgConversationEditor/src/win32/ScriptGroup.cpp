// ======================================================================
//
// ScriptGroup.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptGroup.h"

#include "sharedFile/Iff.h"

#include <cstdio>
#include <map>
#include <string>

// ======================================================================
// ScriptGroupNamespace
// ======================================================================

namespace ScriptGroupNamespace
{
	Tag const TAG_SGRP = TAG (S,G,R,P);
	Tag const TAG_SFAM = TAG (S,F,A,M);
}

using namespace ScriptGroupNamespace;

// ======================================================================
// ScriptGroup::Family
// ======================================================================

class ScriptGroup::Family
{
public:

	explicit Family (int familyId);
	~Family ();

	int getFamilyId () const;
	void setFamilyId (int familyId);

	char const * getName () const;
	void setName (char const * name);

	char const * getText () const;
	void setText (char const * text);

private:

	Family ();
	Family (const Family & rhs); //lint -esym (754, Family::Family)
	Family & operator= (const Family & rhs); //lint -esym (754, Family::operator=)

private:

	int m_familyId;
	std::string m_name;
	std::string m_text;
};

// ----------------------------------------------------------------------

ScriptGroup::Family::Family (int const familyId) :
	m_familyId (familyId),
	m_name (),
	m_text ()
{
}

// ----------------------------------------------------------------------

ScriptGroup::Family::~Family ()
{
}

// ----------------------------------------------------------------------

char const * ScriptGroup::Family::getName () const
{
	return m_name.c_str ();
}

// ----------------------------------------------------------------------

void ScriptGroup::Family::setName (char const * const name)
{
	m_name = name;
}

// ----------------------------------------------------------------------

int ScriptGroup::Family::getFamilyId () const
{
	return m_familyId;
}

// ----------------------------------------------------------------------

void ScriptGroup::Family::setFamilyId (int const familyId)
{
	m_familyId = familyId;
}

// ----------------------------------------------------------------------

char const * ScriptGroup::Family::getText () const
{
	return m_text.c_str ();
}

// ----------------------------------------------------------------------

void ScriptGroup::Family::setText (char const * const text)
{
	m_text = text;
}

// ======================================================================
// ScriptGroup
// ======================================================================

ScriptGroup::ScriptGroup () :
	m_familyMap (new FamilyMap)
{
}

// ----------------------------------------------------------------------

ScriptGroup::~ScriptGroup ()
{
	//-- delete family list
	reset ();

	delete m_familyMap;
}

// ----------------------------------------------------------------------

void ScriptGroup::load (Iff & iff)
{
	//-- delete family list first
	reset ();

	//-- load data
	if (iff.enterForm (TAG_SGRP, true))
	{
		DEBUG_WARNING (iff.getCurrentName () != TAG_0000, ("ScriptGroup::load - loading old version"));

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			DEBUG_FATAL (true, ("ScriptGroup::load unknown tag"));
		}

		iff.exitForm ();
	}
}

// ----------------------------------------------------------------------

void ScriptGroup::save (Iff & iff) const
{
	iff.insertForm (TAG_SGRP);

		iff.insertForm (TAG_0000);

			for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
			{
				Family const * const family = iter->second;

				iff.insertForm (TAG_SFAM);

					iff.insertChunk (TAG_DATA);

						iff.insertChunkData (static_cast<int32> (family->getFamilyId ()));
						iff.insertChunkString (family->getName ());
						iff.insertChunkString (family->getText ());

					iff.exitChunk ();

				iff.exitForm ();
			}

		iff.exitForm ();

	iff.exitForm ();
}

// ----------------------------------------------------------------------

void ScriptGroup::reset ()
{
	for (FamilyMap::iterator iter = m_familyMap->begin (); iter != m_familyMap->end (); ++iter)
		delete iter->second;

	m_familyMap->clear ();
}

// ----------------------------------------------------------------------

char const * ScriptGroup::getFamilyName (int const familyId) const
{
	Family const * const family = getFamily (familyId);
	return family->getName ();
}

// ----------------------------------------------------------------------

void ScriptGroup::setFamilyName (int const familyId, char const * const name)
{
	Family * const family = getFamily (familyId);
	family->setName (name);
}

// ----------------------------------------------------------------------

char const * ScriptGroup::getFamilyText (int const familyId) const
{
	Family const * const family = getFamily (familyId);
	return family->getText ();
}

// ----------------------------------------------------------------------

void ScriptGroup::setFamilyText (int const familyId, char const * const text)
{
	Family * const family = getFamily (familyId);
	family->setText (text);
}

// ----------------------------------------------------------------------

void ScriptGroup::addFamily (int const familyId, char const * const name)
{
	DEBUG_FATAL (getFamily (familyId) != 0, ("family with id %i already exists", familyId));

	Family * family = new Family (familyId);
	family->setName (name);

	IGNORE_RETURN (m_familyMap->insert (FamilyMap::value_type (familyId, family)));
}

// ----------------------------------------------------------------------

void ScriptGroup::removeFamily (int const familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	DEBUG_FATAL (iter == m_familyMap->end (), ("family with id %i not found", familyId));

	delete iter->second;
	m_familyMap->erase (iter);
}

// ----------------------------------------------------------------------

bool ScriptGroup::hasFamily (int const familyId) const
{
	return getFamily (familyId) != 0;
}

// ----------------------------------------------------------------------

bool ScriptGroup::findFamily (char const * const name, int & familyId) const
{
	Family const * const family = getFamily (name);

	if (family)
		familyId = family->getFamilyId ();

	return family != 0;
}

// ----------------------------------------------------------------------

int ScriptGroup::getNumberOfFamilies () const
{
	return static_cast<int> (m_familyMap->size ());
}

// ----------------------------------------------------------------------

ScriptGroup::Family const * ScriptGroup::getFamily (int const familyId) const
{
	FamilyMap::const_iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

// ----------------------------------------------------------------------

ScriptGroup::Family * ScriptGroup::getFamily (int const familyId)
{
	FamilyMap::iterator iter = m_familyMap->find (familyId);
	if (iter != m_familyMap->end ())
		return iter->second;

	return 0;
}

// ----------------------------------------------------------------------

ScriptGroup::Family const * ScriptGroup::getFamily (char const * const familyName) const
{
	FamilyMap::iterator iter = m_familyMap->begin ();
	for (; iter != m_familyMap->end (); ++iter)
	{
		Family const * const family = iter->second;

		if (_stricmp (familyName, family->getName ()) == 0)
			return family;
	}

	return 0;
}

// ----------------------------------------------------------------------

ScriptGroup::Family * ScriptGroup::getFamily (char const * const familyName)
{
	FamilyMap::iterator iter = m_familyMap->begin ();
	for (; iter != m_familyMap->end (); ++iter)
	{
		Family * const family = iter->second;

		if (_stricmp (familyName, family->getName ()) == 0)
			return family;
	}

	return 0;
}

// ----------------------------------------------------------------------

void ScriptGroup::load_0000 (Iff & iff)
{
	iff.enterForm (TAG_0000);

		int n = iff.getNumberOfBlocksLeft ();
		int i;
		for (i = 0; i < n; i++)
		{
			iff.enterForm (TAG_SFAM);

				iff.enterChunk (TAG_DATA);

					int const familyId = iff.read_int32 ();

					//-- see if family id is already in use
					DEBUG_FATAL (getFamily (familyId) != 0, ("ScriptGroup::load familyId %i is already in use", familyId));

					std::string name;
					iff.read_string (name);

					std::string text;
					iff.read_string (text);

					//-- add family
					addFamily (familyId, name.c_str ());

				iff.exitChunk ();

				Family * const family = getFamily (familyId);
				NOT_NULL (family);
				family->setText (text.c_str ());

			iff.exitForm ();
		} //lint !e429 //-- family not freed or returned

	iff.exitForm ();
}

// ----------------------------------------------------------------------

int ScriptGroup::getFamilyId (int const familyIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, familyIndex, static_cast<int> (m_familyMap->size ()));

	FamilyMap::iterator iter = m_familyMap->begin ();
	int i = 0;
	for (; i < familyIndex || iter == m_familyMap->end (); ++i, ++iter)
		;

	DEBUG_FATAL (iter == m_familyMap->end (), (""));
	return iter->second->getFamilyId ();
}

// ----------------------------------------------------------------------

int ScriptGroup::createUniqueFamilyId () const
{
	int familyId = 1;

	while (getFamily (familyId) != 0)
		++familyId;

	return familyId;
}

// ----------------------------------------------------------------------

std::string ScriptGroup::createUniqueFamilyName (char const * const baseName) const
{
	char familyName [1000];
	int i = 0;

	do
	{
		++i;

		sprintf (familyName, "%s_%i", baseName, i);
	}
	while (getFamily (familyName) != 0);

	return std::string (familyName);
}

// ======================================================================
