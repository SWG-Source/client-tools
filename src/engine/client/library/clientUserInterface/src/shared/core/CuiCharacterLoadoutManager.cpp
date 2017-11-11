//======================================================================
//
// CuiCharacterLoadoutManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCharacterLoadoutManager.h"

#include "sharedFile/Iff.h"
#include "clientGame/ClientObject.h"
#include "sharedObject/ObjectTemplate.h"
#include "clientGame/ContainerInterface.h"
#include "sharedObject/SlottedContainer.h"

#include <map>

//======================================================================

namespace
{

	bool s_installed = false;

	namespace Tags
	{
		const Tag LOEQ = TAG(L,O,E,Q);
		const Tag PTMP = TAG(P,T,M,P);
		const Tag ITEM = TAG(I,T,E,M);
	}
}

//----------------------------------------------------------------------

CuiCharacterLoadoutManager::LoadoutMap *  CuiCharacterLoadoutManager::ms_loadoutMap;
std::string                               CuiCharacterLoadoutManager::ms_filename ("creation/default_pc_equipment.iff");

//----------------------------------------------------------------------

void CuiCharacterLoadoutManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiCharacterLoadoutManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	delete ms_loadoutMap;
	ms_loadoutMap = 0;
	s_installed = false;
}

//----------------------------------------------------------------------

const CuiCharacterLoadoutManager::LoadoutVector * CuiCharacterLoadoutManager::getLoadout (const std::string & playerTemplate)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n")); 

	if (!ms_loadoutMap &&
		!initializeLoadoutMap (ms_filename))
	{
		WARNING_STRICT_FATAL (true, ("Unable to initialize loadout equipment from file '%s'", ms_filename.c_str ()));
		return 0;
	}

	NOT_NULL (ms_loadoutMap);

	const LoadoutMap::const_iterator it = ms_loadoutMap->find (playerTemplate);

	if (it == ms_loadoutMap->end ())
	{
		WARNING_STRICT_FATAL (true, ("Unable to find default loadout for player template '%s'", playerTemplate.c_str ()));
		return 0;
	}

	return &(*it).second;
}

//----------------------------------------------------------------------

void CuiCharacterLoadoutManager::setLoadoutFilename (const std::string & filename)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	ms_filename = filename;
	clear ();
}

//----------------------------------------------------------------------

void CuiCharacterLoadoutManager::clear ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	delete ms_loadoutMap;
	ms_loadoutMap = 0;
}

//----------------------------------------------------------------------

void CuiCharacterLoadoutManager::init  ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	IGNORE_RETURN (initializeLoadoutMap (ms_filename));
}

//----------------------------------------------------------------------

bool CuiCharacterLoadoutManager::initializeLoadoutMap (const std::string & filename)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING_STRICT_FATAL (true, ("Specified equipment loadout file '%s' could not be opened.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (Tags::LOEQ, true))
	{
		WARNING_STRICT_FATAL (true, ("equipment loadout file '%s' no LOEQ form found.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (TAG_0000, true))
	{
		WARNING_STRICT_FATAL (true, ("equipment loadout file '%s' no 0000 form found.", filename.c_str ()));
		return false;
	}

	if (ms_loadoutMap)
		ms_loadoutMap->clear ();
	else
		ms_loadoutMap = new LoadoutMap;

	int ptmp_count = 0;

	for (; iff.enterForm (Tags::PTMP, true); ++ptmp_count)
	{
		std::string playerTemplateStr;
		if (iff.enterChunk (TAG_NAME, true))
		{
			 playerTemplateStr = iff.read_stdstring ();
			 iff.exitChunk ();
		}

		if (playerTemplateStr.empty ())
		{
			WARNING_STRICT_FATAL (true, ("equipment loadout file '%s' empty player template string in PTMP form %d.", filename.c_str (), ptmp_count));
			return false;
		}

		LoadoutVector v;
		while (iff.enterChunk (Tags::ITEM, true))
		{
			v.push_back (ArrangementTemplatePair (iff.read_int32 (), iff.read_stdstring ()));
			iff.exitChunk ();
		}

		IGNORE_RETURN (ms_loadoutMap->insert (std::make_pair (playerTemplateStr, v)));

		iff.exitForm ();
	}

	if (ptmp_count == 0)
	{
		WARNING (true, ("equipment loadout file '%s' contained no PTMP forms.", filename.c_str ()));
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool CuiCharacterLoadoutManager::setupLoadout (ClientObject & obj)
{
	const LoadoutVector * const loadout = getLoadout (obj.getObjectTemplateName ());

	if (loadout)
	{
		for (LoadoutVector::const_iterator it = loadout->begin (); it != loadout->end (); ++it)
		{
			const ArrangementTemplatePair & atp = *it;
			
			int arrangement = atp.first;
			
			ClientObject * const eq = safe_cast<ClientObject*>(ObjectTemplate::createObject (atp.second.c_str ()));
			
			if (!eq)
				WARNING (true, ("Invalid equipment template: '%s'", atp.second.c_str ()));
			else
			{
				//-- the loadout is always client-only
				eq->endBaselines ();
				SlottedContainer *slotted = ContainerInterface::getSlottedContainer(obj);
				if (slotted)
				{
					//Bypass the loadout arrangement and use the first available.
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					if (!slotted->getFirstUnoccupiedArrangement(*eq, arrangement, tmp))
					{
						WARNING (true, ("Unable to equip item: '%s'", eq->getObjectTemplateName ()));
					
						return false;
					}
				
					if (!ContainerInterface::transferItemToSlottedContainer (obj, *eq, arrangement))
					{
						WARNING (true, ("Unable to equip item: '%s'", eq->getObjectTemplateName ()));
						return false;
					}
				}
			}						
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool CuiCharacterLoadoutManager::test (std::string & result, const stdvector<std::string>::fwd & templateNames)
{
	clear ();

	for (std::vector<std::string>::const_iterator it = templateNames.begin (); it != templateNames.end (); ++it)
	{
		if (!getLoadout (*it))
		{
			result += "Unable to create loadout for ";
			result += *it + "\n";
			return true;
		}
	}

	return false;
}

//======================================================================
