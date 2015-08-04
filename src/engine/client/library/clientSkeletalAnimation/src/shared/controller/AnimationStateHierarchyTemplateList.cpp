// ======================================================================
//
// AnimationStateHierarchyTemplateList.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"

#include "clientSkeletalAnimation/AnimationStateHierarchyTemplate.h"
#include "fileInterface/AbstractFile.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedXml/XmlTreeDocument.h"
#include "sharedXml/XmlTreeDocumentList.h"

#include <map>
#include <string>

#ifdef _DEBUG
#include "sharedDebug/DataLint.h"
#endif

// ======================================================================
// lint supression

//lint -e1702 // operator* is both ordinary and member // started getting this a lot lately.

// ======================================================================

bool                                                 AnimationStateHierarchyTemplateList::ms_installed;

// ======================================================================

namespace AnimationStateHierarchyTemplateListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdmap<const CrcString*, AnimationStateHierarchyTemplate*, LessPointerComparator>::fwd  TemplateMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  nameEndsInXml(char const *name);

	AnimationStateHierarchyTemplate *createTemplateFromXmlFileName(CrcString const &pathName);
	AnimationStateHierarchyTemplate *createTemplateFromIffFileName(CrcString const &pathName);
	AnimationStateHierarchyTemplate *fetchCachedTemplate(CrcString const &pathName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	AnimationStateHierarchyTemplateList::CreateFunction     s_createFunction;
	AnimationStateHierarchyTemplateList::XmlCreateFunction  s_xmlCreateFunction;
	TemplateMap                                            *s_templates;

	bool                                                    s_disableAshXmlSupport;
}

using namespace AnimationStateHierarchyTemplateListNamespace;

// ======================================================================
// namespace AnimationStateHierarchyTemplateListNamespace
// ======================================================================

bool AnimationStateHierarchyTemplateListNamespace::nameEndsInXml(char const *name)
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

AnimationStateHierarchyTemplate *AnimationStateHierarchyTemplateListNamespace::createTemplateFromXmlFileName(CrcString const &pathName)
{
	FATAL(!s_xmlCreateFunction, ("AnimationStateHierarchyTemplateList requires an XML create function but none was provided."));

	if (s_disableAshXmlSupport)
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

	AnimationStateHierarchyTemplate *const ashTemplate = (*s_xmlCreateFunction)(pathName, *xmlTreeDocument);

	//-- Release reference.
	xmlTreeDocument->release();

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	return ashTemplate;
}
// ----------------------------------------------------------------------

AnimationStateHierarchyTemplate *AnimationStateHierarchyTemplateListNamespace::createTemplateFromIffFileName(CrcString const &pathName)
{
	if (!pathName.getString())
		return NULL;

#ifdef _DEBUG
	DataLint::pushAsset(pathName.getString());
#endif // _DEBUG

	//-- load the LAT template from the IFF
	Iff  iff(pathName.getString());
		
	AnimationStateHierarchyTemplate *const ashTemplate = (*s_createFunction)(pathName, iff);
	WARNING_STRICT_FATAL(!ashTemplate, ("AnimationStateHierarchyTemplate create function for [%s] returned null.", pathName.getString()));

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	return ashTemplate;
}

// ----------------------------------------------------------------------

AnimationStateHierarchyTemplate *AnimationStateHierarchyTemplateListNamespace::fetchCachedTemplate(CrcString const &pathName)
{
	const TemplateMap::iterator findIt = s_templates->find(&pathName);
	if (findIt == s_templates->end())
		return NULL;

	// Use the cached version, bump up reference count for caller.
	AnimationStateHierarchyTemplate *const ashTemplate = findIt->second;
	FATAL(!ashTemplate, ("ASH List: pathName [%s] is cached but maps to a NULL ASH.", pathName.getString()));

	ashTemplate->fetch();
	return ashTemplate;
}

// ======================================================================
// class AnimationStateHierarchyTemplateList: PUBLIC STATIC
// ======================================================================

void AnimationStateHierarchyTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("AnimationStateHierarchyTemplateList already installed"));

	s_templates = new TemplateMap();

	DebugFlags::registerFlag(s_disableAshXmlSupport, "ClientSkeletalAnimation/Animation", "disableAshXmlSupport");

	ms_installed = true;
	ExitChain::add(remove, "AnimationStateHierarchyTemplateList");
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyTemplateList::setCreateFunction(CreateFunction createFunction)
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateHierarchyTemplateList not installed"));
	s_createFunction = createFunction;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyTemplateList::setXmlCreateFunction(XmlCreateFunction xmlCreateFunction)
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateHierarchyTemplateList not installed"));
	s_xmlCreateFunction = xmlCreateFunction;
}

// ----------------------------------------------------------------------

const AnimationStateHierarchyTemplate *AnimationStateHierarchyTemplateList::fetch(const CrcString &pathName)
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateHierarchyTemplateList not installed"));
	DEBUG_FATAL(!s_createFunction, ("AnimationStateHierarchyTemplateList create function not installed"));

	AnimationStateHierarchyTemplate *ashTemplate = 0;

	//-- check if the specified template is cached
	ashTemplate = fetchCachedTemplate(pathName);
	if (ashTemplate)
		return ashTemplate;

	//-- build an XML version of the filename.
	bool const requestedFileNameIsXml = nameEndsInXml(pathName.getString());
	std::string xmlFileName(pathName.getString());
	if (!requestedFileNameIsXml)
		IGNORE_RETURN(xmlFileName.append(".xml"));

	TemporaryCrcString const crcXmlFileName(xmlFileName.c_str(), false);

	//-- check if the .XML-named version of the template is cached
	if (!requestedFileNameIsXml)
	{
		ashTemplate = fetchCachedTemplate(crcXmlFileName);
		if (ashTemplate)
			return ashTemplate;
	}
	else // only attempt to load the xml version if it is specifically requested
	{
		ashTemplate = createTemplateFromXmlFileName(crcXmlFileName);
		FATAL(!ashTemplate, ("AnimationStateHierarchyTemplateList::fetch(): caller tried to open XML ASH file [%s] but we got back a NULL template, file probably doesn't exist.", xmlFileName.c_str()));
	}

	if (!ashTemplate)
	{
		// The requested file name was not Xml.  
		ashTemplate = createTemplateFromIffFileName(pathName);
	}

	//-- store template in the map
	if (ashTemplate)
	{
		IGNORE_RETURN(s_templates->insert(TemplateMap::value_type(&ashTemplate->getName(), ashTemplate)));
		// bump up the reference count
		ashTemplate->fetch();
	}

	return ashTemplate;
}

// ======================================================================

void AnimationStateHierarchyTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateHierarchyTemplateList not installed"));
	ms_installed = false;

	delete s_templates;
	s_templates = 0;

	s_createFunction = 0;
	s_xmlCreateFunction = 0;
}

// ----------------------------------------------------------------------

void AnimationStateHierarchyTemplateList::stopTracking(const AnimationStateHierarchyTemplate &ashTemplate)
{
	//-- check if this template has a name.
	CrcString const &templateName = ashTemplate.getName();
	if (!*templateName.getString())
	{
		// no name, so there's no chance we're keeping a named copy.
		return;
	}

	//-- check if it's one of our named templates
	TemplateMap::iterator findIt = s_templates->find(&templateName);
	if (findIt == s_templates->end())
	{
		DEBUG_WARNING(true, ("AnimationStateHierarchyTemplateList [%s] not managed by list\n", templateName.getString()));
		return;
	}

	//-- remove from list of tracked templates
	s_templates->erase(findIt);
}

// ======================================================================
