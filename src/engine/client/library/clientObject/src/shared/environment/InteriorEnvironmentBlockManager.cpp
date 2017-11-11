//===================================================================
//
// InteriorEnvironmentBlockManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/InteriorEnvironmentBlockManager.h"

#include "clientObject/InteriorEnvironmentBlock.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>
#include <set>

//===================================================================

namespace InteriorEnvironmentBlockManagerNamespace
{
	enum ColumnData
	{
		CD_pobShortName,
		CD_cellName,
		CD_dayAmbientSoundTemplateName,
		CD_nightAmbientSoundTemplateName,
		CD_firstMusicSoundTemplateName,
		CD_surfaceType,
		CD_fogEnabled,
		CD_fogDensity,
		CD_fogColorR,
		CD_fogColorG,
		CD_fogColorB,
		CD_environmentTextureName,
		CD_roomType
	};

	bool ms_installed;

	typedef std::map<const CrcString*, InteriorEnvironmentBlock*, LessPointerComparator> EnvironmentBlockMap;
	EnvironmentBlockMap* ms_environmentBlockMap;

	InteriorEnvironmentBlock* ms_defaultEnvironmentBlock;

	typedef std::set<std::string> WarningSet;
	WarningSet* ms_warningSet;

	bool ms_defaultDefaultWarning;

	void remove ();
	void load (const char* fileName);
}

using namespace InteriorEnvironmentBlockManagerNamespace;

//===================================================================
// PUBLIC InteriorEnvironmentBlockManager
//===================================================================

void InteriorEnvironmentBlockManager::install ()
{
	DEBUG_FATAL (ms_installed, ("InteriorEnvironmentBlockManager installed"));
	ms_installed = true;

	ms_environmentBlockMap = new EnvironmentBlockMap;

	InteriorEnvironmentBlockData data;
	data.m_name = "_default";
	ms_defaultEnvironmentBlock = new InteriorEnvironmentBlock ();
	ms_defaultEnvironmentBlock->setData (data);

	ms_warningSet = new WarningSet;

	ms_defaultDefaultWarning = false;

	load ("datatables/interior/interior.iff");

	//-- add to exitchain
	ExitChain::add (remove, "InteriorEnvironmentBlockManager::remove");
}

//-------------------------------------------------------------------

void InteriorEnvironmentBlockManagerNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("InteriorEnvironmentBlockManager not installed"));
	ms_installed = false;

	std::for_each (ms_environmentBlockMap->begin (), ms_environmentBlockMap->end (), PointerDeleterPairSecond ());
	delete ms_environmentBlockMap;
	ms_environmentBlockMap = 0;

	delete ms_defaultEnvironmentBlock;
	ms_defaultEnvironmentBlock = 0;

	delete ms_warningSet;
	ms_warningSet = 0;

	ms_defaultDefaultWarning = false;
}

//-------------------------------------------------------------------

const InteriorEnvironmentBlock* InteriorEnvironmentBlockManager::getEnvironmentBlock (const char* const pobShortName, const char* const cellName)
{
	DEBUG_FATAL (!ms_installed, ("InteriorEnvironmentBlockManager not installed"));

	//-- look for pobShortName_cellName
	char buffer [256];
	snprintf (buffer, 256, "%s_%s", pobShortName, cellName);

	{
		TemporaryCrcString name (buffer, false);
		EnvironmentBlockMap::iterator iter = ms_environmentBlockMap->find (&name);
		if (iter != ms_environmentBlockMap->end ())
			return iter->second;
	}

	//-- not found, look for pobShortName_default
	{
		snprintf (buffer, 256, "%s_default", pobShortName);

		//-- look for pobShortName_default
		TemporaryCrcString name (buffer, false);
		EnvironmentBlockMap::iterator iter = ms_environmentBlockMap->find (&name);
		if (iter != ms_environmentBlockMap->end ())
			return iter->second;
	}

	//-- not found, warn and return default_default
#ifdef _DEBUG
	{
		WarningSet::iterator iter = ms_warningSet->find (pobShortName);
		if (iter == ms_warningSet->end ())
		{
			DEBUG_WARNING (true, ("Did not find interior environment block entry for pob %s", pobShortName));
			ms_warningSet->insert (pobShortName);
		}
	}
#endif

	ConstCharCrcString name ("default_default");
	EnvironmentBlockMap::iterator iter = ms_environmentBlockMap->find (&name);
	if (iter != ms_environmentBlockMap->end ())
		return iter->second;

	//-- not found, sigh, return _default 
#ifdef _DEBUG
	if (!ms_defaultDefaultWarning)
	{
		ms_defaultDefaultWarning = true;
		DEBUG_WARNING (true, ("Did not find interior environment block entry for pob default, cell default"));
	}
#endif

	return ms_defaultEnvironmentBlock;
}

//-------------------------------------------------------------------

const InteriorEnvironmentBlock* InteriorEnvironmentBlockManager::getDefaultEnvironmentBlock ()
{
	return getEnvironmentBlock ("default", "default");
}

//===================================================================
// PRIVATE InteriorEnvironmentBlockManager
//===================================================================

void InteriorEnvironmentBlockManagerNamespace::load (const char* fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
	{
		DataTable dataTable;
		dataTable.load (iff);

		const int numberOfRows = dataTable.getNumRows ();
		int row;
		for (row = 0; row < numberOfRows; ++row)
		{
			InteriorEnvironmentBlockData data;

			char buffer [256];
			snprintf (buffer, 256, "%s_%s", dataTable.getStringValue (CD_pobShortName, row), dataTable.getStringValue (CD_cellName, row));
			data.m_name = buffer;
			data.m_dayAmbientSoundTemplateName        = dataTable.getStringValue (CD_dayAmbientSoundTemplateName, row);
			data.m_nightAmbientSoundTemplateName      = dataTable.getStringValue (CD_nightAmbientSoundTemplateName, row);
			data.m_firstMusicSoundTemplateName        = dataTable.getStringValue (CD_firstMusicSoundTemplateName, row);
			data.m_surfaceType                        = dataTable.getStringValue (CD_surfaceType, row);
			data.m_fogEnabled                         = dataTable.getIntValue    (CD_fogEnabled, row) != 0;
			data.m_fogDensity                         = dataTable.getFloatValue  (CD_fogDensity, row);
			data.m_fogColor.a                         = 1.f;
			data.m_fogColor.r                         = dataTable.getFloatValue  (CD_fogColorR, row);
			data.m_fogColor.g                         = dataTable.getFloatValue  (CD_fogColorG, row);
			data.m_fogColor.b                         = dataTable.getFloatValue  (CD_fogColorB, row);
			data.m_environmentTextureName             = dataTable.getStringValue (CD_environmentTextureName, row);
			data.m_roomType                           = dataTable.getIntValue    (CD_roomType, row);

			TemporaryCrcString name (buffer, false);
			if (ms_environmentBlockMap->find (&name) == ms_environmentBlockMap->end ())
			{
				InteriorEnvironmentBlock* const environmentBlock = new InteriorEnvironmentBlock ();
				environmentBlock->setData (data);

				ms_environmentBlockMap->insert (std::make_pair (environmentBlock->getName (), environmentBlock));
			}
			else
				DEBUG_WARNING (true, ("InteriorEnvironmentBlockManager::load: environment block file %s specified %s more than once", fileName, buffer));
		}
	}
}

//===================================================================
