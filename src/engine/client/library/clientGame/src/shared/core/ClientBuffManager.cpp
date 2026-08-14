//======================================================================
//
// ClientBuffManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientBuffManager.h"

#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiIconManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/ProsePackage.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"
#include "sharedFoundation/Exitchain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include <vector>

//======================================================================

namespace ClientBuffManagerNamespace
{
	bool m_installed = false;

	const int ms_numEffects = 5;

	const float BUFF_PERSIST_THRESHOLD_IN_SECONDS = 60.0f * 5.0f;

	const std::string BUFF_STRING_FILE("ui_buff");

	const StringId BUFF_TIME_REMAINING_STRING(BUFF_STRING_FILE, "time_remaining");
	const StringId BUFF_TIME_REMAINING_LARGE_STRING(BUFF_STRING_FILE, "time_remaining_large");
	
	const std::string EFFECT_STRING_FILE("ui_effect");

	DataTable *ms_buffDatatable = NULL;
	DataTable *ms_internalBuffDatatable = NULL;

	struct BuffRecord
	{
		std::string name;
		uint32 nameCrc;
		uint32 group1Crc;
		uint32 group2Crc;
		uint32 maxStacks;
		int priority;
		int state;
		float duration;
		std::string callback;
		bool visible;
		std::string iconName;
		std::string effectParamString[ms_numEffects];
		uint32 effectParamStringCrc[ms_numEffects];
		float effectValue[ms_numEffects];
		bool debuff;
		bool isCelestial;
		bool isDispellable;
		int displayOrder;
	};

	const std::string ms_columnName("NAME");
	const std::string ms_columnGroup1("GROUP1");
	const std::string ms_columnGroup2("GROUP2");
	const std::string ms_columnPriority("PRIORITY");
	const std::string ms_columnDuration("DURATION");
	const std::string ms_columnCallback("CALLBACK");
	const std::string ms_columnVisible("VISIBLE");
	const std::string ms_columnIcon("ICON");
	const std::string ms_columnEffect1("EFFECT1_PARAM");
	const std::string ms_columnState("STATE");
	const std::string ms_columnDebuff("DEBUFF");
	const std::string ms_columnMaxStacks("MAX_STACKS");
	const std::string ms_columnIsCelestial("IS_CELESTIAL");
	const std::string ms_columnIsDispellable("DISPELL_PLAYER");
	const std::string ms_columnDisplayOrder("DISPLAY_ORDER");

	std::unordered_map<uint32, BuffRecord> ms_buffRecords;

	struct EffectRecord
	{
		std::string name;
		uint32 nameCrc;
		std::string type;
		std::string subType;
		std::string other;
	};

	const std::string ms_columnInternalName = "NAME";
	const std::string ms_columnInternalType = "TYPE";
	const std::string ms_columnInternalSubType = "SUBTYPE";
	const std::string ms_columnInternalOther = "OTHER";

	std::map<uint32, EffectRecord> ms_effectRecords;

	const std::string ATTRIB_TYPE_HEALTH = "health";
	const std::string ATTRIB_TYPE_STAMINA = "stamina";
	const std::string ATTRIB_TYPE_CONSTITUTION = "constitution";
	const std::string ATTRIB_TYPE_WILLPOWER = "willpower";
	
	const std::string BUFF_HANDLER_SCRIPT = "systems.buff.buff_handler";
	const std::string BUFF_HANDLER_ADD_TRIGGER = "AddBuffHandler";
	const std::string BUFF_HANDLER_REMOVE_TRIGGER = "RemoveBuffHandler";

	const Unicode::String newline = Unicode::narrowToWide("\n");
	const std::string ms_buffTableName = "datatables/buff/buff.iff";
	const std::string ms_effectTableName = "datatables/buff/effect_mapping.iff";
};

using namespace ClientBuffManagerNamespace;

//======================================================================

void ClientBuffManager::install()
{
	if(m_installed)
		return;
	m_installed = true;

	ms_buffDatatable = DataTableManager::getTable(ms_buffTableName, true);

	unsigned int const numRows = static_cast<unsigned int>(ms_buffDatatable->getNumRows());
	int const numColumns = ms_buffDatatable->getNumColumns();
	int const nameColumn = ms_buffDatatable->findColumnNumber(ms_columnName);
	int const group1Column = ms_buffDatatable->findColumnNumber(ms_columnGroup1);
	int const group2Column = ms_buffDatatable->findColumnNumber(ms_columnGroup2);
	int const priorityColumn = ms_buffDatatable->findColumnNumber(ms_columnPriority);
	int const durationColumn = ms_buffDatatable->findColumnNumber(ms_columnDuration);
	int const callbackColumn = ms_buffDatatable->findColumnNumber(ms_columnCallback);
	int const visibleColumn = ms_buffDatatable->findColumnNumber(ms_columnVisible);
	int const iconColumn = ms_buffDatatable->findColumnNumber(ms_columnIcon);
	int const effect1Column = ms_buffDatatable->findColumnNumber(ms_columnEffect1);
	int const stateColumn = ms_buffDatatable->findColumnNumber(ms_columnState);
	int const debuffColumn = ms_buffDatatable->findColumnNumber(ms_columnDebuff);
	int const maxStacksColumn = ms_buffDatatable->findColumnNumber(ms_columnMaxStacks);
	int const isCelestialColumn = ms_buffDatatable->findColumnNumber(ms_columnIsCelestial);
	int const isDispellableColumn = ms_buffDatatable->findColumnNumber(ms_columnIsDispellable);
	int const displayOrderColumn = ms_buffDatatable->findColumnNumber(ms_columnDisplayOrder);

	unsigned int i;
	for(i = 0; i < numRows; ++i)
	{
		BuffRecord buffRecord;

		buffRecord.name = ms_buffDatatable->getStringValue(nameColumn, i);
		buffRecord.nameCrc = ms_buffDatatable->getIntValue(nameColumn, i);
		buffRecord.group1Crc = ms_buffDatatable->getIntValue(group1Column, i);
		buffRecord.group2Crc = ms_buffDatatable->getIntValue(group2Column, i);
		buffRecord.priority = ms_buffDatatable->getIntValue(priorityColumn, i);
		buffRecord.duration = ms_buffDatatable->getFloatValue(durationColumn, i);
		buffRecord.callback = ms_buffDatatable->getStringValue(callbackColumn, i);
		buffRecord.visible = ms_buffDatatable->getIntValue(visibleColumn, i) != 0;
		buffRecord.iconName = "/Styles.Icon.";
		buffRecord.iconName.append(ms_buffDatatable->getStringValue(iconColumn, i));
		buffRecord.state = ms_buffDatatable->getIntValue(stateColumn, i);
		buffRecord.debuff = ms_buffDatatable->getIntValue(debuffColumn, i) != 0;
		buffRecord.maxStacks = ms_buffDatatable->getIntValue(maxStacksColumn, i);
		buffRecord.isCelestial = ms_buffDatatable->getIntValue(isCelestialColumn, i) == 1;
		buffRecord.isDispellable = ms_buffDatatable->getIntValue(isDispellableColumn, i) == 1;
		buffRecord.displayOrder = ms_buffDatatable->getIntValue(displayOrderColumn, i);
		
		int curColumn = effect1Column;
		int j;
		for(j = 0; j < ms_numEffects; j++)
			buffRecord.effectParamStringCrc[j] = 0;
		for(j = 0; (j < ms_numEffects) && (curColumn < numColumns); j++)
		{
			buffRecord.effectParamString[j] = ms_buffDatatable->getStringValue(curColumn, i);
			buffRecord.effectParamStringCrc[j] = Crc::calculate(buffRecord.effectParamString[j].c_str());
			curColumn++;
			buffRecord.effectValue[j] = ms_buffDatatable->getFloatValue(curColumn, i);
			curColumn++;
		}
		
		ms_buffRecords.insert(std::make_pair(buffRecord.nameCrc, buffRecord));

	}
	DataTableManager::close(ms_buffTableName);
	
	ms_internalBuffDatatable = DataTableManager::getTable(ms_effectTableName, true);

	unsigned int const numInternalRows = static_cast<unsigned int>(ms_internalBuffDatatable->getNumRows());
	int const internalNameColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalName);
	int const internalTypeColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalType);
	int const internalSubTypeColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalSubType);
	int const internalOtherColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalOther);
	
	for(i = 0; i < numInternalRows; i++)
	{
		EffectRecord effectRecord;
		
		effectRecord.name = ms_internalBuffDatatable->getStringValue(internalNameColumn, i);
		effectRecord.nameCrc = Crc::calculate(effectRecord.name.c_str());
		effectRecord.type = ms_internalBuffDatatable->getStringValue(internalTypeColumn, i);
		effectRecord.subType = ms_internalBuffDatatable->getStringValue(internalSubTypeColumn, i);
		effectRecord.other = ms_internalBuffDatatable->getStringValue(internalOtherColumn, i);

		ms_effectRecords.insert(std::make_pair(effectRecord.nameCrc, effectRecord));
	}
	DataTableManager::close(ms_effectTableName);

	ExitChain::add(ClientBuffManager::remove, "ClientBuffManager::remove", 0, false);
}

//======================================================================

void ClientBuffManager::remove()
{
	
}

//======================================================================

int ClientBuffManager::getBuffState(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.state;
}

//======================================================================

float ClientBuffManager::getBuffDefaultDuration(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0.0f;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.duration;
}

//======================================================================

bool ClientBuffManager::getBuffIsDebuff(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.debuff;
}

//======================================================================

bool ClientBuffManager::getBuffIsGroupVisible(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.visible;
}

//======================================================================

bool ClientBuffManager::getBuffGroupAndPriority(uint32 buffNameCrc, uint32 & group1Crc, uint32 & group2Crc, int & priority)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	group1Crc = buffRecord.group1Crc;
	group2Crc = buffRecord.group2Crc;
	priority = buffRecord.priority;
	return true;
}

//======================================================================

int ClientBuffManager::getBuffMaxStacks(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.maxStacks;
}

//======================================================================

bool ClientBuffManager::getBuffIsCelestial(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.isCelestial;
}

//======================================================================

bool ClientBuffManager::getBuffIsDispellable(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.isDispellable;
}

//======================================================================

int ClientBuffManager::getBuffDisplayOrder(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.displayOrder;
}


//======================================================================

UIImageStyle * ClientBuffManager::getBuffIconStyle(uint32 buffNameCrc)
{
	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return NULL;
	}
	BuffRecord const &buffRecord = it->second;

	UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (buffRecord.iconName.c_str(), TUIImageStyle));

	if (!imageStyle)
		imageStyle = CuiIconManager::getFallback ();

	return imageStyle;	
}

//======================================================================

void ClientBuffManager::getBuffDescription(Buff const & buff, Unicode::String &result)
{
	// Start with the default description, looked up from the default buff string file
	// Then add a line for each effect, based on a prose package with a %DF and using the effect's value

	std::unordered_map<uint32, BuffRecord>::const_iterator it = ms_buffRecords.find(buff.m_nameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buff.m_nameCrc));
		return;
	}
	BuffRecord const &buffRecord = it->second;

	std::string buffName = buffRecord.name;
	buffName = Unicode::toLower(buffName);
	StringId buffNameId(BUFF_STRING_FILE, buffName);

	result.append(buffNameId.localize());
	
	// Then add a line for each effect, based on a prose package with a %DF and using the effect's value
	for(int effectNum = 0; effectNum < ms_numEffects; effectNum++)
	{
		uint32 effectCrc = buffRecord.effectParamStringCrc[effectNum];
		if(effectCrc)
		{
			result.append(newline);
			std::map<uint32, EffectRecord>::iterator eit = ms_effectRecords.find(effectCrc);
			if(eit == ms_effectRecords.end())
			{
				WARNING(true, ("Unknown effect crc %d (%s)\n", effectCrc, buffRecord.effectParamString[effectNum].c_str()));
				return;
			}
			EffectRecord const &effectRecord = eit->second;

			char tmp[512];
			snprintf(tmp, 512, "%s_%s", effectRecord.type.c_str(), effectRecord.subType.c_str());
			std::string tmpS(tmp);
			tmpS = Unicode::toLower(tmpS);
			
			ProsePackage effectPackage;
			effectPackage.stringId = StringId(EFFECT_STRING_FILE, tmpS);
			if((effectNum == 0) && (buff.m_value != 0.0f))
				effectPackage.digitFloat = buff.m_value * buff.m_stackCount;
			else
				effectPackage.digitFloat = buffRecord.effectValue[effectNum] * buff.m_stackCount;
			
			ProsePackageManagerClient::appendTranslation(effectPackage, result);			
		}
	}
}	

//======================================================================

void ClientBuffManager::addTimestampToBuffDescription(Unicode::String const & description, int timeLeft, Unicode::String & result)
{
	if(timeLeft < 120)
	{	
		ProsePackage remainingTimePackage;
		remainingTimePackage.stringId = BUFF_TIME_REMAINING_STRING;
		remainingTimePackage.digitInteger = timeLeft;
		ProsePackageManagerClient::appendTranslation(remainingTimePackage, result);
	}
	else
	{
		static Unicode::String colon(Unicode::narrowToWide(":"));
		static Unicode::String zero(Unicode::narrowToWide("0"));
		int hours = timeLeft / 3600;
		int minutes = (timeLeft / 60) - hours * 60;
		int seconds = timeLeft % 60;
		result.append(BUFF_TIME_REMAINING_LARGE_STRING.localize());
		Unicode::String temp;
		UIUtils::FormatInteger(temp, hours);
		if(hours < 10) 
			result.append(zero);
		result.append(temp);
		result.append(colon);
		UIUtils::FormatInteger(temp, minutes);
		if(minutes < 10)
			result.append(zero);
		result.append(temp);
		result.append(colon);
		UIUtils::FormatInteger(temp, seconds);
		if(seconds < 10)			
			result.append(zero);
		result.append(temp);		
	}
	result.append(newline);
	result.append(description);
}

//======================================================================
