//======================================================================
//
// CuiCharacterHairManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCharacterHairManager.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "sharedFile/Iff.h"
#include "sharedObject/Container.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedUtility/DataTable.h"

//======================================================================

namespace CuiCharacterHairManagerNamespace
{
	namespace Tags
	{
		const Tag HAIR = TAG(H,A,I,R);
		const Tag PTMP = TAG(P,T,M,P);
		const Tag ITMS = TAG(I,T,M,S);
		const Tag DEFA = TAG(D,E,F,A);

	}

	namespace Filenames
	{
		const std::string s_defaultHairstyles   ("creation/default_pc_hairstyles.iff");
		const std::string s_hairstyleAssetTable ("datatables/customization/hair_assets_skill_mods.iff");
	}

	namespace SlotNames
	{
		const std::string s_hair ("hair");
	}
}

using namespace CuiCharacterHairManagerNamespace;

//----------------------------------------------------------------------

bool CuiCharacterHairManager::getAvailableStylesSkillMod (std::string const & templateName, int const skillModValue, StringVector & /*OUT*/ result)
{
	DataTable dataTable;
	Iff customizationIff(Filenames::s_hairstyleAssetTable.c_str());
	dataTable.load(customizationIff);

	int row = -1;
	for (int i = 0; i < dataTable.getNumRows(); ++i)
	{
		row = i;
		std::string const & playerTemplate = dataTable.getStringValue(1, row);
		//only investigate rows that apply to this player template (race/gender)
		if(playerTemplate == templateName)
		{
			//check skill mod
			int requiredModValue = dataTable.getIntValue   (2, row);
			if(requiredModValue <= skillModValue)
			{
				result.push_back(dataTable.getStringValue(0, row));
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

bool CuiCharacterHairManager::getAvailableStylesCreation (std::string const & templateName, StringVector & /*OUT*/result, std::string & /*OUT*/ defaultHair)
{
	DataTable dataTable;
	Iff customizationIff(Filenames::s_hairstyleAssetTable.c_str());
	dataTable.load(customizationIff);

	bool defaultHairSet = false;

	int row = -1;
	for (int i = 0; i < dataTable.getNumRows(); ++i)
	{
		row = i;
		std::string const & playerTemplate = dataTable.getStringValue(1, row);
		//only investigate rows that apply to this player template (race/gender)
		if(playerTemplate == templateName)
		{
			//check if it's available at creation
			bool availableAtCreation = dataTable.getIntValue   (3, row) ? true : false;
			if(availableAtCreation)
			{
				std::string const & hair = dataTable.getStringValue(0, row);
				result.push_back(hair);
				if(!defaultHairSet)
				{
					defaultHair = hair;
				}
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

int CuiCharacterHairManager::getIndexForHair(std::string const & templateName)
{
	DataTable dataTable;
	Iff customizationIff(Filenames::s_hairstyleAssetTable.c_str());
	dataTable.load(customizationIff);

	int row = -1;
	for (int i = 0; i < dataTable.getNumRows(); ++i)
	{
		row = i;
		std::string const & hair = dataTable.getStringValue(0, row);
		{
			if(hair == templateName)
				return row;
		}
	}
	return -1;
}

//----------------------------------------------------------------------

std::string CuiCharacterHairManager::getHairForIndex(int const hairIndex)
{
	DataTable dataTable;
	Iff customizationIff(Filenames::s_hairstyleAssetTable.c_str());
	dataTable.load(customizationIff);

	if(hairIndex < dataTable.getNumRows())
	{
		std::string const & hairTemplate = dataTable.getStringValue(0, hairIndex);
		return hairTemplate;
	}
	return std::string();
}

//----------------------------------------------------------------------

bool CuiCharacterHairManager::loadHairStyles (std::string const & templateName, StringVector & result, std::string & /*OUT*/ defaultHair, int const hairSkillModValue)
{
	if (templateName.empty ())
		return false;

	std::string defaultHairFromDatatable;
	if(hairSkillModValue != 0)
	{
		IGNORE_RETURN(getAvailableStylesSkillMod(templateName, hairSkillModValue, result));
	}
	else
		IGNORE_RETURN(getAvailableStylesCreation(templateName, result, defaultHairFromDatatable));

	const std::string filename = Filenames::s_defaultHairstyles;

	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING_STRICT_FATAL (true, ("Specified default hairstyle file '%s' could not be opened.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (Tags::HAIR, true))
	{
		WARNING_STRICT_FATAL (true, ("default hairstyle file '%s' no HAIR form found.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (TAG_0000, true))
	{
		WARNING_STRICT_FATAL (true, ("default hairstyle file '%s' no 0000 form found.", filename.c_str ()));
		return false;
	}

	bool found = false;

	for (; !found && iff.enterForm (Tags::PTMP, true); )
	{
		std::string playerTemplateStr;
		
		if (iff.enterChunk (TAG_NAME, true))
		{
			playerTemplateStr = iff.read_stdstring ();
			iff.exitChunk ();
		}

		if (playerTemplateStr != templateName)
		{
			iff.exitForm (true);
		}
		else
		{
			found = true;

			if (iff.enterChunk (Tags::DEFA, true))
			{
				defaultHair = iff.read_stdstring ();
				iff.exitChunk ();
			}

			if (iff.enterChunk (Tags::ITMS, true))
			{
				while (iff.getChunkLengthLeft ())
				{
					//read, ignore this data (use the data from the earlier call to getAvailableStylesCreation)
					//TODO remove this unused data from the mif file
					IGNORE_RETURN(iff.read_stdstring ());
				}
				iff.exitChunk ();
			}
		}
	}

	WARNING (!found, ("Hairstyles: No PTMP form for '%s'", templateName.c_str ()));

	return true;
}

//----------------------------------------------------------------------

bool CuiCharacterHairManager::setupDefaultHair (ClientObject & obj)
{
	std::string defaultHair;
	StringVector result;
	if (!loadHairStyles (obj.getObjectTemplateName (), result, defaultHair, 0))
		return false;

	if (defaultHair.empty ())
		return true;	

	ClientObject * const hair = safe_cast<ClientObject*>(ObjectTemplate::createObject (defaultHair.c_str ()));

	if (!hair)
		WARNING (true, ("Invalid hair template: '%s'", defaultHair.c_str ()));
	else
	{
		//-- the default hair is always client-only
		hair->endBaselines ();
		SlottedContainer *slotContainer = ContainerInterface::getSlottedContainer(obj);
		if (slotContainer)
		{
			int arrangement = -1;
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			if (!slotContainer->getFirstUnoccupiedArrangement(*hair, arrangement, tmp))
			{
				WARNING (true, ("Unable to equip hair: '%s'", hair->getObjectTemplateName ()));
				return false;
			}
			if (!ContainerInterface::transferItemToSlottedContainer (obj, *hair, arrangement))
			{
				WARNING (true, ("Unable to equip hair: '%s'", hair->getObjectTemplateName ()));
				return false;
			}
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------

void CuiCharacterHairManager::replaceHair (CreatureObject & obj, TangibleObject * srcHair, bool const destroyOldHair)
{
	ClientObject * const destHair = obj.getHairObject();
	if (destHair)
	{
		Container * const container = ContainerInterface::getContainer (obj);
		if (container)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			if (!container->remove (*destHair, tmp))
				WARNING (true, ("Unable to remove dest hair"));
			else
			{
				//-- it probably shouldn't be necessary to do this.
				//-- This is required to force TangibleObject's m_containerChangeMap to be correct for
				//-- the next alter of creature

				if (destHair->alter    (1.0f))
					destHair->conclude ();
			}
		}
	}

	if (srcHair)
	{
		SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(obj);
		if (slotContainer)
		{
			int arrangement = -1;
			Container::ContainerErrorCode tmp = Container::CEC_Success;

			if (!slotContainer->getFirstUnoccupiedArrangement(*srcHair, arrangement, tmp))
			{
				WARNING (true, ("Unable to attach new hair to clone"));
			}
			slotContainer->add(*srcHair, arrangement, tmp);
		}
	}

	if (destHair)
	{
		//-- The TangibleObject m_containerChangeMap refers to the removal of objects
		//-- before deleting the objects, we must force the TangibleObject creature to
		//-- process the changes

		if (obj.alter    (1.0f))
			obj.conclude ();

		//-- it should now be safe to delete the previously worn objects

		if (destroyOldHair)
			delete destHair;
	}
}

//======================================================================
