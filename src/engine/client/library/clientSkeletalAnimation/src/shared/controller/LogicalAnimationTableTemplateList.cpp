// ======================================================================
//
// LogicalAnimationTableTemplateList.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"

#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedXml/XmlTreeDocument.h"
#include "sharedXml/XmlTreeDocumentList.h"

#include <map>
#include <string>

#ifdef _DEBUG
#include "sharedDebug/DataLint.h"
#endif

// ======================================================================

namespace LogicalAnimationTableTemplateListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdmap<const CrcString*, LogicalAnimationTableTemplate*, LessPointerComparator>::fwd  TemplateMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                           nameEndsInXml(char const *name);

	LogicalAnimationTableTemplate *fetchCachedTemplate(CrcString const &pathName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool         s_installed;
	TemplateMap *s_templates;
	bool         s_logLatFileActivity;
	bool         s_disableLatXmlSupport;
}

using namespace LogicalAnimationTableTemplateListNamespace;

// ======================================================================
// namespace LogicalAnimationTableTemplateListNamespace
// ======================================================================

// @todo: combine with ASH table's local version at some point.  This is duplicate code.

bool LogicalAnimationTableTemplateListNamespace::nameEndsInXml(char const *name)
{
	// Null strings don't end in XML.
	if (!name)
		return false;

	// Check last 4 characters for .xml
	size_t const pathNameLength = strlen(name);
	return
		(pathNameLength >= 4) && 
		(name[pathNameLength - 4] == '.') &&
		(name[pathNameLength - 3] == 'x') &&
		(name[pathNameLength - 2] == 'm') &&
		(name[pathNameLength - 1] == 'l');
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate *LogicalAnimationTableTemplateListNamespace::fetchCachedTemplate(CrcString const &pathName)
{
	const TemplateMap::iterator findIt = s_templates->find(&pathName);
	if (findIt == s_templates->end())
		return NULL;

	// Use the cached version, bump up reference count for caller.
	LogicalAnimationTableTemplate *const latTemplate = findIt->second;
	FATAL(!latTemplate, ("LAT List: pathName [%s] is cached but maps to a NULL LAT.", pathName.getString()));

	latTemplate->fetch();
	return latTemplate;
}

// ======================================================================

void LogicalAnimationTableTemplateList::install()
{
	DEBUG_FATAL(s_installed, ("LogicalAnimationTableTemplateList already installed"));

	//-- Create class resources.
	s_templates = new TemplateMap();
	
	//-- Register flags.
	DebugFlags::registerFlag(s_logLatFileActivity, "ClientSkeletalAnimation/Animation", "logLatFileActivity");
	DebugFlags::registerFlag(s_disableLatXmlSupport, "ClientSkeletalAnimation/Animation", "disableLatXmlSupport");

	s_installed = true;
	ExitChain::add(remove, "LogicalAnimationTableTemplateList");
}

// ----------------------------------------------------------------------

const LogicalAnimationTableTemplate *LogicalAnimationTableTemplateList::fetch(const CrcString &pathName)
{
	DEBUG_FATAL(!s_installed, ("LogicalAnimationTableTemplateList not installed"));

	LogicalAnimationTableTemplate *latTemplate = 0;

	//-- check if the specified template is cached
	latTemplate = fetchCachedTemplate(pathName);
	if (latTemplate)
		return latTemplate;

	//-- build an XML version of the filename.
	bool const requestedFileNameIsXml = nameEndsInXml(pathName.getString());
	std::string xmlFileName(pathName.getString());
	if (!requestedFileNameIsXml)
		IGNORE_RETURN(xmlFileName.append(".xml"));

	TemporaryCrcString const crcXmlFileName(xmlFileName.c_str(), false);

	//-- check if the .XML-named version of the template is cached
	if (!requestedFileNameIsXml)
	{
		latTemplate = fetchCachedTemplate(crcXmlFileName);
		if (latTemplate)
			return latTemplate;
	}
	else // only attempt to load the xml version if it is specifically requested
	{
		latTemplate = createTemplateFromXmlFileName(crcXmlFileName);
		FATAL(!latTemplate, ("LogicalAnimationTableTemplateList::fetch(): caller tried to open XML LAT file [%s] but we got back a NULL template, file probably doesn't exist.", xmlFileName.c_str()));
	}

	if (!latTemplate)
	{
		// The requested file name was not Xml.  
		latTemplate = createTemplateFromIffFileName(pathName);
	}

	//-- store template in the map
	if (latTemplate)
	{
		IGNORE_RETURN(s_templates->insert(TemplateMap::value_type(&latTemplate->getName(), latTemplate)));
		// bump up the reference count
		latTemplate->fetch();
	}

	return latTemplate;






	// REMOVE BELOW	

#if 0
	else
	{
		// need to load it

#ifdef _DEBUG
		DataLint::pushAsset(pathName.getString());
#endif // _DEBUG

		//-- load the LAT template from the IFF
		DEBUG_REPORT_LOG(s_logLatFileActivity, ("LATFILE: loading [%s] [%d total loaded].\n", pathName.getString(), static_cast<int>(s_templates->size()) + 1));

		Iff  iff;
		
		bool const exists = iff.open(pathName.getString(), true);
		if (!exists)
		{
			DEBUG_WARNING(true, ("LAT file [%s] does not exist or is not an IFF file.", pathName.getString()));
			return 0;
		}

		latTemplate = new LogicalAnimationTableTemplate(pathName, iff);

		//-- map it
		IGNORE_RETURN(s_templates->insert(lowerBoundResult, TemplateMap::value_type(&latTemplate->getName(), latTemplate)));

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG
	}

	//-- bump up the reference count
	NOT_NULL(latTemplate);
	latTemplate->fetch();

	return latTemplate;
#endif
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplateList::garbageCollect()
{
	//-- Ask each LogicalAnimationTableTemplate to garbage collect.  This will
	//   cause all .ANS references held by ProxySkeletalAnimations to be dropped,
	//   effectively throwing out all .ANS files except for those currently playing
	//   and those on the PreloadedAssetManager list.

#if PRODUCTION == 0
	// Collect start time/memory info.
	if (!s_templates)
		return;

	unsigned long const startAllocatedBytes = MemoryManager::getCurrentNumberOfBytesAllocated();
	unsigned long const startTimeMs = Clock::timeMs();
#endif

	TemplateMap::iterator const endIt = s_templates->end();
	for (TemplateMap::iterator it = s_templates->begin(); it != endIt; ++it)
	{
		if (it->second)
			it->second->garbageCollect();
	}

#if PRODUCTION == 0
	unsigned long const stopTimeMs = Clock::timeMs();
	unsigned long const stopAllocatedBytes = MemoryManager::getCurrentNumberOfBytesAllocated();
	float const collectTime = static_cast<float>(stopTimeMs - startTimeMs) / 1000.0f;
	float const memoryPercentage = static_cast<float>(startAllocatedBytes - stopAllocatedBytes) / (1024.0f * 1024.0f);

	REPORT_LOG(memoryPercentage > 0.0f || collectTime > 0.0f, ("LogicalAnimationTableTemplateList::garbageCollect() took [%.2f] seconds and decreased memory usage by [%.2f] MB.\n",
		collectTime,
		memoryPercentage));

	UNREF(collectTime);
	UNREF(memoryPercentage);
#endif
}

// ======================================================================

void LogicalAnimationTableTemplateList::remove()
{
	DEBUG_FATAL(!s_installed, ("LogicalAnimationTableTemplateList not installed"));
	s_installed = false;

	delete s_templates;
	s_templates = 0;
}

// ----------------------------------------------------------------------

void LogicalAnimationTableTemplateList::stopTracking(const LogicalAnimationTableTemplate &latTemplate)
{
	//-- check if this template has a name.
	const CrcString &templateName = latTemplate.getName();
	if (!*templateName.getString())
	{
		// no name, so there's no chance we're keeping a named copy.
		return;
	}

	//-- check if it's one of our named templates
	TemplateMap::iterator findIt = s_templates->find(&templateName);
	if (findIt == s_templates->end())
	{
		DEBUG_WARNING(true, ("LogicalAnimationTableTemplateList [%s] not managed by list\n", templateName.getString()));
		return;
	}

	//-- remove from list of tracked templates
	s_templates->erase(findIt);
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate *LogicalAnimationTableTemplateList::createTemplateFromIffFileName(CrcString const &pathName)
{
	if (!pathName.getString())
		return NULL;

#ifdef _DEBUG
	DataLint::pushAsset(pathName.getString());
#endif // _DEBUG

	//-- load the LAT template from the IFF
	Iff  iff(pathName.getString());
		
	LogicalAnimationTableTemplate *const latTemplate = new LogicalAnimationTableTemplate(pathName, iff);
	WARNING_STRICT_FATAL(!latTemplate, ("LogicalAnimationTableTemplate create function for [%s] returned null.", pathName.getString()));

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	return latTemplate;
}

// ----------------------------------------------------------------------

LogicalAnimationTableTemplate *LogicalAnimationTableTemplateList::createTemplateFromXmlFileName(CrcString const &pathName)
{
	if (s_disableLatXmlSupport)
		return NULL;

	char const *const cPathName = pathName.getString();
	if (!cPathName)
		return NULL;

	if (!TreeFile::exists(cPathName))
		return NULL;

#ifdef _DEBUG
	DataLint::pushAsset(cPathName);
#endif // _DEBUG

	XmlTreeDocument const *const xmlTreeDocument = XmlTreeDocumentList::fetch(pathName);
	FATAL(!xmlTreeDocument, ("XmlTreeDocumentList::fetch(): return NULL for filename [%s].", pathName.getString()));

	LogicalAnimationTableTemplate *const latTemplate = new LogicalAnimationTableTemplate(pathName, *xmlTreeDocument);

	//-- Release reference.
	xmlTreeDocument->release();

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	return latTemplate;
}

// ======================================================================
