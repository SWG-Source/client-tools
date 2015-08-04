//======================================================================
//
// ShipComponentEditorServerTemplate.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ShipComponentEditorServerTemplate.h"

#include "ConfigShipComponentEditor.h"
#include "ShipComponentEditorServerTemplateManager.h"
#include "UnicodeUtils.h"
#include "fileinterface/StdioFile.h"
#include <string>

//----------------------------------------------------------------------

namespace ShipComponentEditorServerTemplateNamespace
{

}

using namespace ShipComponentEditorServerTemplateNamespace;

//======================================================================

void ShipComponentEditorServerTemplate::loadFromString(std::string const & bufferStr, bool isShip)
{
	std::string const keyBase = "@base ";

	std::string const keySharedTemplate = "\nsharedTemplate";
	
	std::string const keyShipType = "\nshipType";
	
	std::string const whitespace = " \t\n";
	
	//-- find the @base tag
	{
		size_t const basePosKey = bufferStr.find(keyBase);
		if (basePosKey != std::string::npos)
		{
			size_t const baseValueStart = bufferStr.find_first_not_of(whitespace, basePosKey + keyBase.size());
			if (baseValueStart != std::string::npos)
			{
				size_t const baseValueEnd = bufferStr.find_first_of(whitespace, baseValueStart);
				if (baseValueEnd != std::string::npos)
				{
					this->baseServerTemplate = Unicode::getTrim(bufferStr.substr(baseValueStart, (baseValueEnd - baseValueStart)));
				}
				else
					WARNING(true, ("ShipComponentEditorServerTemplate could not find VALUE END for [%s] tag in [%s]", keyBase.c_str(), name.c_str()));
			}
			else
				WARNING(true, ("ShipComponentEditorServerTemplate could not find VALUE START for [%s] tag in [%s]", keyBase.c_str(), name.c_str()));
		}
		else
			WARNING(true, ("ShipComponentEditorServerTemplate could not find [%s] tag in [%s]", keyBase.c_str(), name.c_str()));
	}
	
	findValueForKey(bufferStr, keySharedTemplate, this->sharedTemplateName);

	if (isShip)
		findValueForKey(bufferStr, keyShipType, this->chassisType);
}

//----------------------------------------------------------------------

bool ShipComponentEditorServerTemplate::findValueForKey(std::string const & bufferStr, std::string const & key, std::string & result)
{
	size_t const sharedTemplatePosKey = bufferStr.find(key);
	if (sharedTemplatePosKey != std::string::npos)
	{
		size_t const quoteposFirst = bufferStr.find('\"', sharedTemplatePosKey);
		
		if (quoteposFirst != std::string::npos)
		{
			size_t const quoteposSecond = bufferStr.find('\"', quoteposFirst + 1);
			
			if (quoteposSecond != std::string::npos)
			{
				result = Unicode::getTrim(bufferStr.substr(quoteposFirst + 1, quoteposSecond - (quoteposFirst + 1)));
				return true;
			}
			else
				WARNING(true, ("ShipComponentEditorServerTemplate could not find VALUE END for [%s] tag in [%s]", key.c_str(), name.c_str()));
		}
		else
			WARNING(true, ("ShipComponentEditorServerTemplate could not find VALUE START for [%s] tag in [%s]", key.c_str(), name.c_str()));
	}

	return false;
}

//----------------------------------------------------------------------

bool ShipComponentEditorServerTemplate::load(std::string const & otName, bool const isShip)
{
	name = otName;

	StdioFileFactory sff;
	
	std::string otPath = ConfigShipComponentEditor::getServerPathDsrc() + "/" + std::string(otName);
	{
		size_t lastDot = otPath.rfind('.');
		if (lastDot != std::string::npos)
		{
			otPath.erase(lastDot, otPath.size() - lastDot);
			otPath.append(".tpf");
		}
	}

	fullPath = otPath;
	
	AbstractFile * const af = sff.createFile(otPath.c_str(), "rb");
	
	if (NULL == af || !af->isOpen())
	{
		WARNING(true, ("ShipComponentEditorServerTemplateManager unable to open server template [%s]", otPath.c_str()));
		delete af;
	}
	else
	{
		byte * const buffer = af->readEntireFileAndClose();
		delete af;

		NOT_NULL(buffer);
		
		std::string bufferStr(reinterpret_cast<char const *>(buffer));
		loadFromString(bufferStr, isShip);
		delete[] buffer;
		
		fullPathSharedTemplate = ConfigShipComponentEditor::getSharedPathDsrc() + "/" + sharedTemplateName;
		{
			size_t lastDot = fullPathSharedTemplate.rfind('.');
			if (lastDot != std::string::npos)
			{
				fullPathSharedTemplate.erase(lastDot, fullPathSharedTemplate.size() - lastDot);
				fullPathSharedTemplate.append(".tpf");
			}
		}	

		return true;
	}
	
	return false;
}
//======================================================================
