// ======================================================================
//
// StringSelectorSkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/StringSelectorSkeletalAnimation.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

namespace StringSelectorSkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ANMS = TAG(A,N,M,S);
	Tag const TAG_DFLT = TAG(D,F,L,T);
	Tag const TAG_SSAT = TAG(S,S,A,T);
	Tag const TAG_VALS = TAG3(V,A,L);

	char const *const cs_stringSelectorAnimationElementName = "stringSelectorAnimation";
	char const *const cs_variableNameAttributeName          = "variableName";

	char const *const cs_choiceElementName                  = "choice";
	char const *const cs_selectValueElementName             = "selectValue";
	char const *const cs_valueAttributeName                 = "value";

	char const *const cs_selectAnimationElementName         = "selectAnimation";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CrcLowerString *s_defaultVariableValue;
	std::string     s_temporaryString;
}

using namespace StringSelectorSkeletalAnimationTemplateNamespace;

// ======================================================================

bool                                StringSelectorSkeletalAnimationTemplate::ms_installed;
SkeletalAnimationTemplateUiFactory *StringSelectorSkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================
// class StringSelectorSkeletalAnimationTemplate: public member functions
// ======================================================================

void StringSelectorSkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("StringSelectorSkeletalAnimationTemplate already installed"));

	//-- Register IFF reader.
	const bool result = SkeletalAnimationTemplateList::registerCreateFunction(TAG_SSAT, create);
	DEBUG_FATAL(!result, ("failed to register create function."));
	UNREF(result);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_stringSelectorAnimationElementName, xmlCreate);

	s_defaultVariableValue = new CrcLowerString("default");

	ms_installed = true;
	ExitChain::add(remove, "StringSelectorSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	DEBUG_FATAL(!ms_installed, ("DirectionSkeletalAnimationTemplate not installed."));

	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete existing uiFactory.
	delete ms_uiFactory;

	//-- Assign new uiFactory.
	ms_uiFactory = uiFactory;
}

// ======================================================================
// class StringSelectorSkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *StringSelectorSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const animation = new StringSelectorSkeletalAnimation(*this, animationEnvironment, transformNameMap);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::garbageCollect() const
{
	NOT_NULL(m_templates);

	//-- Forward on garbageCollect() call to all subordinate templates.
	TemplateVector::iterator const endIt = m_templates->end();
	for (TemplateVector::iterator it = m_templates->begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->garbageCollect();
	}
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *StringSelectorSkeletalAnimationTemplate::fetchConstAnimationTemplateForValue(const CrcLowerString &value) const
{
	const SkeletalAnimationTemplate *saTemplate = 0;

	const ValueMap::const_iterator findIt = m_valueMap->find(value);
	if (findIt != m_valueMap->end())
	{
		//-- Entry found, use it.  If null, I'll try to use the default.
		saTemplate = findIt->second;
	}

	//-- If template is null, try default.
	if (!saTemplate)
	{
		saTemplate = m_defaultTemplate;
	}

	//-- Fetch reference for caller.
	if (saTemplate)
		saTemplate->fetch();

	return saTemplate;
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimationTemplate::getSelectionAnimationTemplateCount() const
{
	return static_cast<int>(m_templates->size());
}

// ----------------------------------------------------------------------

int StringSelectorSkeletalAnimationTemplate::getSelectionIndexForValue(const CrcLowerString &value) const
{
	// @todo need to associated _index_ of selection animation template with value.  It is no longer
	//       critical to keep the animation template pointer directly associated with the value.
	const SkeletalAnimationTemplate *saTemplate = 0;

	const ValueMap::iterator findIt = m_valueMap->find(value);
	if (findIt != m_valueMap->end())
	{
		//-- Entry found, use it.  If null, I'll try to use the default.
		saTemplate = findIt->second;
	}

	if (!saTemplate)
		saTemplate = m_defaultTemplate;

	// This will continue to be more expensive than necessary until I associate index with value explicitly.
	const TemplateVector::iterator templateFindIt = std::find(m_templates->begin(), m_templates->end(), saTemplate);
	if (templateFindIt != m_templates->end())
	{
		// Return index.
		return std::distance(m_templates->begin(), templateFindIt);
	}
	else
	{
		// One of these happened: the value mapped to a bad template, the default template wasn't set, or the default mapped to a bad template.
		return 0;
	}
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *StringSelectorSkeletalAnimationTemplate::fetchSelectionAnimationTemplate(int index) const
{
	//-- Get the template.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getSelectionAnimationTemplateCount());
	const SkeletalAnimationTemplate *const saTemplate = (*m_templates)[static_cast<TemplateVector::size_type>(index)];

	//-- Fetch reference to it for caller.
	if (saTemplate)
		saTemplate->fetch();

	return saTemplate;
}

// ----------------------------------------------------------------------

StringSelectorSkeletalAnimationTemplate::StringSelectorSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_environmentVariableName("<unspecified>"),
	m_templates(new TemplateVector()),
	m_valueMap(new ValueMap()),
	m_defaultTemplate(0)
{
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::setEnvironmentVariableName(const CrcLowerString &variableName)
{
	m_environmentVariableName = variableName;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the count of values that map to the specified template.
 *
 * The returned value does not include the default value if it happens
 * to point to the given selection template.
 *
 * @return  the count of values that map to the specified template.
 */

int StringSelectorSkeletalAnimationTemplate::getSelectionValueCount(const SkeletalAnimationTemplate *selectionTemplate) const
{
	//-- Handle null template case.
	if (!selectionTemplate)
		return 0;

	//-- Count the number of values that map to the specified selection template.
	int count = 0;

	const ValueMap::const_iterator endIt = m_valueMap->end();
	for (ValueMap::const_iterator it = m_valueMap->begin(); it != endIt; ++it)
	{
		if (it->second == selectionTemplate)
			++count;
	}

	//-- Return count to caller.
	return count;
}

// ----------------------------------------------------------------------

const CrcLowerString &StringSelectorSkeletalAnimationTemplate::getSelectionValue(const SkeletalAnimationTemplate *selectionTemplate, int index) const
{
	//-- Handle null template case.
	if (!selectionTemplate)
		return CrcLowerString::empty;

	//-- Count the number of values that map to the specified selection template.
	int count = 0;

	const ValueMap::const_iterator endIt = m_valueMap->end();
	for (ValueMap::const_iterator it = m_valueMap->begin(); it != endIt; ++it)
	{
		if (it->second == selectionTemplate)
		{
			if (count == index)
			{
				// This is the matching entry with the given index.  Return the value that maps to it.
				return it->first;
			}

			++count;
		}
	}

	//-- No match was found.
	return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

bool StringSelectorSkeletalAnimationTemplate::hasValue(const CrcLowerString &value) const
{
	const ValueMap::iterator findIt = m_valueMap->find(value);
	return (findIt != m_valueMap->end());
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::addSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &value)
{
	//-- Ignore null selectionTemplate entries.
	if (!selectionTemplate)
		return;

	//-- If value is mapped to a different selection template, replace the mapping.
	const ValueMap::iterator lowerBoundResult = m_valueMap->lower_bound(value);
	if ((lowerBoundResult == m_valueMap->end()) || m_valueMap->key_comp()(value, lowerBoundResult->first))
	{
		// Entry doesn't exist, create it.
		IGNORE_RETURN(m_valueMap->insert(lowerBoundResult, ValueMap::value_type(value, selectionTemplate)));
	}
	else
	{
		// Entry already exists.
		if (lowerBoundResult->second == selectionTemplate)
		{
			// Value already set to specified entry.
			DEBUG_WARNING(true, ("duplicate selection template value mapping attempted [%s = 0x%08x].", value.getString(), selectionTemplate));
			return;
		}
		else
		{
			// Value mapped to something else.
			const SkeletalAnimationTemplate *oldTemplate = lowerBoundResult->second;
			lowerBoundResult->second = selectionTemplate;

			removeSelectionTemplateIfUnreferenced(oldTemplate);
		}
	}

	addSelectionTemplateIfNotPresent(selectionTemplate);
}

// ----------------------------------------------------------------------
/**
 * blah.
 *
 * The purpose for this function is to allow deletion of a mapping entry
 * without allowing the underlying selection template to get deleted.
 * If this function was not provided, then the deleteMapping function
 * would delete the animation if no other value referenced it.
 */

void StringSelectorSkeletalAnimationTemplate::replaceSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &oldValue, const CrcLowerString &newValue)
{
	//-- Ignore null selectionTemplate entries.
	if (!selectionTemplate)
		return;

	//-- Remove old mapping.
	const ValueMap::iterator oldMappingIt = m_valueMap->find(oldValue);
	if (oldMappingIt != m_valueMap->end())
	{
		m_valueMap->erase(oldMappingIt);
	}

	//-- Assign new mapping.
	const ValueMap::iterator lowerBoundResult = m_valueMap->lower_bound(newValue);
	if ((lowerBoundResult == m_valueMap->end()) || m_valueMap->key_comp()(newValue, lowerBoundResult->first))
	{
		// Entry doesn't exist, create it.
		IGNORE_RETURN(m_valueMap->insert(lowerBoundResult, ValueMap::value_type(newValue, selectionTemplate)));
	}
	else
	{
		// Entry already exists.
		if (lowerBoundResult->second == selectionTemplate)
		{
			// Value already set to specified entry.
			DEBUG_WARNING(true, ("duplicate selection template value mapping attempted [%s = 0x%08x].", newValue.getString(), selectionTemplate));
			return;
		}
		else
		{
			// Value mapped to something else.
			const SkeletalAnimationTemplate *oldTemplate = lowerBoundResult->second;
			lowerBoundResult->second = selectionTemplate;

			removeSelectionTemplateIfUnreferenced(oldTemplate);
		}
	}

	addSelectionTemplateIfNotPresent(selectionTemplate);
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::deleteSelectionAnimationTemplateMapping(const SkeletalAnimationTemplate *selectionTemplate, const CrcLowerString &value)
{
	//-- Ignore null selectionTemplate entries.
	if (!selectionTemplate)
		return;

	//-- Remove old mapping.
	const ValueMap::iterator mappingIt = m_valueMap->find(value);
	if (mappingIt != m_valueMap->end())
	{
		m_valueMap->erase(mappingIt);
	}
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::addSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate)
{
	if (!selectionTemplate)
	{
		DEBUG_WARNING(true, ("tried to add a null selector choice animation template."));
		return;
	}

	//-- Ensure selection template doesn't already exist.
	const TemplateVector::iterator findIt = std::find(m_templates->begin(), m_templates->end(), selectionTemplate);
	if (findIt != m_templates->end())
	{
		DEBUG_WARNING(true, ("tried to add the same selector choice template multiple times."));
		return;
	}

	m_templates->push_back(selectionTemplate);
	
	//-- Keep local reference.
	selectionTemplate->fetch();
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::replaceSelectionAnimationTemplate(const SkeletalAnimationTemplate *oldTemplate, const SkeletalAnimationTemplate *newTemplate)
{
	if (!oldTemplate)
	{
		//-- Treat as add, I don't allow null animation templates.
		addSelectionAnimationTemplate(newTemplate);
		return;
	}

	//-- Find entry for old template.
	const TemplateVector::iterator findIt = std::find(m_templates->begin(), m_templates->end(), oldTemplate);
	if (findIt == m_templates->end())
	{
		DEBUG_WARNING(true, ("tried to replace selector choice template but old template 0x%08x doesn't exist.", oldTemplate));
		return;
	}

	//-- Fetch reference to new template.
	if (!newTemplate)
	{
		DEBUG_WARNING(true, ("tried to replace old template with NULL new template, not supported."));
		return;
	}
	newTemplate->fetch();

	//-- Replace old template entry in template list.
	*findIt = newTemplate;

	//-- Replace old template entry in all value mappings.
	const ValueMap::iterator endIt = m_valueMap->end();
	for (ValueMap::iterator it = m_valueMap->begin(); it != endIt; ++it)
	{
		if (it->second == oldTemplate)
			it->second = newTemplate;
	}

	//-- Release old template.
	oldTemplate->release();
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::deleteSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate)
{
	if (!selectionTemplate)
		return;

	//-- Erase any values that map to the selection template.
	const ValueMap::iterator endIt = m_valueMap->end();
	for (ValueMap::iterator it = m_valueMap->begin(); it != endIt;)
	{
		if (it->second == selectionTemplate)
			m_valueMap->erase(it++);
		else
			++it;
	}

	//-- Clear the default if it points to this selection template.
	if (m_defaultTemplate == selectionTemplate)
		m_defaultTemplate = 0;
	
	//-- Remove the selectionTemplate from the list.
	TemplateVector::iterator findIt = std::find(m_templates->begin(), m_templates->end(), selectionTemplate);
	if (findIt != m_templates->end())
	{
		//-- Release local reference.
		IGNORE_RETURN(m_templates->erase(findIt));
		selectionTemplate->release();
	}
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::setDefaultSelectionAnimationTemplate(const SkeletalAnimationTemplate *selectionTemplate)
{
	if (m_defaultTemplate == selectionTemplate)
		return;

	//-- Assign new template.
	const SkeletalAnimationTemplate *oldDefault = m_defaultTemplate;
	m_defaultTemplate = selectionTemplate;

	//-- Clear default template.
	removeSelectionTemplateIfUnreferenced(oldDefault);

	//-- Add new template if necessary.
	addSelectionTemplateIfNotPresent(selectionTemplate);
}

// ----------------------------------------------------------------------

bool StringSelectorSkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_SSAT);
		iff.insertForm(TAG_0000);

			//-- Write variable name.
			iff.insertChunk(TAG_INFO);
				iff.insertChunkString(m_environmentVariableName.getString());
			iff.exitChunk(TAG_INFO);

			//-- Write selection animation templates.
			std::map<const SkeletalAnimationTemplate*, int>  selectionTemplateIndexMap;
			int assignedIndex = 0;
			
			iff.insertForm(TAG_ANMS);

				//-- Write # selection animation templates.
				// Assume I've kept a non-null list of selection animation templates.
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int16>(m_templates->size()));
				iff.exitChunk(TAG_INFO);

				const TemplateVector::const_iterator endIt = m_templates->end();
				for (TemplateVector::const_iterator it = m_templates->begin(); it != endIt; ++it)
				{
					//-- Write animation template.
					const SkeletalAnimationTemplate *const saTemplate = *it;
					NOT_NULL(saTemplate);
					DEBUG_FATAL(!saTemplate->supportsWriting(), ("animation template doesn't support writing."));

					saTemplate->write(iff);

					//-- Add animation template to template -> index map.
					selectionTemplateIndexMap[saTemplate] = assignedIndex;
					++assignedIndex;
				}

			iff.exitForm(TAG_ANMS);

			if (!m_valueMap->empty())
			{
				iff.insertChunk(TAG_VALS);

					//-- Write # value mappings.
					iff.insertChunkData(static_cast<int16>(m_valueMap->size()));

					//-- Enter each entry.
					const ValueMap::const_iterator vmEndIt = m_valueMap->end();
					for (ValueMap::const_iterator vmIt = m_valueMap->begin(); vmIt != vmEndIt; ++vmIt)
					{
						const int animationTemplateIndex = selectionTemplateIndexMap[vmIt->second];

						iff.insertChunkString(vmIt->first.getString());
						iff.insertChunkData(static_cast<int16>(animationTemplateIndex));
					}

				iff.exitChunk(TAG_VALS);
			}

			if (m_defaultTemplate)
			{
				iff.insertChunk(TAG_DFLT);
					iff.insertChunkData(static_cast<int16>(selectionTemplateIndexMap[m_defaultTemplate]));
				iff.exitChunk(TAG_DFLT);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_SSAT);
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s %s='%s'>", cs_stringSelectorAnimationElementName, cs_variableNameAttributeName, m_environmentVariableName.getString());
	writer.indent();
	{
		const TemplateVector::const_iterator endIt = m_templates->end();
		for (TemplateVector::const_iterator it = m_templates->begin(); it != endIt; ++it)
		{
			//-- Process each animation template choice.
			const SkeletalAnimationTemplate *const saTemplate = *it;
			if (!saTemplate || !saTemplate->supportsWriting())
			{
				DEBUG_WARNING(true, ("string selector animation template is NULL or doesn't support writing."));
				continue;
			}

			writer.writeLineFormat("<%s>", cs_choiceElementName);
			writer.indent();
			{
				//-- Write the animation template.
				writer.writeLineFormat("<%s>", cs_selectAnimationElementName);
				writer.indent();
				{
					saTemplate->writeXml(writer);
				}
				writer.unindent();
				writer.writeLineFormat("</%s>", cs_selectAnimationElementName);

				//-- Find the values that map to this animation template, write out a selectValue element for it.

				// Handle default value case: IFF-based format allowed a default animation to be specified without having 
				// 'default' show up in the value map.
				if (saTemplate == m_defaultTemplate)
				{
					ValueMap::iterator const findIt = m_valueMap->find(*s_defaultVariableValue);
					if (findIt == m_valueMap->end())
						writer.writeLineFormat("<%s %s='default'/>", cs_selectValueElementName, cs_valueAttributeName);
					else
						DEBUG_WARNING(findIt->second != saTemplate, ("StringSelector animation: default value mapping specified but maps to something other than the instance member variable default animation."));
				}

				const ValueMap::const_iterator vmEndIt = m_valueMap->end();
				for (ValueMap::const_iterator vmIt = m_valueMap->begin(); vmIt != vmEndIt; ++vmIt)
				{
					if (saTemplate == vmIt->second)
						writer.writeLineFormat("<%s %s='%s'/>", cs_selectValueElementName, cs_valueAttributeName, vmIt->first.getString());
				}
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_choiceElementName);
		}
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_stringSelectorAnimationElementName);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *StringSelectorSkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ======================================================================
// class StringSelectorSkeletalAnimationTemplate: private static member functions
// ======================================================================

void StringSelectorSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("StringSelectorSkeletalAnimationTemplate not installed"));

	ms_installed = false;

	delete ms_uiFactory;
	ms_uiFactory = 0;

	delete s_defaultVariableValue;
	s_defaultVariableValue = 0;

	const bool success = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_SSAT);
	DEBUG_FATAL(!success, ("deregisterCreateFunction() failed."));
	UNREF(success);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_stringSelectorAnimationElementName);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *StringSelectorSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("StringSelectorSkeletalAnimationTemplate not installed"));
	return new StringSelectorSkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *StringSelectorSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!ms_installed, ("StringSelectorSkeletalAnimationTemplate not installed"));
	return new StringSelectorSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class StringSelectorSkeletalAnimationTemplate: private member functions
// ======================================================================

StringSelectorSkeletalAnimationTemplate::StringSelectorSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_environmentVariableName(),
	m_templates(new TemplateVector()),
	m_valueMap(new ValueMap()),
	m_defaultTemplate(0)
{
	iff.enterForm(TAG_SSAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char  versionString[5];

					ConvertTagToString(version, versionString);
					DEBUG_FATAL(true, ("StringSelectorSkeletalAnimationTemplate version [%s] unsupported.", versionString));
				}
		}

	iff.exitForm(TAG_SSAT);
}

// ----------------------------------------------------------------------

StringSelectorSkeletalAnimationTemplate::StringSelectorSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_environmentVariableName(),
	m_templates(new TemplateVector()),
	m_valueMap(new ValueMap()),
	m_defaultTemplate(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("StringSelectorSkeletalAnimationTemplate: attempted to load unsupported version %d.", versionNumber));
	}
}

// ----------------------------------------------------------------------

StringSelectorSkeletalAnimationTemplate::~StringSelectorSkeletalAnimationTemplate()
{
	m_defaultTemplate = 0;

	delete m_valueMap;

	std::for_each(m_templates->begin(), m_templates->end(), VoidMemberFunction(&SkeletalAnimationTemplate::release));
	delete m_templates;
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	char buffer[1024];

	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			//-- Get variable name.
			iff.read_string(buffer, sizeof(buffer) - 1);
			m_environmentVariableName.setString(buffer);

		iff.exitChunk(TAG_INFO);

		//-- Handle animation templates.
		iff.enterForm(TAG_ANMS);
		{
			iff.enterChunk(TAG_INFO);
				const int templateCount = static_cast<int>(iff.read_int16());
			iff.exitChunk(TAG_INFO);

			m_templates->reserve(static_cast<TemplateVector::size_type>(templateCount));

			for (int i = 0; i < templateCount; ++i)
				m_templates->push_back(SkeletalAnimationTemplateList::fetch(iff));
		}
		iff.exitForm(TAG_ANMS);

		//-- Handle value mappings.
		if (iff.enterChunk(TAG_VALS, true))
		{
			const int valueCount = static_cast<int>(iff.read_int16());

			for (int i = 0; i < valueCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer) - 1);
				const int templateIndex = static_cast<int>(iff.read_int16());

				if ((templateIndex >= 0) && (templateIndex < static_cast<int>(m_templates->size())))
				{
					IGNORE_RETURN(m_valueMap->insert(ValueMap::value_type(CrcLowerString(buffer), (*m_templates)[static_cast<ValueMap::size_type>(templateIndex)])));
				}
			}

			iff.exitChunk(TAG_VALS);
		}

		if (iff.enterChunk(TAG_DFLT, true))
		{
			const int templateIndex = static_cast<int>(iff.read_int16());
			if ((templateIndex >= 0) && (templateIndex < static_cast<int>(m_templates->size())))
				m_defaultTemplate = (*m_templates)[static_cast<ValueMap::size_type>(templateIndex)];

			iff.exitChunk(TAG_DFLT);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	treeNode.assertIsElement(cs_stringSelectorAnimationElementName);

	//-- Get the variable name.
	treeNode.getElementAttributeAsString(cs_variableNameAttributeName, s_temporaryString);
	m_environmentVariableName.setString(s_temporaryString.c_str());

	//-- Get selections.
	for (XmlTreeNode choiceNode = treeNode.getFirstChildElementNode(); !choiceNode.isNull(); choiceNode = choiceNode.getNextSiblingElementNode())
	{
		choiceNode.assertIsElement(cs_choiceElementName);
		
		//-- Load the choice animation template.
		XmlTreeNode selectAnimationNode = choiceNode.getFirstChildElementNode();
		selectAnimationNode.assertIsElement(cs_selectAnimationElementName);

		SkeletalAnimationTemplate const *const animationTemplate = SkeletalAnimationTemplateList::fetch(selectAnimationNode.getFirstChildElementNode(), versionNumber);
		DEBUG_FATAL(!animationTemplate, ("StringSelectorSkeletalAnimationTemplate: failed to load a selection's an animation template."));
		m_templates->push_back(animationTemplate);

		//-- Load the values that map to this animation choice.
		for (XmlTreeNode selectValueNode = selectAnimationNode.getNextSiblingElementNode(); !selectValueNode.isNull(); selectValueNode = selectValueNode.getNextSiblingElementNode())
		{
			selectValueNode.assertIsElement(cs_selectValueElementName);
			
			selectValueNode.getElementAttributeAsString(cs_valueAttributeName, s_temporaryString);
			IGNORE_RETURN(m_valueMap->insert(ValueMap::value_type(CrcLowerString(s_temporaryString.c_str()), animationTemplate)));

			if (!_stricmp("default", s_temporaryString.c_str()))
				m_defaultTemplate = animationTemplate;
		}
	}
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::addSelectionTemplateIfNotPresent(const SkeletalAnimationTemplate *selectionTemplate)
{
	if (!selectionTemplate)
		return;

	//-- Remove the selectionTemplate from the list.
	TemplateVector::iterator findIt = std::find(m_templates->begin(), m_templates->end(), selectionTemplate);
	if (findIt == m_templates->end())
	{
		//-- Fetch local reference.
		selectionTemplate->fetch();

		//-- Add to list.
		m_templates->push_back(selectionTemplate);
	}
}

// ----------------------------------------------------------------------

void StringSelectorSkeletalAnimationTemplate::removeSelectionTemplateIfUnreferenced(const SkeletalAnimationTemplate *selectionTemplate)
{
	//-- Check if default maps to selection template.
	if (m_defaultTemplate == selectionTemplate)
		return;

	//-- Check if any value maps to the selection template.
	const ValueMap::iterator endIt = m_valueMap->end();
	for (ValueMap::iterator it = m_valueMap->begin(); it != endIt; ++it)
	{
		if (it->second == selectionTemplate)
		{
			// Value maps to it.
			return;
		}
	}

	//-- Nothing maps to the selection template.  Remove it.
	TemplateVector::iterator findIt = std::find(m_templates->begin(), m_templates->end(), selectionTemplate);
	if (findIt != m_templates->end())
	{
		//-- Remove from list.
		IGNORE_RETURN(m_templates->erase(findIt));

		//-- Release local reference.
		if (selectionTemplate)
			selectionTemplate->fetch();
	}
}

// ======================================================================
