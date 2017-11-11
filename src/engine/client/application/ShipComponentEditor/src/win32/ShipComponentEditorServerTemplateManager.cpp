//======================================================================
//
// ShipComponentEditorServerTemplateManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ShipComponentEditorServerTemplateManager.h"

#include "ShipComponentEditorServerTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedDebug/DebugFlags.h"
#include "fileinterface/StdioFile.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedObject/Object.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/Crc.h"
#include "ShipComponentEditor.h"
#include "ConfigShipComponentEditor.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <map>

#if WIN32
#include <direct.h>
#endif

//======================================================================

namespace ShipComponentEditorServerTemplateManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<bool const &, ShipComponentEditorServerTemplateManager::Messages::TemplateListChanged> templateListChanged;
	}

	CrcStringTable ms_crcStringTable;

	typedef stdmap<std::string, ShipComponentEditorServerTemplate>::fwd ServerTemplateMap;
	ServerTemplateMap s_serverTemplateMap;

	typedef stdmap<uint32, std::string>::fwd CrcNameMap;
	CrcNameMap s_crcNameMap;
}

using namespace ShipComponentEditorServerTemplateManagerNamespace;

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::install()
{

	load();
}

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::remove()
{
	clear();
}

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::clear()
{
	s_serverTemplateMap.clear();
	s_crcNameMap.clear();
}

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::load()
{
	clear();

	typedef stdvector<const char *>::fwd ConstCharVector;
	
	StdioFileFactory sff;
	{
		std::string const & stringTablePath = ConfigShipComponentEditor::getServerTemplateCrcStringTable();
		AbstractFile * const af = sff.createFile(stringTablePath.c_str(), "rb");
		if (NULL != af)
		{
			if (!af->isOpen())
			{
				WARNING(true, ("ShipComponentEditorServerTemplateManager unable to open [%s]", stringTablePath.c_str()));
			}
			else
			{
				Iff iff;
				iff.open(*af);
				ms_crcStringTable.load(iff);
			}
			delete af;
			
		}
	}
	ConstCharVector ccv;
	
	ms_crcStringTable.getAllStrings(ccv);
	
	char const * const shipPrefix = "object/ship/";
	size_t const shipPrefixLen = strlen(shipPrefix);

	char const * const shipComponentPrefix = "object/tangible/ship/";
	size_t const shipComponentPrefixLen = strlen(shipComponentPrefix);

	char const * const sharedExclusionString = "/shared_";

	for (ConstCharVector::const_iterator it = ccv.begin(); it != ccv.end(); ++it)
	{
		char const * const otName = *it;
		bool const isShip = !strncmp(otName, shipPrefix, shipPrefixLen);
		bool const isShipComponent = (!isShip) && !strncmp(otName, shipComponentPrefix, shipComponentPrefixLen);

		if (isShip || isShipComponent)
		{
			if (NULL != strstr(otName, sharedExclusionString))
				continue;

			ShipComponentEditorServerTemplate st;

			if (st.load(std::string(otName), isShip))
			{			
				std::string const otNameStr(otName);

				s_serverTemplateMap.insert(std::make_pair(otNameStr, st));
				s_crcNameMap.insert(std::make_pair(Crc::normalizeAndCalculate(otName), otNameStr));
			}			
		}

//		REPORT_LOG_PRINT(true, (":   [%s]\n", *it));
	}

	Transceivers::templateListChanged.emitMessage(true);
}

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::findObjectTemplatesForChassisType(std::string const & chassisType, ServerTemplateVector & result)
{
	for (ServerTemplateMap::const_iterator it = s_serverTemplateMap.begin(); it != s_serverTemplateMap.end(); ++it)
	{
		ShipComponentEditorServerTemplate const & st = (*it).second;

		if (chassisType.empty() || (st.chassisType == chassisType))
			result.push_back(st);
	}
}

//----------------------------------------------------------------------

ShipComponentEditorServerTemplate const * ShipComponentEditorServerTemplateManager::findTemplateByName(std::string const & name)
{
	ServerTemplateMap::const_iterator const it = s_serverTemplateMap.find(name);
	if (it != s_serverTemplateMap.end())
		return &(*it).second;
	return NULL;
}

//----------------------------------------------------------------------

ShipComponentEditorServerTemplate const * ShipComponentEditorServerTemplateManager::findTemplateByCrc(uint32 crc)
{
	CrcNameMap::const_iterator it = s_crcNameMap.find(crc);

	if (it != s_crcNameMap.end())
	{
		return ShipComponentEditorServerTemplateManager::findTemplateByName((*it).second);
	}
	return NULL;
}

//----------------------------------------------------------------------

void ShipComponentEditorServerTemplateManager::regenerateTemplateDb()
{
	char buf[1024];
	size_t const buf_size = sizeof(buf);

	std::string const cwd (_getcwd(buf, buf_size));
	_chdir(ConfigShipComponentEditor::getToolPath().c_str());
	snprintf(buf, buf_size, "perl buildObjectTemplateCrcStringTables.pl %s", ConfigShipComponentEditor::getBranchName().c_str());
	system(buf);
	_chdir(cwd.c_str());

	ShipComponentEditorServerTemplateManager::load();
}

//======================================================================

