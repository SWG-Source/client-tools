// ======================================================================
//
// PriorityBlendAnimationTemplate.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/PriorityBlendAnimationTemplate.h"

#include "clientSkeletalAnimation/PriorityBlendAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedXml/XmlTreeNode.h"

// ======================================================================

namespace PriorityBlendAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_COMP = TAG(C,O,M,P);
	Tag const TAG_PBAT = TAG(P,B,A,T);

	char const *const cs_priorityBlendAnimationElementName  = "priorityBlendAnimation";
	char const *const cs_primaryAnimationIndexAttributeName = "primaryAnimationIndex";
	char const *const cs_componentAnimationsElementName     = "componentAnimations";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                                s_installed;
	SkeletalAnimationTemplateUiFactory *s_uiFactory;
}

using namespace PriorityBlendAnimationTemplateNamespace;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(PriorityBlendAnimationTemplate, true, 0, 0, 0);

// ======================================================================
// class PriorityBlendAnimationTemplate: PUBLIC STATIC
// ======================================================================

void PriorityBlendAnimationTemplate::install()
{
	DEBUG_FATAL(s_installed, ("PriorityBlendAnimationTemplate is already installed."));

	//-- Register IFF create function.
	bool const registerSuccess = SkeletalAnimationTemplateList::registerCreateFunction(TAG_PBAT, create);
	FATAL(!registerSuccess, ("Failed to register required SkeletalAnimationTemplate-derived class: PriorityBlendAnimationTemplate"));

	//-- Register XML create function.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_priorityBlendAnimationElementName, xmlCreate);

	installMemoryBlockManager();

	s_uiFactory = NULL;

	s_installed = true;
	ExitChain::add(remove, "PriorityBlendAnimationTemplate");
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	if (uiFactory == s_uiFactory)
		return;

	delete s_uiFactory;
	s_uiFactory = uiFactory;
}

// ======================================================================
// class PriorityBlendAnimationTemplate: PUBLIC
// ======================================================================

SkeletalAnimation *PriorityBlendAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const componentAnimation0 = fetchNewComponentAnimation(0, animationEnvironment, transformNameMap);
	SkeletalAnimation *const componentAnimation1 = fetchNewComponentAnimation(1, animationEnvironment, transformNameMap);

	SkeletalAnimation *const newAnimation = new PriorityBlendAnimation(*this, transformNameMap, componentAnimation0, componentAnimation1, getPrimaryComponentAnimationIndex());
	newAnimation->fetch();

	if (componentAnimation0)
		componentAnimation0->release();

	if (componentAnimation1)
		componentAnimation1->release();

	return newAnimation;
}

// ----------------------------------------------------------------------

int PriorityBlendAnimationTemplate::getPrimaryComponentAnimationIndex() const
{
	return m_primaryComponentAnimationIndex;
}

// ----------------------------------------------------------------------

PriorityBlendAnimationTemplate::PriorityBlendAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_primaryComponentAnimationIndex(0)
{
	for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
		m_componentAnimationTemplateArray[i] = NULL;
}

// ----------------------------------------------------------------------

bool PriorityBlendAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_PBAT);
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<int8>(m_primaryComponentAnimationIndex));
			iff.exitChunk(TAG_INFO);

			for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
			{
				iff.insertForm(TAG_COMP);
					SkeletalAnimationTemplate const *const animationTemplate = m_componentAnimationTemplateArray[i];
					if (animationTemplate && animationTemplate->supportsWriting())
						animationTemplate->write(iff);
				iff.exitForm(TAG_COMP);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_PBAT);
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s %s='%d'>", cs_priorityBlendAnimationElementName, cs_primaryAnimationIndexAttributeName, m_primaryComponentAnimationIndex);
	writer.indent();
	{
		writer.writeLineFormat("<%s>", cs_componentAnimationsElementName);
		writer.indent();
		{
			for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
			{
				SkeletalAnimationTemplate const *const animationTemplate = m_componentAnimationTemplateArray[i];
				if (animationTemplate && animationTemplate->supportsWriting())
					animationTemplate->writeXml(writer);
			}
		}
		writer.unindent();
		writer.writeLineFormat("</%s>", cs_componentAnimationsElementName);
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_priorityBlendAnimationElementName);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate const *PriorityBlendAnimationTemplate::fetchComponentAnimationTemplate(int componentAnimationIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, componentAnimationIndex, static_cast<int>(MAX_ANIMATION_COUNT));

	// Fetch reference for caller.
	SkeletalAnimationTemplate const *const &componentAnimationTemplate = m_componentAnimationTemplateArray[componentAnimationIndex];
	if (componentAnimationTemplate)
		m_componentAnimationTemplateArray[componentAnimationIndex]->fetch();

	return m_componentAnimationTemplateArray[componentAnimationIndex];
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::setComponentAnimationTemplate(int componentAnimationIndex, SkeletalAnimationTemplate const *componentAnimationTemplate)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, componentAnimationIndex, static_cast<int>(MAX_ANIMATION_COUNT));
	
	//-- Fetch reference to new animation.
	if (componentAnimationTemplate)
		componentAnimationTemplate->fetch();

	//-- Release existing reference.
	if (m_componentAnimationTemplateArray[componentAnimationIndex])
		m_componentAnimationTemplateArray[componentAnimationIndex]->release();

	m_componentAnimationTemplateArray[componentAnimationIndex] = componentAnimationTemplate;
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::setPrimaryComponentAnimationIndex(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(MAX_ANIMATION_COUNT));
	m_primaryComponentAnimationIndex = index;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplateUiFactory const *PriorityBlendAnimationTemplate::getConstUiFactory() const
{
	return s_uiFactory;
}

// ======================================================================
// class PriorityBlendAnimationTemplate: PRIVATE STATIC
// ======================================================================

void PriorityBlendAnimationTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("PriorityBlendAnimationTemplate not installed."));
	s_installed = false;

	delete s_uiFactory;
	s_uiFactory = 0;

	removeMemoryBlockManager();

	bool const deregisterSuccess = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_PBAT);
	DEBUG_WARNING(!deregisterSuccess, ("SkeletalAnimationTemplateList::deregisterCreateFunction() failed for PriorityBlendAnimationTemplate."));
	UNREF(deregisterSuccess);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_priorityBlendAnimationElementName);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *PriorityBlendAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	return new PriorityBlendAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *PriorityBlendAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	return new PriorityBlendAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class PriorityBlendAnimationTemplate: PRIVATE
// ======================================================================

PriorityBlendAnimationTemplate::PriorityBlendAnimationTemplate(CrcLowerString const &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_primaryComponentAnimationIndex(-1)
{
	for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
		m_componentAnimationTemplateArray[i] = NULL;

	iff.enterForm(TAG_PBAT);

		Tag const version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("unsupported PriorityBlendAnimationTemplate version [%s].  Possibly file corruption or old data.", buffer));
				}
		}

	iff.exitForm(TAG_PBAT);
}

// ----------------------------------------------------------------------

PriorityBlendAnimationTemplate::PriorityBlendAnimationTemplate(CrcString const &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_primaryComponentAnimationIndex(-1)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("PriorityBlendAnimationTemplate: tried to load unsupported version number %d.", versionNumber));
	}
}

// ----------------------------------------------------------------------

PriorityBlendAnimationTemplate::~PriorityBlendAnimationTemplate()
{
	for (int i = 0; i < MAX_ANIMATION_COUNT; ++i)
	{
		if (m_componentAnimationTemplateArray[i])
		{
			m_componentAnimationTemplateArray[i]->release();
			m_componentAnimationTemplateArray[i] = 0;
		}
	}
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		
		iff.enterChunk(TAG_INFO);
			
			m_primaryComponentAnimationIndex = static_cast<int>(iff.read_int8());
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_primaryComponentAnimationIndex, static_cast<int>(MAX_ANIMATION_COUNT));

		iff.exitChunk(TAG_INFO);

		for (int componentAnimationIndex = 0; !iff.atEndOfForm() && (componentAnimationIndex < MAX_ANIMATION_COUNT); ++componentAnimationIndex)
		{
			iff.enterForm(TAG_COMP);
				m_componentAnimationTemplateArray[componentAnimationIndex] = (!iff.atEndOfForm()) ? SkeletalAnimationTemplateList::fetch(iff) : NULL;
			iff.exitForm(TAG_COMP);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void PriorityBlendAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	//-- Validate element name.
	{
		char const *const nodeName = treeNode.getName();
		FATAL(!treeNode.isElement() || _stricmp(nodeName, cs_priorityBlendAnimationElementName), ("expecting element [%s], found [%s].", cs_priorityBlendAnimationElementName, nodeName));
	}

	//-- Get the primary component animation index.
	treeNode.getElementAttributeAsInt(cs_primaryAnimationIndexAttributeName, m_primaryComponentAnimationIndex);

	//-- Get the component animations.
	XmlTreeNode componentAnimationsNode = treeNode.getFirstChildElementNode();

	char const *const nodeName = componentAnimationsNode.getName();
	FATAL(!componentAnimationsNode.isElement() || _stricmp(cs_componentAnimationsElementName, nodeName), ("expecting element [%s], found entity [%s].", cs_componentAnimationsElementName, nodeName));

	int componentIndex = 0;
	for (XmlTreeNode animationNode = componentAnimationsNode.getFirstChildElementNode(); !animationNode.isNull() && (componentIndex < MAX_ANIMATION_COUNT); animationNode = animationNode.getNextSiblingElementNode(), ++componentIndex)
		m_componentAnimationTemplateArray[componentIndex] = SkeletalAnimationTemplateList::fetch(animationNode, versionNumber);

	//-- Validate primary component animation index.
	FATAL((m_primaryComponentAnimationIndex < 0) || (m_primaryComponentAnimationIndex >= componentIndex), ("PriorityBlendAnimationTemplate specified primary component index value [%d] out of valid range [0 .. %d].", m_primaryComponentAnimationIndex, componentIndex));
}

// ----------------------------------------------------------------------

SkeletalAnimation *PriorityBlendAnimationTemplate::fetchNewComponentAnimation(int index, AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(MAX_ANIMATION_COUNT));
	return m_componentAnimationTemplateArray[index]->fetchSkeletalAnimation(animationEnvironment, transformNameMap);
}

// ======================================================================
