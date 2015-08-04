// ============================================================================
//
// PlanetWatcherUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "PlanetWatcherUtility.h"

#include "serverGame/ServerArmorTemplate.h"
#include "serverGame/ServerBattlefieldMarkerObjectTemplate.h"
#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerCityObjectTemplate.h"
#include "serverGame/ServerConstructionContractObjectTemplate.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerDraftSchematicObjectTemplate.h"
#include "serverGame/ServerFactoryObjectTemplate.h"
#include "serverGame/ServerGroupObjectTemplate.h"
#include "serverGame/ServerGuildObjectTemplate.h"
#include "serverGame/ServerHarvesterInstallationObjectTemplate.h"
#include "serverGame/ServerInstallationObjectTemplate.h"
#include "serverGame/ServerIntangibleObjectTemplate.h"
#include "serverGame/ServerManufactureInstallationObjectTemplate.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMissionObjectTemplate.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerShipObjectTemplate.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUniverseObjectTemplate.h"
#include "serverGame/ServerVehicleObjectTemplate.h"
#include "serverGame/ServerWeaponObjectTemplate.h"
#include "serverGame/ServerXpManagerObjectTemplate.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/ExitChain.h"

#include <map>
#include <string>

// ============================================================================
//
// PlanetWatcherUtility
//
// ============================================================================

QTextEdit *PlanetWatcherUtility::m_outputWindow = NULL;

namespace PlanetWatcherUtilityNamespace
{
	std::map<int, std::string> tagToNameMap;

	void initializeTagToNameMap()
	{
		if (tagToNameMap.empty())
		{
			tagToNameMap[ServerArmorTemplate::ServerArmorTemplate_tag] = std::string("Armor");
			tagToNameMap[ServerBattlefieldMarkerObjectTemplate::ServerBattlefieldMarkerObjectTemplate_tag] = std::string("BattlefieldMarker");
			tagToNameMap[ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag] = std::string("Building");
			tagToNameMap[ServerCellObjectTemplate::ServerCellObjectTemplate_tag] = std::string("Cell");
			tagToNameMap[ServerCityObjectTemplate::ServerCityObjectTemplate_tag] = std::string("City");
			tagToNameMap[ServerConstructionContractObjectTemplate::ServerConstructionContractObjectTemplate_tag] = std::string("Construction");
			tagToNameMap[ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag] = std::string("Creature");
			tagToNameMap[ServerDraftSchematicObjectTemplate::ServerDraftSchematicObjectTemplate_tag] = std::string("DraftSchematic");
			tagToNameMap[ServerFactoryObjectTemplate::ServerFactoryObjectTemplate_tag] = std::string("Factory");
			tagToNameMap[ServerGroupObjectTemplate::ServerGroupObjectTemplate_tag] = std::string("Group");
			tagToNameMap[ServerGuildObjectTemplate::ServerGuildObjectTemplate_tag] = std::string("Guild");
			tagToNameMap[ServerHarvesterInstallationObjectTemplate::ServerHarvesterInstallationObjectTemplate_tag] = std::string("Harvester");
			tagToNameMap[ServerInstallationObjectTemplate::ServerInstallationObjectTemplate_tag] = std::string("Installation");
			tagToNameMap[ServerIntangibleObjectTemplate::ServerIntangibleObjectTemplate_tag] = std::string("Intangible");
			tagToNameMap[ServerManufactureInstallationObjectTemplate::ServerManufactureInstallationObjectTemplate_tag] = std::string("ManufactureInstallation");
			tagToNameMap[ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag] = std::string("ManufactureSchematic");
			tagToNameMap[ServerMissionObjectTemplate::ServerMissionObjectTemplate_tag] = std::string("Mission");
			tagToNameMap[ServerObjectTemplate::ServerObjectTemplate_tag] = std::string("Object");
			tagToNameMap[ServerPlanetObjectTemplate::ServerPlanetObjectTemplate_tag] = std::string("Planet");
			tagToNameMap[ServerPlayerObjectTemplate::ServerPlayerObjectTemplate_tag] = std::string("Player");
			tagToNameMap[ServerResourceContainerObjectTemplate::ServerResourceContainerObjectTemplate_tag] = std::string("ResourceContainer");
			tagToNameMap[ServerShipObjectTemplate::ServerShipObjectTemplate_tag] = std::string("Ship");
			tagToNameMap[ServerStaticObjectTemplate::ServerStaticObjectTemplate_tag] = std::string("Static");
			tagToNameMap[ServerTangibleObjectTemplate::ServerTangibleObjectTemplate_tag] = std::string("Tangible");
			tagToNameMap[ServerUniverseObjectTemplate::ServerUniverseObjectTemplate_tag] = std::string("Universe");
			tagToNameMap[ServerVehicleObjectTemplate::ServerVehicleObjectTemplate_tag] = std::string("Vehicle");
			tagToNameMap[ServerWeaponObjectTemplate::ServerWeaponObjectTemplate_tag] = std::string("Weapon");
			tagToNameMap[ServerXpManagerObjectTemplate::ServerXpManagerObjectTemplate_tag] = std::string("XpManager");
		}
	}

	char const * ms_serverObjectTemplateCrcStringTableName = "misc/object_template_crc_string_table.iff";
	CrcStringTable * ms_serverObjectTemplateCrcStringTable = NULL;
};

using namespace PlanetWatcherUtilityNamespace;

//-----------------------------------------------------------------------------
bool PlanetWatcherUtility::install()
{
	ExitChain::add(&remove, "PlanetWatcherUtility::remove");

	if(!TreeFile::exists(ms_serverObjectTemplateCrcStringTableName))
	{
		return false;
	}

	ms_serverObjectTemplateCrcStringTable = new CrcStringTable(ms_serverObjectTemplateCrcStringTableName);
	initializeTagToNameMap();
	return true;
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::remove()
{
	delete ms_serverObjectTemplateCrcStringTable;
	ms_serverObjectTemplateCrcStringTable = NULL;
}

//-----------------------------------------------------------------------------

std::vector<std::string> PlanetWatcherUtility::getAllValidTagTypes()
{
	std::vector<std::string> result;
	for(std::map<int, std::string>::const_iterator i = tagToNameMap.begin(); i != tagToNameMap.end(); ++i)
	{
		result.push_back(i->second);
	}
	return result;
}

//-----------------------------------------------------------------------------

char const * PlanetWatcherUtility::getServerObjectTemplateCrcStringTableName()
{
	return ms_serverObjectTemplateCrcStringTableName;
}

//-----------------------------------------------------------------------------
char const * PlanetWatcherUtility::getServerObjectTemplateName(uint32 const serverObjectTemplateCrc)
{
	if(!ms_serverObjectTemplateCrcStringTable)
		return NULL;

	return ms_serverObjectTemplateCrcStringTable->lookUp(serverObjectTemplateCrc).getString();
}

//-----------------------------------------------------------------------------
bool PlanetWatcherUtility::serverTemplateCrcLookupsAreEnabled()
{
	return ms_serverObjectTemplateCrcStringTableName != NULL;
}

//-----------------------------------------------------------------------------
std::string PlanetWatcherUtility::dumpObjectInfo(PlanetWatcherRenderer::MiniMapObject const &object)
{
	std::string templateData;

	char buffer[1024];
	snprintf(buffer, sizeof(buffer)-1, "%x", object.getTemplateCrc());
	buffer[sizeof(buffer)-1] = '\0';

	templateData = buffer;

	if(PlanetWatcherUtility::serverTemplateCrcLookupsAreEnabled())
	{
		templateData += " ";
		templateData += PlanetWatcherUtility::getServerObjectTemplateName(object.getTemplateCrc());
	}

	snprintf(buffer, sizeof(buffer)-1, "%s (%s), level (%d), hibernating(%s), template(%s), at (%g,%g), auth server (%lu) aiActivity(%s)",
	  object.m_objectId.getValueString().c_str(),
	  PlanetWatcherUtility::getObjectTypeName(object.m_objectTypeTag),
	  object.getLevel(),
	  object.getHibernating() ? "true" : "false",
	  templateData.c_str(),
	  object.getWorldX(),
	  object.getWorldZ(),
	  object.m_gameServerId,
	  object.getAiActivityString().c_str()
		);

	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::setOutputWindow(QTextEdit &outputWindow)
{
	m_outputWindow = &outputWindow;
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::report(QString const &text, bool const showTime)
{
	NOT_NULL(m_outputWindow);
	
	if ((m_outputWindow != NULL) && !text.isEmpty())
	{
		QString output;
		
		if (showTime)
		{
			output += QTime::currentTime().toString(Qt::LocalDate);
			output += " - ";
		}

		output += text;

		m_outputWindow->append(output);
		m_outputWindow->scrollToBottom();
	}
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::clearOutputTextWindow()
{
	m_outputWindow->clear();
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::saveWidget(QWidget const &widget)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	QWidget const *parent = (widget.parentWidget() == NULL) ? &widget : widget.parentWidget();

	int const x = parent->pos().x();
	int const y = parent->pos().y();
	int const w = widget.width();
	int const h = widget.height();

	char text[256];
	sprintf(text, "%s_PositionX", widget.name());
	settings.writeEntry(text, x);

	sprintf(text, "%s_PositionY", widget.name());
	settings.writeEntry(text, y);

	sprintf(text, "%s_Width", widget.name());
	settings.writeEntry(text, w);

	sprintf(text, "%s_Height", widget.name());
	settings.writeEntry(text, h);
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth, int const defaultHeight)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	char text[256];
	sprintf(text, "%s_PositionX", widget.name());
	int x = settings.readNumEntry(text, defaultX);
	x = (x > 10000) ? defaultX : x;

	sprintf(text, "%s_PositionY", widget.name());
	int y = settings.readNumEntry(text, defaultY);
	y = (y > 10000) ? defaultY : y;

	sprintf(text, "%s_Width", widget.name());
	int w = settings.readNumEntry(text, defaultWidth);
	w = (w > 10000) ? defaultWidth : w;

	sprintf(text, "%s_Height", widget.name());
	int h = settings.readNumEntry(text, defaultHeight);
	h = (h > 10000) ? defaultHeight : h;

	widget.move(x, y);
	widget.resize(w, h);
	widget.show();
}

//-----------------------------------------------------------------------------
const char * PlanetWatcherUtility::getObjectTypeName(int objectTypeTag)
{
	std::map<int, std::string>::const_iterator iter = tagToNameMap.find(objectTypeTag);
	if (iter != tagToNameMap.end())
		return iter->second.c_str();

	char buffer[32];
	const char * const tag = reinterpret_cast<const char *>(&objectTypeTag);
	snprintf(buffer, sizeof(buffer)-1, "TAG(%c,%c,%c,%c)", tag[3], tag[2], tag[1], tag[0]);
	buffer[sizeof(buffer)-1] = '\0';

	tagToNameMap[objectTypeTag] = std::string(buffer);

	return tagToNameMap[objectTypeTag].c_str();
}

//-----------------------------------------------------------------------------
bool PlanetWatcherUtility::getObjectTypeTagFromName(const char * objectTypeName, int & objectTypeTag)
{
	for (std::map<int, std::string>::const_iterator iter = tagToNameMap.begin(); iter != tagToNameMap.end(); ++iter)
	{
		if (strcmp(iter->second.c_str(), objectTypeName) == 0)
		{
			objectTypeTag = iter->first;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
int PlanetWatcherUtility::getRandomObjectType()
{
	int index = Random::random(0, tagToNameMap.size() - 1);
	std::map<int, std::string>::const_iterator iter = tagToNameMap.begin();
	for (int i = 0; i < index && iter != tagToNameMap.end(); ++iter, ++i)
	{}

	return iter->first;
}

//-----------------------------------------------------------------------------
QString PlanetWatcherUtility::getSearchPath()
{
	return "/SWG/PlanetWatcher";
}

//-----------------------------------------------------------------------------
void PlanetWatcherUtility::writeEntry(QString const &location, QString const &value)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());
	settings.writeEntry(location, value);
}

//-----------------------------------------------------------------------------
QString PlanetWatcherUtility::readEntry(QString const &location, QString const &defaultValue)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());
	return settings.readEntry(location, defaultValue);
}

// ============================================================================
