// ======================================================================
//
// SkeletalAnimationTemplateList.cpp
// Copyright 2001-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"

#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Tag.h"
#include "sharedXml/XmlTreeNode.h"

#include <map>
#include <string>

// ======================================================================

namespace SkeletalAnimationTemplateListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdmap<std::string, SkeletalAnimationTemplateList::XmlCreateFunction>::fwd  XmlCreationFunctionMap;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	XmlCreationFunctionMap  s_xmlCreationFunctions;
}

using namespace SkeletalAnimationTemplateListNamespace;

// ======================================================================
// static variable storage
// ======================================================================

bool  SkeletalAnimationTemplateList::ms_installed;

SkeletalAnimationTemplateList::CreationFunctionMap *SkeletalAnimationTemplateList::ms_creationFunctions;
SkeletalAnimationTemplateList::TemplateMap         *SkeletalAnimationTemplateList::ms_namedTemplates;

// ======================================================================

void SkeletalAnimationTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("SkeletalAnimationTemplateList already installed"));
	DEBUG_FATAL(ms_namedTemplates, ("invalid startup state"));
	DEBUG_FATAL(ms_creationFunctions, ("invalid startup state"));

	ms_namedTemplates    = new TemplateMap();
	ms_creationFunctions = new CreationFunctionMap();

	ms_installed = true;

	ExitChain::add(SkeletalAnimationTemplateList::remove, "SkeletalAnimationTemplateList");
}

// ----------------------------------------------------------------------

bool SkeletalAnimationTemplateList::registerCreateFunction(Tag tag, CreateFunction createFunction)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));
	NOT_NULL(createFunction);

	//-- try to insert it
	std::pair<CreationFunctionMap::iterator, bool> insertReturn = ms_creationFunctions->insert(CreationFunctionMap::value_type(tag, createFunction));
	return insertReturn.second;
}

// ----------------------------------------------------------------------

bool SkeletalAnimationTemplateList::deregisterCreateFunction(Tag tag)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));

	//-- look for it
	CreationFunctionMap::iterator it = ms_creationFunctions->find(tag);
	if (it == ms_creationFunctions->end())
	{
		// didn't find it
		return false;
	}
	else
	{
		// found it.  erase it.
		ms_creationFunctions->erase(it);
		return true;
	}
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplateList::registerXmlCreateFunction(char const *elementName, XmlCreateFunction createFunction)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));
	NOT_NULL(createFunction);
	NOT_NULL(elementName);

	//-- try to insert it
	std::pair<XmlCreationFunctionMap::iterator, bool> insertReturn = s_xmlCreationFunctions.insert(XmlCreationFunctionMap::value_type(elementName, createFunction));
	FATAL(!insertReturn.second, ("Failed to insert animation's xml create function for element [%s], probably trying to register it more than once.", elementName));
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplateList::deregisterXmlCreateFunction(char const *elementName)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));
	NOT_NULL(elementName);

	//-- look for it
	XmlCreationFunctionMap::iterator it = s_xmlCreationFunctions.find(elementName);
	FATAL(it == ms_creationFunctions->end(), ("Failed to deregister animation's xml create function for element [%s] --- element not registered.", elementName));

	s_xmlCreationFunctions.erase(it);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *SkeletalAnimationTemplateList::fetch(const CrcString &name)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));

	//-- check if we already have this one loaded
	TemplateMap::const_iterator it = ms_namedTemplates->find(&name);
	if (it != ms_namedTemplates->end())
	{
		// found it.  bump up ref count
		const SkeletalAnimationTemplate *const skeletalAnimationTemplate = it->second;
		skeletalAnimationTemplate->fetch();

		return skeletalAnimationTemplate;
	}

	//-- load up skeletal animation template

#ifdef _DEBUG
	DataLint::pushAsset(name.getString());
#endif // _DEBUG

	// open the iff file
	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogAnimationLoading(), ("SATL: loading animation [%s].\n", name.getString()));	
	Iff iff(name.getString());

	// find create function from type tag
	const Tag typeTag = iff.getCurrentName();
	CreationFunctionMap::const_iterator itCreator = ms_creationFunctions->find(typeTag);

#ifdef _DEBUG
	if (itCreator == ms_creationFunctions->end())
	{
		char badTagName[5];
		ConvertTagToString(typeTag, badTagName);
		DEBUG_FATAL(true, ("SkeletalAnimationTemplate type [tag=%s] unknown", badTagName));
	}
#endif

	CreateFunction createFunction = itCreator->second;

	// create skeleton template
	// @todo fix the CrcLowerString when we get a chance --- change signature and all IFF animation load code.
	SkeletalAnimationTemplate *newTemplate = NON_NULL((*createFunction)(CrcLowerString(name.getString()), iff));
	newTemplate->fetch();

	//-- add to list of known templates
	IGNORE_RETURN(ms_namedTemplates->insert(TemplateMap::value_type(&newTemplate->getName(), newTemplate)));

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	//-- success
	return newTemplate;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *SkeletalAnimationTemplateList::fetch(Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));

	// find create function from type tag
	const Tag typeTag = iff.getCurrentName();

	if (typeTag == TAG_NAME)
	{
		// create named template from specified file
		char filename[2048];

		iff.enterChunk(TAG_NAME);
			iff.read_string(filename, sizeof(filename) - 1);
		iff.exitChunk(TAG_NAME);

		return fetch(CrcLowerString(filename));
	}

	// create anonymous template
	CreationFunctionMap::const_iterator itCreator = ms_creationFunctions->find(typeTag);

#ifdef _DEBUG
	if (itCreator == ms_creationFunctions->end())
	{
		char badTagName[5];
		ConvertTagToString(typeTag, badTagName);
		DEBUG_FATAL(true, ("SkeletalAnimationTemplate type [tag=%s] unknown", badTagName));
	}
#endif

	CreateFunction createFunction = itCreator->second;

	// create skeleton template
	SkeletalAnimationTemplate *const newTemplate = NON_NULL((*createFunction)(CrcLowerString::empty, iff));
	newTemplate->fetch();

	//-- success
	return newTemplate;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *SkeletalAnimationTemplateList::fetch(XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));

	// find create function from element name.
	char const *const elementName = treeNode.getName();

	// create anonymous template
	XmlCreationFunctionMap::iterator const itCreator = s_xmlCreationFunctions.find(elementName);
	FATAL(itCreator == s_xmlCreationFunctions.end(), ("unsupported animation type with XML entity name [%s].", elementName));

	XmlCreateFunction createFunction = itCreator->second;

	// create skeleton template
	SkeletalAnimationTemplate *const newTemplate = NON_NULL((*createFunction)(CrcLowerString::empty, treeNode, versionNumber));
	newTemplate->fetch();

	//-- success
	return newTemplate;
}

// ----------------------------------------------------------------------

void  SkeletalAnimationTemplateList::stopTracking(const SkeletalAnimationTemplate *skeletalAnimationTemplate)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));
	NOT_NULL(skeletalAnimationTemplate);

	const CrcString &name = skeletalAnimationTemplate->getName();
	if (!name.getString() || !*name.getString())
	{
		// we don't have anything to stop tracking if the template is unnamed
		return;
	}

	//-- search for it in named container
	TemplateMap::iterator findIt = ms_namedTemplates->find(&name);

	if (findIt == ms_namedTemplates->end())
	{
		DEBUG_WARNING(true, ("named template [%s] does not appear to be tracked", name.getString()));
		return;
	}

	//-- sanity check
	DEBUG_FATAL(skeletalAnimationTemplate != findIt->second, ("found named SkeletalAnimationTemplate, but different object is there"));

	//-- erase it
	ms_namedTemplates->erase(findIt);
}

// ======================================================================

void SkeletalAnimationTemplateList::remove()
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateList not installed"));

	//-- cleanup named templates
	if (ms_namedTemplates->size())
	{
		DEBUG_WARNING(true, ("Named SkeletalAnimationTemplate leaks [%u objects]:", ms_namedTemplates->size()));

		const TemplateMap::iterator itEnd = ms_namedTemplates->end();
		for (TemplateMap::iterator it = ms_namedTemplates->begin(); it != itEnd; ++it)
		{
			const CrcString &crcName = *NON_NULL(it->first);
			DEBUG_REPORT_LOG(true, ("-- [%s] leaked\n", crcName.getString()));

			UNREF(crcName);
		}
	}

	delete ms_namedTemplates;
	ms_namedTemplates = 0;

	//-- cleanup creation function container
	delete ms_creationFunctions;
	ms_creationFunctions = 0;

	s_xmlCreationFunctions.clear();

	ms_installed = false;
}

// ======================================================================
