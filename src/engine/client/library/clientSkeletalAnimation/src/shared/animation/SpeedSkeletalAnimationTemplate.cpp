// ======================================================================
//
// SpeedSkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/SpeedSkeletalAnimation.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace SpeedSkeletalAnimationTemplateNamespace
{
	Tag const TAG_SPAT = TAG(S,P,A,T);

	char const *const cs_speedAnimationElementName = "speedAnimation";
	char const *const cs_choiceElementName         = "choice";
}

using namespace SpeedSkeletalAnimationTemplateNamespace;

// ======================================================================

bool                                SpeedSkeletalAnimationTemplate::ms_installed;
SkeletalAnimationTemplateUiFactory *SpeedSkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================
// class SpeedSkeletalAnimationTemplate: public static member functions
// ======================================================================

void SpeedSkeletalAnimationTemplate::install()
{
	//-- Ensure the system isn't already installed.
	DEBUG_FATAL(ms_installed, ("SpeedSkeletalAnimationTemplate already installed."));

	//-- Register IFF reader.
	const bool result = SkeletalAnimationTemplateList::registerCreateFunction(TAG_SPAT, create);
	DEBUG_FATAL(!result, ("failed to register speed animation temlpate's create function."));
	UNREF(result);

	//-- Register Xml reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_speedAnimationElementName, xmlCreate);

	//-- Add system removal to the exit chain.
	ms_installed = true;
	ExitChain::add(remove, "SpeedSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	DEBUG_FATAL(!ms_installed, ("SpeedSkeletalAnimationTemplate not installed."));

	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete existing uiFactory.
	delete ms_uiFactory;

	//-- Assign new uiFactory.
	ms_uiFactory = uiFactory;
}

// ======================================================================
// class SpeedSkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *SpeedSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const animation = new SpeedSkeletalAnimation(*this, animationEnvironment, transformNameMap);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimationTemplate::getSpeedChoiceCount() const
{
	return static_cast<int>(m_choiceTemplates->size());
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *SpeedSkeletalAnimationTemplate::fetchSpeedChoiceAnimationTemplate(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getSpeedChoiceCount());
	const SkeletalAnimationTemplate *const choiceTemplate = (*m_choiceTemplates)[static_cast<SkeletalAnimationTemplateVector::size_type>(index)];

	if (choiceTemplate)
		choiceTemplate->fetch();

	return choiceTemplate;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::garbageCollect() const
{
	NOT_NULL(m_choiceTemplates);

	//-- Forward on garbageCollect() call to all subordinate templates.
	SkeletalAnimationTemplateVector::iterator const endIt = m_choiceTemplates->end();
	for (SkeletalAnimationTemplateVector::iterator it = m_choiceTemplates->begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->garbageCollect();
	}
}

// ----------------------------------------------------------------------

SpeedSkeletalAnimationTemplate::SpeedSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_choiceTemplates(new SkeletalAnimationTemplateVector())
{
}

// ----------------------------------------------------------------------

bool SpeedSkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_SPAT);
		iff.insertForm(TAG_0000);

			//-- Write # choice templates.
			iff.insertChunk(TAG_INFO);

				const int writableChoiceCount = countWritableTemplates();
				iff.insertChunkData(static_cast<int8>(writableChoiceCount));				

			iff.exitChunk(TAG_INFO);

			//-- Write the choice templates.
			const SkeletalAnimationTemplateVector::const_iterator endIt = m_choiceTemplates->end();
			for (SkeletalAnimationTemplateVector::const_iterator it = m_choiceTemplates->begin(); it != endIt; ++it)
			{
				const SkeletalAnimationTemplate *const choiceTemplate = *it;
				if (choiceTemplate && choiceTemplate->supportsWriting())
					choiceTemplate->write(iff);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_SPAT);
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s>", cs_speedAnimationElementName);
	writer.indent();
	{
		const SkeletalAnimationTemplateVector::const_iterator endIt = m_choiceTemplates->end();
		for (SkeletalAnimationTemplateVector::const_iterator it = m_choiceTemplates->begin(); it != endIt; ++it)
		{
			const SkeletalAnimationTemplate *const choiceTemplate = *it;
			if (choiceTemplate && choiceTemplate->supportsWriting())
			{
				writer.writeLineFormat("<%s>", cs_choiceElementName);
				writer.indent();
				{
					choiceTemplate->writeXml(writer);
				}
				writer.unindent();
				writer.writeLineFormat("</%s>", cs_choiceElementName);
			}
		}
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_speedAnimationElementName);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *SpeedSkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::addSpeedChoiceAnimationTemplate(const SkeletalAnimationTemplate *newChoiceTemplate)
{
	//-- Ignore null templates.
	if (!newChoiceTemplate)
		return;

	//-- Check to make sure template doesn't already exist in list.
	const SkeletalAnimationTemplateVector::const_iterator findIt = std::find(m_choiceTemplates->begin(), m_choiceTemplates->end(), newChoiceTemplate);
	if (findIt != m_choiceTemplates->end())
	{
		WARNING(true, ("tried to add skeletal animation template to speed template but already is a speed choice."));
		return;
	}

	//-- Fetch local reference.
	newChoiceTemplate->fetch();

	//-- Add the speed choice entry.
	m_choiceTemplates->push_back(newChoiceTemplate);
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::deleteSpeedChoiceAnimationTemplate(const SkeletalAnimationTemplate *newChoiceTemplate)
{
	//-- Find speed choice entry.
	const SkeletalAnimationTemplateVector::iterator findIt = std::find(m_choiceTemplates->begin(), m_choiceTemplates->end(), newChoiceTemplate);
	if (findIt == m_choiceTemplates->end())
	{
		WARNING(true, ("tried to delete skeletal animation template but doesn't exist."));
		return;
	}

	//-- Release local reference.
	if (*findIt)
		(*findIt)->release();

	//-- Destroy entry.
	IGNORE_RETURN(m_choiceTemplates->erase(findIt));
}

// ======================================================================
// class SpeedSkeletalAnimationTemplate: private static member functions
// ======================================================================

void SpeedSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("SpeedSkeletalAnimationTemplate not installed."));
	
	ms_installed = false;

	const bool result = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_SPAT);
	DEBUG_FATAL(!result, ("failed to deregister speed animation temlpate's create function."));
	UNREF(result);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_speedAnimationElementName);

	delete ms_uiFactory;
	ms_uiFactory = 0;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *SpeedSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("SpeedSkeletalAnimationTemplate not installed."));

	return new SpeedSkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *SpeedSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!ms_installed, ("SpeedSkeletalAnimationTemplate not installed."));
	return new SpeedSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================

SpeedSkeletalAnimationTemplate::SpeedSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_choiceTemplates(new SkeletalAnimationTemplateVector())
{
	iff.enterForm(TAG_SPAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported SpeedSkeletalAnimationTemplate format [%s]", buffer));
				}
		}

	iff.exitForm(TAG_SPAT);
}

// ----------------------------------------------------------------------

SpeedSkeletalAnimationTemplate::SpeedSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_choiceTemplates(new SkeletalAnimationTemplateVector())
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("SpeedSkeletalAnimationTemplate: tried to load unsupported version %d", versionNumber));
	}
}

// ----------------------------------------------------------------------

SpeedSkeletalAnimationTemplate::~SpeedSkeletalAnimationTemplate()
{
	//-- Release all choice templates.
	std::for_each(m_choiceTemplates->begin(), m_choiceTemplates->end(), VoidMemberFunction(&SkeletalAnimationTemplate::release));
	delete m_choiceTemplates;
}

// ----------------------------------------------------------------------

int SpeedSkeletalAnimationTemplate::countWritableTemplates() const
{
	int count = 0;

	const SkeletalAnimationTemplateVector::const_iterator endIt = m_choiceTemplates->end();
	for (SkeletalAnimationTemplateVector::const_iterator it = m_choiceTemplates->begin(); it != endIt; ++it)
	{
		const SkeletalAnimationTemplate *const choiceTemplate = *it;
		if (choiceTemplate && choiceTemplate->supportsWriting())
			++count;
	}

	return count;
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Load # choice templates.
		iff.enterChunk(TAG_INFO);

			const int choiceTemplateCount = static_cast<int>(iff.read_int8());
			DEBUG_FATAL(choiceTemplateCount < 0, ("bad choice template count [%d].", choiceTemplateCount));

		iff.exitChunk(TAG_INFO);	

		//-- Load choice templates.
		m_choiceTemplates->reserve(static_cast<SkeletalAnimationTemplateVector::size_type>(choiceTemplateCount));

		for (int i = 0; i < choiceTemplateCount; ++i)
		{
			m_choiceTemplates->push_back(SkeletalAnimationTemplateList::fetch(iff));
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void SpeedSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	treeNode.assertIsElement(cs_speedAnimationElementName);

	for (XmlTreeNode choiceNode = treeNode.getFirstChildElementNode(); !choiceNode.isNull(); choiceNode = choiceNode.getNextSiblingElementNode())
	{
		choiceNode.assertIsElement(cs_choiceElementName);
		m_choiceTemplates->push_back(SkeletalAnimationTemplateList::fetch(choiceNode.getFirstChildElementNode(), versionNumber));
	}
}

// ======================================================================
