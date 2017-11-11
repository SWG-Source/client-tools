//===================================================================
//
// ClientWeaponObjectTemplate.cpp
// copyright 2001 Sony Online Entertainment
//
// Wraps SharedWeaponObjectTemplate 
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientWeaponObjectTemplate.h"

#include "clientGame/ClientEffectTemplate.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "clientGame/WeaponObject.h"
#include "clientGame/TrackingProjectile.h"
#include "clientGame/NonTrackingProjectile.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"

//===================================================================
// namespace ClientWeaponObjectTemplateNamespace
//===================================================================

namespace ClientWeaponObjectTemplateNamespace
{
	enum Column
	{
		// C_weaponEffectId  = 0,
		C_projectile      = 2,
		C_fire            = 3,
		C_hitCreature     = 4,
		C_hitMetal        = 5,
		C_hitStone        = 6,
		C_hitWood         = 7,
		C_hitOther        = 8,
		C_missHitWater    = 9,
		C_missHitTerrain  = 10,
		C_missHitCreature = 11,
		C_missHitMetal    = 12,
		C_missHitStone    = 13,
		C_missHitWood     = 14,
		C_missHitOther    = 15,
		C_missHitNothing  = 16,
		C_ricochet        = 17
	};

	typedef stdmap<std::string, int>::fwd String2IntMap;

	DataTable     *ms_weaponEffectDataTable;
	String2IntMap *ms_weaponEffectDataTableIndexMap;

	void remove ();
}

using namespace ClientWeaponObjectTemplateNamespace;

//-------------------------------------------------------------------

void ClientWeaponObjectTemplate::install(bool allowDefaultTemplateParams)
{
	InstallTimer const installTimer("ClientWeaponObjectTemplate::install");

	SharedWeaponObjectTemplate::install(allowDefaultTemplateParams);
	IGNORE_RETURN(ObjectTemplateList::assignBinding(SharedWeaponObjectTemplate_tag, create));

	// Create the data table
	delete ms_weaponEffectDataTableIndexMap;
	ms_weaponEffectDataTableIndexMap = new String2IntMap;
	NOT_NULL(ms_weaponEffectDataTableIndexMap);

	Iff dataTableIff;

	if (dataTableIff.open("datatables/weapon/weapon.iff", true))
	{
		delete ms_weaponEffectDataTable;
		ms_weaponEffectDataTable = new DataTable();
		NOT_NULL(ms_weaponEffectDataTable);

		ms_weaponEffectDataTable->load(dataTableIff);

		// Store the row appearance name and its row index for fast indexing
		// into the DataTable.

		const int rowCount = ms_weaponEffectDataTable->getNumRows();

		for (int row = 0; row < rowCount; ++row)
		{
			std::string const &weaponEffect = ms_weaponEffectDataTable->getStringValue(0, row);
			int const weaponEffectIndex = ms_weaponEffectDataTable->getIntValue(1, row);

			char weaponIndexString[256];
			IGNORE_RETURN(_itoa(weaponEffectIndex, weaponIndexString, 10));

			std::string lookUpId(weaponEffect + weaponIndexString);

			IGNORE_RETURN(ms_weaponEffectDataTableIndexMap->insert(std::make_pair(lookUpId, row)));
		}
	}

	ExitChain::add (remove, "ClientWeaponObjectTemplateNamespace::remove");
}

//-------------------------------------------------------------------

void ClientWeaponObjectTemplateNamespace::remove ()
{
	delete ms_weaponEffectDataTableIndexMap;
	ms_weaponEffectDataTableIndexMap = NULL;

	delete ms_weaponEffectDataTable;
	ms_weaponEffectDataTable = NULL;
}

//-------------------------------------------------------------------

ClientWeaponObjectTemplate::ClientWeaponObjectTemplate(const std::string & filename) :
	SharedWeaponObjectTemplate(filename),
	m_lookUpId (),
	m_appearanceTemplate (0),
	m_fireClientEffectTemplate (0),
	m_hitCreatureClientEffectTemplate (0),
	m_hitMetalClientEffectTemplate (0),
	m_hitStoneClientEffectTemplate (0),
	m_hitWoodClientEffectTemplate (0),
	m_hitOtherClientEffectTemplate (0),
	m_missHitWaterClientEffectTemplate (0),
	m_missHitTerrainClientEffectTemplate (0),
	m_missHitCreatureClientEffectTemplate (0),
	m_missHitMetalClientEffectTemplate (0),
	m_missHitStoneClientEffectTemplate (0),
	m_missHitWoodClientEffectTemplate (0),
	m_missHitOtherClientEffectTemplate (0),
	m_missHitNothingClientEffectTemplate (0),
	m_ricochetClientEffectTemplate (0)
{
}

//-------------------------------------------------------------------

ClientWeaponObjectTemplate::~ClientWeaponObjectTemplate()
{
	AppearanceTemplateList::release (m_appearanceTemplate);
	m_appearanceTemplate = 0;

	if (m_fireClientEffectTemplate)
	{
		m_fireClientEffectTemplate->release ();
		m_fireClientEffectTemplate = 0;
	}

	if (m_hitCreatureClientEffectTemplate)
	{
		m_hitCreatureClientEffectTemplate->release ();
		m_hitCreatureClientEffectTemplate = 0;
	}

	if (m_hitMetalClientEffectTemplate)
	{
		m_hitMetalClientEffectTemplate->release ();
		m_hitMetalClientEffectTemplate = 0;
	}

	if (m_hitStoneClientEffectTemplate)
	{
		m_hitStoneClientEffectTemplate->release ();
		m_hitStoneClientEffectTemplate = 0;
	}

	if (m_hitWoodClientEffectTemplate)
	{
		m_hitWoodClientEffectTemplate->release ();
		m_hitWoodClientEffectTemplate = 0;
	}

	if (m_hitOtherClientEffectTemplate)
	{
		m_hitOtherClientEffectTemplate->release ();
		m_hitOtherClientEffectTemplate = 0;
	}

	if (m_missHitWaterClientEffectTemplate)
	{
		m_missHitWaterClientEffectTemplate->release ();
		m_missHitWaterClientEffectTemplate = 0;
	}

	if (m_missHitTerrainClientEffectTemplate)
	{
		m_missHitTerrainClientEffectTemplate->release ();
		m_missHitTerrainClientEffectTemplate = 0;
	}

	if (m_missHitCreatureClientEffectTemplate)
	{
		m_missHitCreatureClientEffectTemplate->release ();
		m_missHitCreatureClientEffectTemplate = 0;
	}

	if (m_missHitMetalClientEffectTemplate)
	{
		m_missHitMetalClientEffectTemplate->release ();
		m_missHitMetalClientEffectTemplate = 0;
	}

	if (m_missHitWoodClientEffectTemplate)
	{
		m_missHitWoodClientEffectTemplate->release ();
		m_missHitWoodClientEffectTemplate = 0;
	}

	if (m_missHitStoneClientEffectTemplate)
	{
		m_missHitStoneClientEffectTemplate->release ();
		m_missHitStoneClientEffectTemplate = 0;
	}

	if (m_missHitOtherClientEffectTemplate)
	{
		m_missHitOtherClientEffectTemplate->release ();
		m_missHitOtherClientEffectTemplate = 0;
	}

	if (m_missHitNothingClientEffectTemplate)
	{
		m_missHitNothingClientEffectTemplate->release ();
		m_missHitNothingClientEffectTemplate = 0;
	}

	if (m_ricochetClientEffectTemplate)
	{
		m_ricochetClientEffectTemplate->release ();
		m_ricochetClientEffectTemplate = 0;
	}
}

//-------------------------------------------------------------------
/**
 * Creates a new object from this template.
 *
 * @return the object
 */
Object * ClientWeaponObjectTemplate::createObject(void) const
{
	return new WeaponObject(this);
}

//-------------------------------------------------------------------
/**
 * Create a projectile object from this weapon template.
 *
 * @param nonTracking  True if a non-tracking projectile should be used; false for a tracking projectile.
 * @param useRicochet  Normally this should be false.  In the case of a lightsaber
 *                     blocking with a ricochet, set this to true to use the ricochet client effect
 *                     for any type of hit.
 */

Projectile *ClientWeaponObjectTemplate::createProjectile (const bool nonTracking, bool useRicochet) const
{
	Projectile* projectile = 0;

	WeaponObject weapon(this);

	if (m_appearanceTemplate)
	{
		if (nonTracking)
		{
			projectile = new NonTrackingProjectile (m_appearanceTemplate, weapon, CrcLowerString::empty);
		}
		else
		{
			projectile = new TrackingProjectile (m_appearanceTemplate, weapon, CrcLowerString::empty, useRicochet);
		}
	}

	return projectile;
}

//-------------------------------------------------------------------

void ClientWeaponObjectTemplate::postLoad ()
{
	// chain up to parent.
	SharedWeaponObjectTemplate::postLoad ();

	std::string const &weaponEffect = getWeaponEffect();
	int const weaponIndex           = getWeaponEffectIndex();
	char weaponIndexString[256];
	IGNORE_RETURN(_itoa(weaponIndex, weaponIndexString, 10));
	m_lookUpId = weaponEffect + weaponIndexString;

	typedef std::pair <const ClientEffectTemplate **, int> CefTemplateData;
	typedef std::pair <const char *, CefTemplateData> NamedCefTemplateData;

	const NamedCefTemplateData cefTemplateData [] =
	{
		NamedCefTemplateData ("fire",            CefTemplateData (&m_fireClientEffectTemplate,             static_cast<int>(C_fire))),
		NamedCefTemplateData ("hitCreature",     CefTemplateData (&m_hitCreatureClientEffectTemplate,      static_cast<int>(C_hitCreature))),
		NamedCefTemplateData ("hitMetal",        CefTemplateData (&m_hitMetalClientEffectTemplate,         static_cast<int>(C_hitMetal))),
		NamedCefTemplateData ("hitStone",        CefTemplateData (&m_hitStoneClientEffectTemplate,         static_cast<int>(C_hitStone))),
		NamedCefTemplateData ("hitWood",         CefTemplateData (&m_hitWoodClientEffectTemplate,          static_cast<int>(C_hitWood))),
		NamedCefTemplateData ("hitOther",        CefTemplateData (&m_hitOtherClientEffectTemplate,         static_cast<int>(C_hitOther))),
		NamedCefTemplateData ("missHitWater",    CefTemplateData (&m_missHitWaterClientEffectTemplate,     static_cast<int>(C_missHitWater))),
		NamedCefTemplateData ("missHitTerrain",  CefTemplateData (&m_missHitTerrainClientEffectTemplate,   static_cast<int>(C_missHitTerrain))),
		NamedCefTemplateData ("missHitCreature", CefTemplateData (&m_missHitCreatureClientEffectTemplate,  static_cast<int>(C_missHitCreature))),
		NamedCefTemplateData ("missHitMetal",    CefTemplateData (&m_missHitMetalClientEffectTemplate,     static_cast<int>(C_missHitMetal))),
		NamedCefTemplateData ("missHitStone",    CefTemplateData (&m_missHitStoneClientEffectTemplate,     static_cast<int>(C_missHitStone))),
		NamedCefTemplateData ("missHitWood",     CefTemplateData (&m_missHitWoodClientEffectTemplate,      static_cast<int>(C_missHitWood))),
		NamedCefTemplateData ("missHitOther",    CefTemplateData (&m_missHitOtherClientEffectTemplate,     static_cast<int>(C_missHitOther))),
		NamedCefTemplateData ("missHitNothing",  CefTemplateData (&m_missHitNothingClientEffectTemplate,   static_cast<int>(C_missHitNothing))),
		NamedCefTemplateData ("ricochet",        CefTemplateData (&m_ricochetClientEffectTemplate,         static_cast<int>(C_ricochet))),
	};
	
	static const size_t numCefTemplateData = sizeof (cefTemplateData) / sizeof (cefTemplateData [0]);
	
	const String2IntMap::const_iterator iter = ms_weaponEffectDataTableIndexMap->find(m_lookUpId);

	if (iter != ms_weaponEffectDataTableIndexMap->end())
	{
		const int row = (*iter).second;
		
		m_appearanceTemplate = AppearanceTemplateList::fetch(ms_weaponEffectDataTable->getStringValue(static_cast<int>(C_projectile), row));
		
		for (int i = 0; i < numCefTemplateData; ++i)
		{
			const char * const cefName   =  cefTemplateData [i].first;
			const ClientEffectTemplate *& ceft = *cefTemplateData [i].second.first;
			const int column             =  cefTemplateData [i].second.second;
			
			const std::string & stringValue = ms_weaponEffectDataTable->getStringValue(column, row);
			
			if (stringValue.empty ())
			{
				WARNING (true, ("ClientWeaponObjectTemplate empty cef template name for [%s], cef [%s]", getName (), cefName));
			}
			else
			{
				ceft = ClientEffectTemplateList::fetch (CrcLowerString (stringValue.c_str ()));
			}
		}
	}
	else
		DEBUG_WARNING(true, ("Projectile weapon effect [%s] not found in weapon.tab or it is not set for object template: %s", m_lookUpId.c_str (), getName()));
}

/**
 * Creates a ClientWeaponObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate* ClientWeaponObjectTemplate::create (const std::string& filename)
{
	return new ClientWeaponObjectTemplate (filename);
}

//-------------------------------------------------------------------

const ClientEffectTemplate* ClientWeaponObjectTemplate::getFireClientEffectTemplate() const
{
	return m_fireClientEffectTemplate;
}

//===================================================================
