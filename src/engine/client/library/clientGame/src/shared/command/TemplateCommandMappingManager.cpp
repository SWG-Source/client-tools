//======================================================================
//
// TemplateCommandMappingManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/TemplateCommandMappingManager.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

namespace TemplateCommandMappingManagerNamespace
{
	bool m_installed = false;

	const std::string ms_columnTemplate = "TEMPLATE_NAME";
	const std::string ms_columnCommand = "COMMAND";
	const std::string ms_columnFakeCommandGroup = "FAKE_COMMAND_GROUP";
	const std::string ms_columnExecuteFromToolbar = "EXECUTE_FROM_TOOLBAR";

	DataTable *ms_datatable;
		
	struct Record
	{
		Record()
		: command("")
		, commandHash(0)
		, fakeCommandGroupHash(0)
		{}

		std::string command;
		uint32 commandHash;
		uint32 fakeCommandGroupHash;
		bool executeFromToolbar;
	};

	typedef std::map<uint32, Record> TemplateCommandMap;
	TemplateCommandMap s_templateCommandMap;
};

using namespace TemplateCommandMappingManagerNamespace;


void TemplateCommandMappingManager::install()
{
	if(m_installed)
		return;
	m_installed = true;

	s_templateCommandMap.clear();
	
	ms_datatable = DataTableManager::getTable("datatables/timer/template_command_mapping.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(ms_datatable->getNumRows());
	int templateColumn = ms_datatable->findColumnNumber(ms_columnTemplate);
	int commandColumn = ms_datatable->findColumnNumber(ms_columnCommand);
	int fakeCommandGroupColumn = ms_datatable->findColumnNumber(ms_columnFakeCommandGroup);
	int executeFromToolbarColumn = ms_datatable->findColumnNumber(ms_columnExecuteFromToolbar);
	
	unsigned int i;

	for(i = 0; i < numRows; ++i)
	{
		std::string const & templateStr = ms_datatable->getStringValue(templateColumn, i);
		std::string const & command = ms_datatable->getStringValue(commandColumn, i);
		std::string const & fakeCommandGroup = ms_datatable->getStringValue(fakeCommandGroupColumn, i);
		bool executeFromToolbar = (ms_datatable->getIntValue(executeFromToolbarColumn, i) != 0);

		const uint32 templateHash = Crc::normalizeAndCalculate(templateStr.c_str());

		Record &rec = s_templateCommandMap[templateHash];
		rec.command = command;
		rec.commandHash = Crc::normalizeAndCalculate(command.c_str());
		rec.fakeCommandGroupHash = Crc::normalizeAndCalculate(fakeCommandGroup.c_str());
		rec.executeFromToolbar = executeFromToolbar;
	}
	ExitChain::add(remove, "TemplateCommandMappingManager::remove");
}

void TemplateCommandMappingManager::remove()
{
}

uint32 TemplateCommandMappingManager::getCommandCrcForTemplateCrc(uint32 templateCrc)
{
	TemplateCommandMap::iterator i = s_templateCommandMap.find(templateCrc);
	if(i == s_templateCommandMap.end())
		return 0;
	return i->second.commandHash;
}
	
const std::string& TemplateCommandMappingManager::getCommandForTemplateCrc(uint32 templateCrc)
{
	static std::string const s_commandNameDefault("");

	TemplateCommandMap::iterator i = s_templateCommandMap.find(templateCrc);
	if(i == s_templateCommandMap.end())
		return s_commandNameDefault;
	return i->second.command;
}

uint32 TemplateCommandMappingManager::getFakeCommandGroupForTemplateCrc(uint32 templateCrc)
{
	TemplateCommandMap::iterator i = s_templateCommandMap.find(templateCrc);
	if(i == s_templateCommandMap.end())
		return 0;
	return i->second.fakeCommandGroupHash;
}

bool TemplateCommandMappingManager::getExecuteFromToolbarForTemplateCrc(uint32 templateCrc)
{
	TemplateCommandMap::iterator i = s_templateCommandMap.find(templateCrc);
	if(i == s_templateCommandMap.end())
		return 0;
	return i->second.executeFromToolbar;
}
