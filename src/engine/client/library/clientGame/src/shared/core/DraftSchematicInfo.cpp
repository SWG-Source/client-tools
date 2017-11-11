//======================================================================
//
// DraftSchematicInfo.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DraftSchematicInfo.h"

#include "UIClipboard.h"
#include "UIUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/Universe.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/AttributeListMessage.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsData.h"
#include "sharedObject/ObjectTemplateList.h"

#include <vector>
#include <map>

//======================================================================

namespace
{
	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const DraftSchematicInfo::Messages::Changed::Payload &, DraftSchematicInfo::Messages::Changed> 
			changed;
	}

	const uint16 s_magic_ending_char = 16384;

	//----------------------------------------------------------------------

	static const std::string draft_fallbackObject = "object/tangible/crafting/shared_default_draft_not_found.iff";

	const Unicode::String keyColor             = Unicode::narrowToWide ("\\#pcontrast1 ");
	const Unicode::String keyColor2            = Unicode::narrowToWide ("\\#pcontrast3 ");
	const Unicode::String indent               = Unicode::narrowToWide (":   \\>024\\#.");
	const Unicode::String indent_big           = Unicode::narrowToWide (":   \\>128\\#.");
	const Unicode::String front_indent_small   = Unicode::narrowToWide ("\\>012\\#.");
	const Unicode::String front_indent_big     = Unicode::narrowToWide ("\\>024\\#.");
	const Unicode::String unindent             = Unicode::narrowToWide ("\\>000\n");

	const Unicode::String redColor             = Unicode::narrowToWide ("\\#ff0000");
	const Unicode::String greenColor           = Unicode::narrowToWide ("\\#00ff00");
	const Unicode::String yellowColor          = Unicode::narrowToWide ("\\#ffff00");

	const Unicode::String colorSlotName        = Unicode::narrowToWide ("\\#ffff88");

	struct TypeToAttrib
	{
		std::string id;
		Unicode::String localName;
	};

	//TODO this should be a std::map
	static TypeToAttrib s_typeToAttribMap[11];	
	static const int s_numTypes = 11;
	static bool s_typesInstalled = false;

}

//----------------------------------------------------------------------

struct DraftSchematicInfo::CachedAttribWeights
{
	typedef std::pair<int, int>                              WeightPercentPair;
	typedef stdmap<Unicode::String,  WeightPercentPair>::fwd LocalWeightMap;

	int               attribIndex;
	int               attribMin;
	int               attribMax;
	int               attribSlot;
	
	int               totalWeights;
	LocalWeightMap    weights;
};

//----------------------------------------------------------------------

DraftSchematicInfo::DraftSchematicInfo   () :
m_draftSchematicTemplate     (),
m_craftedSharedTemplate      (),
m_clientObject               (0),
m_slots                      (new SlotVector),
m_lastFrameUpdate            (0),
m_slotsRequested             (false),
m_slotsReceived              (false),
m_complexity                 (0),
m_manufactureSchematicVolume (0),
m_cachedExpAttribWeightMap   (new CachedExpAttribWeightMap),
m_assemblyWeights            (new AttribWeightVector),
m_resourceMaxWeights         (new AttribWeightVector),
m_weightsRequested           (false),
m_weightsReceived            (false)
{
}

//----------------------------------------------------------------------

DraftSchematicInfo::DraftSchematicInfo   (uint32 serverCrc, uint32 sharedCrc) :
m_draftSchematicTemplate     (serverCrc, sharedCrc),
m_craftedSharedTemplate      (),
m_clientObject               (0),
m_slots                      (new SlotVector),
m_lastFrameUpdate            (0),
m_slotsRequested             (false),
m_slotsReceived              (false),
m_complexity                 (0),
m_manufactureSchematicVolume (0),
m_cachedExpAttribWeightMap   (new CachedExpAttribWeightMap),
m_assemblyWeights            (new AttribWeightVector),
m_resourceMaxWeights         (new AttribWeightVector),
m_weightsRequested           (false),
m_weightsReceived            (false)
{
}

//----------------------------------------------------------------------

DraftSchematicInfo::DraftSchematicInfo(const std::pair<uint32, uint32> & crc) :
m_draftSchematicTemplate     (crc),
m_craftedSharedTemplate      (),
m_clientObject               (0),
m_slots                      (new SlotVector),
m_lastFrameUpdate            (0),
m_slotsRequested             (false),
m_slotsReceived              (false),
m_complexity                 (0),
m_manufactureSchematicVolume (0),
m_cachedExpAttribWeightMap   (new CachedExpAttribWeightMap),
m_assemblyWeights            (new AttribWeightVector),
m_resourceMaxWeights         (new AttribWeightVector),
m_weightsRequested           (false),
m_weightsReceived            (false)
{
}

//----------------------------------------------------------------------

DraftSchematicInfo::~DraftSchematicInfo ()
{
	delete m_cachedExpAttribWeightMap;
	delete m_assemblyWeights;
	delete m_resourceMaxWeights;

	m_cachedExpAttribWeightMap = 0;
	m_assemblyWeights       = 0;
	m_resourceMaxWeights    = 0;

	delete m_clientObject;
	m_clientObject = 0;

	delete m_slots;
	m_slots = 0;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::createClientObject   () const
{
	if (m_clientObject)
		return;
	
	const SharedDraftSchematicObjectTemplate * sdsot = NULL;
	const ConstCharCrcString & schematicName = ObjectTemplateList::lookUp(
		getSharedDraftSchematicTemplate());
	if (!schematicName.isEmpty())
	{
		sdsot = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(ObjectTemplateList::fetch(schematicName));
	}
	
	if (!sdsot)
		WARNING (true, ("DraftSchematicInfo could not find SharedDraftSchematicTemplate [%s]", schematicName.getString()));
	else
	{
		const SharedObjectTemplate * const sot = sdsot->fetchCraftedSharedObjectTemplate ();
		
		if (!sot)
			WARNING (true, ("DraftSchematicInfo could not fetch CraftedSharedObjectTemplate from [%s]", schematicName.getString()));
		else
		{
			m_clientObject = safe_cast <ClientObject *>(sot->createObject ());

			if (!m_clientObject)
				WARNING (true, ("DraftSchematicInfo failed to create object from [%s] for [%s]", sot->DataResource::getName (), schematicName.getString()));
			else
			{
				m_clientObject->endBaselines ();
				m_clientObject->setObjectNameStringId (sdsot->getCraftedName ());		
				m_clientObject->setNetworkId(ClientObject::getNextFakeNetworkId());
			}
			
			sot->releaseReference ();
		}
		sdsot->releaseReference ();
	}

	if (!m_clientObject)
	{
		m_clientObject = safe_cast<ClientObject *>(ObjectTemplate::createObject (draft_fallbackObject.c_str ()));	
		m_clientObject->endBaselines ();

		DEBUG_FATAL (!m_clientObject, ("DraftSchematicInfo could not create fallback object [%s]\n", draft_fallbackObject.c_str ()));

		Unicode::String localizedName = m_clientObject->getLocalizedName ();
		localizedName.append (1, ' ');
		localizedName.append (1, '(');
		localizedName.append (Unicode::narrowToWide (schematicName.getString()));
		localizedName.append (1, ')');
		
		m_clientObject->setObjectName (localizedName);
		m_clientObject->setNetworkId(ClientObject::getNextFakeNetworkId());
	}
	
	{
		Appearance * const app = m_clientObject->getAppearance ();
		
		SkeletalAppearance2 * const skelApp = app ? app->asSkeletalAppearance2 () : 0;
		
		if (skelApp)
		{
			//-- preload all lod assets
			const int count = skelApp->getDetailLevelCount ();
			for (int i = 0; i < count && i < 2; ++i)
			{
				skelApp->isDetailLevelAvailable (i);
			}
		}
	}
	
	if (Game::getSinglePlayer ())
	{
		SlotVector sv;

		Slot   slot;
		Option option;

		slot.name = StringId ("table_slot", "string_1");
		slot.optional = false;

		option.name         = StringId ("table_option", "option_1");
		option.ingredient   = Unicode::narrowToWide ("copper");
		option.type         = Crafting::IT_resourceClass;
		option.amountNeeded = 500;

		slot.options.push_back (option);

		option.name         = StringId ("table_option", "option_2");
		option.ingredient   = Unicode::narrowToWide ("carbonite");
		option.type         = Crafting::IT_resourceClass;
		option.amountNeeded = 100;

		slot.options.push_back (option);

		sv.push_back (slot);

		slot.options.clear ();

		slot.name = StringId ("table_slot", "string_2");

		option.name       = StringId ("table_option", "option_3");
		option.ingredient = Unicode::narrowToWide ("object/creature/player/shared_rodian_male.iff");
		option.type       = Crafting::IT_template;
		
		slot.options.push_back (option);

		sv.push_back (slot);

		const_cast<DraftSchematicInfo *>(this)->setSlots (sv);

		m_lastFrameUpdate = Game::getLoopCount ();
	}
}

//----------------------------------------------------------------------

const Unicode::String & DraftSchematicInfo::getLocalizedName     () const
{
	const ClientObject * const clientObject = getClientObject ();
	if (clientObject)
		return clientObject->getLocalizedName ();

	return Unicode::emptyString;
}

//----------------------------------------------------------------------

bool DraftSchematicInfo::formatDescription  (Unicode::String & header, Unicode::String & desc, Unicode::String & attribs, bool minimalAttribs, ResourceContainerObject* considerResource) const
{
	const ClientObject * const clientObject = getClientObject ();
	if (clientObject)
	{
		ObjectAttributeManager::formatHeaderAndDesc (*clientObject, header, desc);
		header.append (1, '\n');
		header.append (GameObjectTypes::getLocalizedName (SharedObjectTemplate::GOT_data));

		formatAttribs (attribs, minimalAttribs, considerResource);

		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::formatAttribs (Unicode::String & attribs, bool minimal, ResourceContainerObject *considerResource) const
{
#if PRODUCTION == 0
	if (CuiPreferences::getDebugClipboardExamine ())
	{
		attribs += Unicode::narrowToWide ("Draft Template:   \\>024")    + Unicode::narrowToWide (ObjectTemplateList::lookUp(getServerDraftSchematicTemplate()).getString()) + Unicode::narrowToWide ("\\>000\n");
		attribs.push_back ('\n');

		UIClipboard::gUIClipboard ().SetText (attribs);

//		attribs += Unicode::narrowToWide ("Crafted Template:   \\>024")  + Unicode::narrowToWide (m_craftedSharedTemplate) + Unicode::narrowToWide ("\\>000\n");
	}

#endif

	if (m_slotsReceived)
	{
		Unicode::String one, two, three;

		//----------------------------------------------------------------------
		//-- complexity

		attribs.append (keyColor);
		attribs.append (StringId (ObjectAttributeManager::ms_nameStringTable, SharedObjectAttributes::complexity).localize ());

		UIUtils::FormatInteger (one, m_complexity);

		attribs.append    (indent);
		attribs.append    (one);
		attribs.append    (unindent);

		static const float COMPLEXITY_LIMIT [] = 
		{
			15, 
			20,
			25
		};

		if (m_complexity <= COMPLEXITY_LIMIT [0])
			attribs += CuiStringIdsCraft::complexity_level_0.localize ();
		else if (m_complexity <= COMPLEXITY_LIMIT [1])
			attribs += CuiStringIdsCraft::complexity_level_1.localize ();
		else
			attribs += CuiStringIdsCraft::complexity_level_2.localize ();

		attribs.push_back ('\n');

		//----------------------------------------------------------------------
		//-- volume

		attribs.append (keyColor);
		attribs.append (StringId (ObjectAttributeManager::ms_nameStringTable, SharedObjectAttributes::manufacture_schematic_volume).localize ());

		UIUtils::FormatInteger (one, m_manufactureSchematicVolume);

		attribs.append    (indent);
		attribs.append    (one);
		attribs.append    (unindent);


		//----------------------------------------------------------------------
		
		for (SlotVector::const_iterator it = m_slots->begin (); it != m_slots->end (); ++it)
		{
			const Slot & slot = *it;
			createSlotDescription (slot, 0, one, two, three);
			
			attribs.append (one);
			attribs.push_back ('\n');

			if (!minimal)
				attribs.append (three);
		}
	}
	else
		attribs = CuiStringIdsCraft::draft_slots_waiting_data.localize ();

	//----------------------------------------------------------------------

	if (!minimal)
	{
		attribs.push_back ('\n');

		Unicode::String draftAttribs;

		formatDraftAttribWeights (draftAttribs, Unicode::emptyString, true, considerResource);
		
		attribs += draftAttribs;
	}
	
	//----------------------------------------------------------------------

}

//----------------------------------------------------------------------

bool DraftSchematicInfo::formatDraftAttribWeights (Unicode::String & draftAttribs, const Unicode::String & expNameFilter, bool showWeights, ResourceContainerObject* considerResource) const 
{
	if(!s_typesInstalled)
	{		
		s_typeToAttribMap[0].id = "entangle_resistance";
		s_typeToAttribMap[1].id = "res_cold_resist";
		s_typeToAttribMap[2].id = "res_conductivity";
		s_typeToAttribMap[3].id = "res_decay_resist";
		s_typeToAttribMap[4].id = "res_flavor";
		s_typeToAttribMap[5].id = "res_heat_resist";
		s_typeToAttribMap[6].id = "res_malleability";
		s_typeToAttribMap[7].id = "res_potential_energy";
		s_typeToAttribMap[8].id = "res_quality";
		s_typeToAttribMap[9].id = "res_shock_resistance";
		s_typeToAttribMap[10].id = "res_toughness";
		
		for(int i = 0; i < s_numTypes; i++)
		{
			s_typeToAttribMap[i].localName = StringId("obj_attr_n", s_typeToAttribMap[i].id).localize();
		}
		s_typesInstalled = true;
	}
	if (m_weightsReceived)
	{
		for (CachedExpAttribWeightMap::const_iterator it = m_cachedExpAttribWeightMap->begin (); it != m_cachedExpAttribWeightMap->end (); ++it)
		{
			const Unicode::String & expAttribName = (*it).first;
			const CachedAttribWeightMap & cawMap  = (*it).second;
			
			if (cawMap.empty ())
				continue;
			
			if (!expNameFilter.empty () && expNameFilter != expAttribName)
				continue;
			
			draftAttribs.append (keyColor2);
			if (!expAttribName.empty () && expAttribName [0] == s_magic_ending_char)
				draftAttribs += CuiStringIdsCraft::draft_attrib_misc.localize ();
			else
				draftAttribs += expAttribName;
			
			draftAttribs += unindent;
			
			AttributeListMessage::AttributeVector resourceAttribs;
			if(considerResource)
				ObjectAttributeManager::getAttributes(considerResource->getResourceType(), resourceAttribs, false);
			
			static char buf [64];
			static const int buf_size = sizeof (buf);
			
			const Unicode::String &resourceName = considerResource ? considerResource->getResourceName() : Unicode::emptyString;
			
			for (CachedAttribWeightMap::const_iterator cit = cawMap.begin (); cit != cawMap.end (); ++cit)
			{
				const Unicode::String & attribName = (*cit).first;
				const CachedAttribWeights & caw    = (*cit).second;
				
				if (caw.attribMin != caw.attribMax)
				{
					draftAttribs += front_indent_small;
					draftAttribs.append (keyColor);
					draftAttribs += attribName;
					if (m_slotsReceived && m_slots != NULL && caw.attribSlot >= 0 && 
						caw.attribSlot < static_cast<int>(m_slots->size()))
					{
						draftAttribs += colorSlotName;
						draftAttribs += Unicode::narrowToWide(" (");
						draftAttribs += (*m_slots)[caw.attribSlot].name.localize();
						draftAttribs += Unicode::narrowToWide(")");
					}
					draftAttribs += unindent;
					
					if (showWeights)
					{
						for (CachedAttribWeights::LocalWeightMap::const_iterator lit = caw.weights.begin (); lit != caw.weights.end (); ++lit)
						{
							const Unicode::String & resourceAttribName        = (*lit).first;
							const CachedAttribWeights::WeightPercentPair & wp = (*lit).second;
							
							snprintf (buf, buf_size, "%d%%", wp.second);							
							draftAttribs += front_indent_big;
							draftAttribs += resourceAttribName;
							draftAttribs += indent_big;
							draftAttribs += Unicode::narrowToWide (buf);
							draftAttribs += unindent;
							if(considerResource)
							{
								static char buf2[256];
								static const int buf2_size = sizeof (buf2);
								
								int resourceValue = -1;
								std::string mappedType;
								bool foundMapping = false;
								for(int i = 0; i < s_numTypes; i++)
								{
									if(!_wcsicmp(s_typeToAttribMap[i].localName.c_str(), resourceAttribName.c_str()))
									{
										mappedType = s_typeToAttribMap[i].id;
										foundMapping = true;
									}
								}
								DEBUG_WARNING(!foundMapping, ("Can't find mapping for resource name '%s'", Unicode::wideToNarrow(resourceAttribName).c_str()));								
								for (ObjectAttributeManager::AttributeVector::iterator it = resourceAttribs.begin(); it != resourceAttribs.end(); ++it)
								{
									ObjectAttributeManager::AttributePair & attributePair = *it;
									if(attributePair.first == mappedType)
									{
										resourceValue = atoi(Unicode::wideToNarrow(attributePair.second).c_str());									
									}
								}
								if(resourceAttribs.empty())
								{
										draftAttribs += front_indent_big;
										draftAttribs += resourceName;
										draftAttribs += indent_big;							
										draftAttribs += Unicode::narrowToWide("Loading...");
										draftAttribs += unindent;
								}
								else
								{
									if(resourceValue != -1)
									{
										float resourceContribution = resourceValue * wp.second / 100.0f;
										snprintf(buf2, buf2_size, "%d x %d%% = %4.0f", resourceValue, wp.second, resourceContribution);
										draftAttribs += front_indent_big;
										draftAttribs += resourceName;
										draftAttribs += indent_big;							
										if(resourceValue < 333)
											draftAttribs.append(redColor);
										else if(resourceValue < 666)
											draftAttribs.append(yellowColor);
										else
											draftAttribs.append(greenColor);
										draftAttribs += Unicode::narrowToWide (buf2);
										draftAttribs += unindent;
									}
									else
									{
										draftAttribs += front_indent_big;
										draftAttribs += resourceName;
										draftAttribs += indent_big;							
										draftAttribs += Unicode::narrowToWide("-- x --% = ----");
										draftAttribs += unindent;
									}
								}
							}
						}
					}
				}
			}
		}
		
		return true;
	}
	else
	{
		draftAttribs = CuiStringIdsCraft::draft_attribs_waiting_data.localize ();
		return false;
	}
}

//----------------------------------------------------------------------

void DraftSchematicInfo::setSlots (const SlotVector & sv)
{
	*m_slots = sv;
	m_lastFrameUpdate = Game::getLoopCount ();
	m_slotsReceived   = true;

	Transceivers::changed.emitMessage (*this);
}

//----------------------------------------------------------------------

StringId DraftSchematicInfo::convertNameStringIdToDescription (const StringId & id)
{		
	StringId descId (id);
	std::string tableName = descId.getTable ();
	
	const size_t tableSize = tableName.size ();
	static const std::string _n_suffix ("_n");
	static const std::string _d_suffix ("_d");
	
	if (tableSize > 2 && tableName.compare (tableSize - 2, 2, _n_suffix) == 0)
		tableName.replace (tableSize - 2, 2, _d_suffix);
	else
		tableName.append (_d_suffix);
	
	descId.setTable (tableName);	
	return descId;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::createSlotDescription (const Slot & slot, const ManufactureSchematicObject * manf_schem, Unicode::String & slotName, Unicode::String & slotDesc, Unicode::String & optionDesc)
{
	static const Unicode::String colorWhite    (Unicode::narrowToWide ("\\#ffffff"));
	static const Unicode::String colorZero     (Unicode::narrowToWide ("\\#."));
	static const Unicode::String colorRed      (Unicode::narrowToWide ("\\#ff0000"));

	//----------------------------------------------------------------------
	//-- slot name
	{
		slotName = colorSlotName + slot.name.localize ();

		if (slot.optional)
			slotName += Unicode::narrowToWide ( " (optional)");

		slotName += colorZero;
	}

	//----------------------------------------------------------------------
	//-- slot description
	{		
		const StringId & slotStringId = DraftSchematicInfo::convertNameStringIdToDescription (slot.name);
		CuiManager::setIgnoreBadStringId (slotStringId);
		if (!slotStringId.localize (slotDesc))
			slotDesc.clear ();
		CuiManager::setIgnoreBadStringId (StringId::cms_invalid);

	}
	
	//----------------------------------------------------------------------
	//-- slot option information

	Unicode::String str;
		
	//-- build slot info
	
	const MessageQueueDraftSlotsData::OptionVector & options = slot.options;
	
	if (options.empty ())
	{
		static const Unicode::String theIndent  (Unicode::narrowToWide ("\\>012"));
		str.append (theIndent);
		str.append (colorRed);
		str += CuiStringIdsCraft::draft_slot_no_options.localize ();
		str.append (colorZero);
		str.append (Unicode::narrowToWide ("\\>000\n"));
	}
	else
	{
		int optionIndex = 0;
		const bool printName = options.size () != 1;
		for (MessageQueueDraftSlotsData::OptionVector::const_iterator it = options.begin (); it != options.end (); ++it, ++optionIndex)
		{
			createOptionDescription (slot, manf_schem, optionIndex, str, printName, false);
		}
	}

	optionDesc = str;
}

//----------------------------------------------------------------------

bool DraftSchematicInfo::createOptionDescription (const Slot & slot, const ManufactureSchematicObject * manf_schem, int optionIndex, Unicode::String & str, bool printName, bool printDescription)
{
	if (optionIndex < 0 || optionIndex >= static_cast<int>(slot.options.size ()))
		return false;

	Crafting::IngredientSlot isdata;
	const bool loaded = manf_schem ? manf_schem->getIngredient (slot.name, isdata) : false;

	const MessageQueueDraftSlotsDataOption & option = slot.options [optionIndex];

	static const Unicode::String zeroIndent    (Unicode::narrowToWide ("\\>000"));
	static const Unicode::String theIndent     (Unicode::narrowToWide ("\\>012"));
	static const Unicode::String theIndent2    (Unicode::narrowToWide ("\\>024"));
	static const Unicode::String colorWhite    (Unicode::narrowToWide ("\\#ffffff"));
	static const Unicode::String colorZero     (Unicode::narrowToWide ("\\#."));

	str.append (theIndent);

	if (printName)
	{
		str.append (colorWhite);
		str.append (1, '-');
		str.append (3, ' ');
		str.append (option.name.localize ());
		str.append (colorZero);
		str.append (1, '\n');
	}

	if (printDescription)
	{
		str.append (4, ' ');
		const StringId & descId = DraftSchematicInfo::convertNameStringIdToDescription (option.name);
		str.append (descId.localize ());
		str.append (1, '\n');
	}
	
	const StringId * sidRequirement  = 0;
	const StringId * sidLoaded       = 0;
	
	Unicode::String strRequirement   = option.ingredient;
	switch (option.type)
	{
	case Crafting::IT_item:
		{
			sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_item_prose;
			sidLoaded      = &CuiStringIdsCraft::draft_slot_option_loaded_item_prose;
		}
		break;
	case Crafting::IT_template:
	case Crafting::IT_templateGeneric:
	case Crafting::IT_schematic:
	case Crafting::IT_schematicGeneric:
		{
			const SharedObjectTemplate * const ot = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch (Unicode::wideToNarrow (strRequirement)));
			
			if (ot)
			{
				strRequirement = ot->getObjectName ().localize ();
				ot->releaseReference ();
			}
			else
			{
				if (manf_schem)
				{
					WARNING (true, ("Received a slot option template ingredient with an invalid template: [%s] for draft schematic [%lu]", 
						Unicode::wideToNarrow (strRequirement).c_str (), manf_schem->getDraftSchematicSharedTemplate()));
				}
			}
			
			if (option.amountNeeded > 1)
			{
				if (option.type == Crafting::IT_templateGeneric)
					sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_item_generic_prose;
				else
					sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_item_factory_prose;
			}
			else
				sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_item_prose;

			sidLoaded      = &CuiStringIdsCraft::draft_slot_option_loaded_item_prose;
		}
		break;
	case Crafting::IT_resourceClass:
		{
			sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_resclass_prose;
			sidLoaded      = &CuiStringIdsCraft::draft_slot_option_loaded_resclass_prose;

			const ResourceClassObject * const rco = Universe::getInstance ().getResourceClassByName (Unicode::wideToNarrow (strRequirement));
			if (rco)
				strRequirement = rco->getFriendlyName ().localize();
		}
		break;
	case Crafting::IT_resourceType:
		{
			const NetworkId & typeId = ResourceTypeManager::findTypeByName (strRequirement);
			if (typeId != NetworkId::cms_invalid)
				ResourceTypeManager::createTypeDisplayLabel (typeId, strRequirement);

			sidRequirement = &CuiStringIdsCraft::draft_slot_option_req_resclass_prose;
			sidLoaded = &CuiStringIdsCraft::draft_slot_option_loaded_resclass_prose;
		}
		break;
	}
	
	CuiStringVariablesData cvd;

	bool needsReset = true;

	if (sidRequirement)
	{
		str.append (theIndent2);
		cvd.sourceName = strRequirement;
		cvd.digit_i    = option.amountNeeded;
		Unicode::String tmpStr;
		CuiStringVariablesManager::process (*sidRequirement, cvd, tmpStr);
		str += tmpStr;
		str.append (zeroIndent);
		str.push_back ('\n');
		needsReset = false;
	}
	
	if (sidLoaded)
	{
		const bool isOptionLoaded = (loaded && isdata.draftSlotOption == optionIndex && !isdata.ingredients.empty ());
		if (isOptionLoaded)
		{
			needsReset = true;
			str.append (theIndent2);

			const Crafting::SimpleIngredient * simpleIngredient = isdata.ingredients.front().get();
			NOT_NULL(simpleIngredient);
			
			const CachedNetworkId ingredientId (simpleIngredient->ingredient);
			
			int ingredientCount  = 0;//simpleIngredient->count;
			{
				for (Crafting::Ingredients::const_iterator iter = isdata.ingredients.begin(); iter != isdata.ingredients.end(); ++iter)
				{
					ingredientCount += (*iter)->count;
				}
			}
			
			Unicode::String ingredientName;
			ClientObject * const obj = safe_cast<ClientObject *>(ingredientId.getObject ());
			if (obj)
				ingredientName = obj->getLocalizedName ();
			else if (isdata.ingredientType == Crafting::IT_resourceClass || isdata.ingredientType == Crafting::IT_resourceType)
			{
				if (!ResourceTypeManager::createTypeDisplayLabel (ingredientId, ingredientName))
				{
					WARNING (true, ("Failed to find ingredient name for [%s]", ingredientId.getValueString ().c_str ()));
					ingredientName = Unicode::narrowToWide ("Unknown Resource Type");
				}
			}

			else
				ingredientName = Unicode::narrowToWide ("Unknown Object");
				
			cvd.sourceName = ingredientName;
			cvd.digit_i    = ingredientCount;
			
			Unicode::String tmpStr;
			CuiStringVariablesManager::process (*sidLoaded, cvd, tmpStr);		
			str += tmpStr;

			if (isdata.ingredientType == Crafting::IT_resourceClass || isdata.ingredientType == Crafting::IT_resourceType)
			{
				tmpStr.clear ();
				if (ObjectAttributeManager::formatAttributes (ingredientId, tmpStr, false))
				{
					str.append (1, '\n');
					str += tmpStr;
				}
			}

		}
	}

	if (needsReset)
	{
		str.append (zeroIndent);
		str.push_back ('\n');
	}

	return true;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::requestDataIfNeeded () const
{
	if (!m_slotsRequested)
	{
		m_slotsRequested = true;
		DraftSchematicManager::requestDraftSlots(this);
	}

	if (!m_weightsRequested)
	{
		m_weightsRequested = true;
		DraftSchematicManager::requestResourceWeights(this);
	}
}

//----------------------------------------------------------------------

void DraftSchematicInfo::setComplexity                    (int complexity)
{
	m_complexity = complexity;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::setManufactureSchematicVolume                    (int volume)
{
	m_manufactureSchematicVolume = volume;
}

//----------------------------------------------------------------------

void DraftSchematicInfo::setResourceWeights               (const AttribWeightVector & weights, const AttribWeightVector & resourceMaxWeights)
{
	*m_assemblyWeights     = weights;
	*m_resourceMaxWeights  = resourceMaxWeights;

	m_weightsReceived  = true;
	
	m_cachedExpAttribWeightMap->clear ();
	
	if (m_assemblyWeights->size () != m_resourceMaxWeights->size ())
	{
		WARNING (true, ("DraftSchematicInfo::setResourceWeights [%s] assembly weights size [%d] & resource max weights size [%d] don't match", 
			ObjectTemplateList::lookUp(getServerDraftSchematicTemplate()).getString(), static_cast<int>(m_assemblyWeights->size ()), static_cast<int>(m_resourceMaxWeights->size ())));
		return;
	}

	const SharedDraftSchematicObjectTemplate * sdsot = NULL;
	const ConstCharCrcString & schematicName = ObjectTemplateList::lookUp(getSharedDraftSchematicTemplate());
	if (!schematicName.isEmpty())
	{
		sdsot = dynamic_cast<const SharedDraftSchematicObjectTemplate *>(
			ObjectTemplateList::fetch(schematicName));
	}
	
	if (!sdsot)
	{
		WARNING (true, ("DraftSchematicInfo::setResourceWeights could not find SharedDraftSchematicTemplate [%s]", schematicName.getString()));
		return;
	}
	
	const int attributeCount = sdsot->getAttributesCount ();
	const int numWeights = static_cast<int>(m_assemblyWeights->size ());
	
	if (attributeCount < numWeights)
	{
		WARNING (numWeights != 0, ("DraftSchematicInfo [%s] attribute count [%d] doesn't match weights attribute count [%d]", schematicName.getString(), attributeCount, numWeights));
	}
	else
	{
		SharedDraftSchematicObjectTemplate::SchematicAttribute sattr;
		SharedDraftSchematicObjectTemplate::SchematicAttribute sattrMin;
		SharedDraftSchematicObjectTemplate::SchematicAttribute sattrMax;		
		CachedAttribWeights caw;

		for (int i = 0; i < numWeights; ++i)
		{
			const WeightVector & weightsResourceMax = (*m_resourceMaxWeights)[i].second;

			//-- this one is not important
			if (weightsResourceMax.empty ())
				continue;			

			sdsot->getAttributes(sattr,       i);
			sdsot->getAttributesMin(sattrMin, i);
			sdsot->getAttributesMax(sattrMax, i);

			//-- this one is not modifiable
			if (sattrMin.value == sattrMax.value)
				continue;
						
			//@todo: the draft schematics should be fixed, their attribute stringids are generally invalid
			const StringId fixedExperimentId (sattr.experiment.getTable (), Unicode::toLower (sattr.experiment.getText ()));
			const StringId fixedNameId       ("obj_attr_n" /*sattr.name.getTable ()*/, Unicode::toLower (sattr.name.getText ()));


			const Unicode::String & localExpName = fixedExperimentId.isValid () ? fixedExperimentId.localize () : Unicode::String (size_t (1), s_magic_ending_char);
			const Unicode::String & localName    = fixedNameId.localize ();
			
			caw.attribIndex = i;
			caw.attribMin           = sattrMin.value;
			caw.attribMax           = sattrMax.value;
			caw.attribSlot          = (*m_resourceMaxWeights)[i].first;
			caw.totalWeights        = 0;			
			caw.weights.clear ();

			for (WeightVector::const_iterator wit = weightsResourceMax.begin (); wit != weightsResourceMax.end (); ++wit)
			{
				const int resourceIndex = (*wit).first;
				const int weight        = (*wit).second;
				
				CachedAttribWeights::WeightPercentPair wp(weight, 0);
				const StringId & resourceAttributeNameStringId = Crafting::getResourceAttributeNameStringId (resourceIndex);
				const Unicode::String & resourceAttributeName  = resourceAttributeNameStringId.localize ();

				caw.weights.insert (std::make_pair (resourceAttributeName, wp));
				caw.totalWeights += weight;
			}
			
			if (caw.totalWeights)
			{
				for (CachedAttribWeights::LocalWeightMap::iterator lit = caw.weights.begin (); lit != caw.weights.end (); ++lit)
				{
					CachedAttribWeights::WeightPercentPair & wp = (*lit).second;
					wp.second = wp.first * 100 / caw.totalWeights;
				}
			}

			CachedAttribWeightMap & cawMap = (*m_cachedExpAttribWeightMap) [localExpName];
			cawMap.insert (std::make_pair (localName, caw));
		}
	}
	
	sdsot->releaseReference ();

	Transceivers::changed.emitMessage (*this);
}

//----------------------------------------------------------------------

float DraftSchematicInfo::calculateResourceMatch(const NetworkId &resourceTypeId, int slot) const
{
	if (m_weightsReceived)
	{
		const ResourceClassObject * resClass = NULL;
		if (m_slots != NULL && slot >= 0 && slot < static_cast<int>(m_slots->size()))
		{
			resClass = Universe::getInstance().getResourceClassByName(Unicode::wideToNarrow((*m_slots)[slot].options[0].ingredient));
		}

		float result = 0.0f;
		float count = 0.0f;
		for (CachedExpAttribWeightMap::const_iterator it = m_cachedExpAttribWeightMap->begin (); it != m_cachedExpAttribWeightMap->end (); ++it)
		{
			const CachedAttribWeightMap & cawMap  = (*it).second;
			
			if (cawMap.empty ())
				continue;
			
			AttributeListMessage::AttributeVector resourceAttribs;
			ObjectAttributeManager::getAttributes(resourceTypeId, resourceAttribs, false);
							
			for (CachedAttribWeightMap::const_iterator cit = cawMap.begin (); cit != cawMap.end (); ++cit)
			{
				const CachedAttribWeights & caw    = (*cit).second;
				
				if (caw.attribMin != caw.attribMax)
				{					
					for (CachedAttribWeights::LocalWeightMap::const_iterator lit = caw.weights.begin (); lit != caw.weights.end (); ++lit)
					{
						const Unicode::String & resourceAttribName        = (*lit).first;
						const CachedAttribWeights::WeightPercentPair & wp = (*lit).second;
						
						int resourceValue = -1;
						std::string mappedType;
						bool foundMapping = false;
						for(int i = 0; i < s_numTypes; i++)
						{
							if(!_wcsicmp(s_typeToAttribMap[i].localName.c_str(), resourceAttribName.c_str()))
							{
								mappedType = s_typeToAttribMap[i].id;
								foundMapping = true;
							}
						}
						DEBUG_WARNING(!foundMapping, ("Can't find mapping for resource name '%s'", Unicode::wideToNarrow(resourceAttribName).c_str()));
						for (ObjectAttributeManager::AttributeVector::iterator it = resourceAttribs.begin(); it != resourceAttribs.end(); ++it)
						{
							ObjectAttributeManager::AttributePair & attributePair = *it;
							if(attributePair.first == mappedType)
							{
								resourceValue = atoi(Unicode::wideToNarrow(attributePair.second).c_str());
								if (resClass != NULL)
								{
									// scale the resource value for the slot's required resource class
									ResourceClassObject::ResourceAttributeRangesType::const_iterator found = resClass->getResourceAttributeRanges().find(attributePair.first);
									if (found != resClass->getResourceAttributeRanges().end())
									{
										int min = (*found).second.first;
										int max = (*found).second.second;
										if ((max != min) && (max > 0))
										{
											int scaledValue = (resourceValue * 1000) / max;
											if ((resourceValue * 1000) % max)
												++scaledValue;

											if (scaledValue > 1000)
												scaledValue = 1000;
											else if (scaledValue < 1)
												scaledValue = 1;

											resourceValue = scaledValue;
										}
									}
								}

								if (resourceValue > 1000)
									resourceValue = 1000;
								else if (resourceValue < 1)
									resourceValue = 1;
							}
						}
						if(resourceValue != -1)
						{
							float resourceContribution = resourceValue * wp.second / 100.0f / 1000.0f;
							result += resourceContribution;
							count += wp.second / 100.0f;						
						}
					}					
				}
			}
		}
		if(count == 0.0f)
			return 0.0f;
		else
			return (result / count);
	}
	else
	{		
		return 0.0f;
	}
}

//======================================================================
