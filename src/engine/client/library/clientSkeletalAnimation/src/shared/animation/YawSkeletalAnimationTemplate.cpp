// ======================================================================
//
// YawSkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/YawSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/YawSkeletalAnimation.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace YawSkeletalAnimationTemplateNamespace
{
	Tag const TAG_NONE = TAG(N,O,N,E);
	Tag const TAG_YNEG = TAG(Y,N,E,G);
	Tag const TAG_YPOS = TAG(Y,P,O,S);
	Tag const TAG_YWAT = TAG(Y,W,A,T);

	char const *const cs_yawAnimationElementName   = "yawAnimation";
	char const *const cs_yawLeftChoiceElementName  = "yawLeftChoice";
	char const *const cs_yawRightChoiceElementName = "yawRightChoice";
	char const *const cs_yawNoneChoiceElementName  = "yawNoneChoice";
}

using namespace YawSkeletalAnimationTemplateNamespace;

// ======================================================================

bool                                YawSkeletalAnimationTemplate::ms_installed;
SkeletalAnimationTemplateUiFactory *YawSkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================
// class YawSkeletalAnimationTemplate: public static member functions
// ======================================================================

void YawSkeletalAnimationTemplate::install()
{
	//-- Ensure the system isn't already installed.
	DEBUG_FATAL(ms_installed, ("YawSkeletalAnimationTemplate already installed."));

	//-- Register IFF reader.
	const bool result = SkeletalAnimationTemplateList::registerCreateFunction(TAG_YWAT, create);
	DEBUG_FATAL(!result, ("failed to register yaw animation template's create function."));
	UNREF(result);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_yawAnimationElementName, xmlCreate);
	
	//-- Add system removal to the exit chain.
	ms_installed = true;
	ExitChain::add(remove, "YawSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	DEBUG_FATAL(!ms_installed, ("YawSkeletalAnimationTemplate not installed."));

	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete existing uiFactory.
	delete ms_uiFactory;

	//-- Assign new uiFactory.
	ms_uiFactory = uiFactory;
}

// ======================================================================
// class YawSkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *YawSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const animation = new YawSkeletalAnimation(*this, animationEnvironment, transformNameMap);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::garbageCollect() const
{
	//-- Forward call to all subordinate templates.
	if (m_yawLeftAnimationTemplate)
		m_yawLeftAnimationTemplate->garbageCollect();

	if (m_yawRightAnimationTemplate)
		m_yawRightAnimationTemplate->garbageCollect();

	if (m_noYawAnimationTemplate)
		m_noYawAnimationTemplate->garbageCollect();
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *YawSkeletalAnimationTemplate::fetchYawLeftAnimationTemplate() const
{
	// Fetch a reference for the caller.
	if (m_yawLeftAnimationTemplate)
		m_yawLeftAnimationTemplate->fetch();

	// Return the animation.
	return m_yawLeftAnimationTemplate;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *YawSkeletalAnimationTemplate::fetchYawRightAnimationTemplate() const
{
	// Fetch a reference for the caller.
	if (m_yawRightAnimationTemplate)
		m_yawRightAnimationTemplate->fetch();

	// Return the animation.
	return m_yawRightAnimationTemplate;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *YawSkeletalAnimationTemplate::fetchNoYawAnimationTemplate() const
{
	// Fetch a reference for the caller.
	if (m_noYawAnimationTemplate)
		m_noYawAnimationTemplate->fetch();

	// Return the animation.
	return m_noYawAnimationTemplate;
}

// ----------------------------------------------------------------------

YawSkeletalAnimationTemplate::YawSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_yawLeftAnimationTemplate(0),
	m_yawRightAnimationTemplate(0),
	m_noYawAnimationTemplate(0)
{
}

// ----------------------------------------------------------------------

bool YawSkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_YWAT);
		iff.insertForm(TAG_0000);

			//-- Write negative yaw data.
			if (m_yawLeftAnimationTemplate && m_yawLeftAnimationTemplate->supportsWriting())
			{
				iff.insertForm(TAG_YNEG);
					m_yawLeftAnimationTemplate->write(iff);
				iff.exitForm(TAG_YNEG);
			}

			//-- Write positive yaw data.
			if (m_yawRightAnimationTemplate && m_yawRightAnimationTemplate->supportsWriting())
			{
				iff.insertForm(TAG_YPOS);
					m_yawRightAnimationTemplate->write(iff);
				iff.exitForm(TAG_YPOS);
			}

			//-- Write no yaw (not rotating) data.
			if (m_noYawAnimationTemplate && m_noYawAnimationTemplate->supportsWriting())
			{
				iff.insertForm(TAG_NONE);
					m_noYawAnimationTemplate->write(iff);
				iff.exitForm(TAG_NONE);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_YWAT);
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s>", cs_yawAnimationElementName);
	writer.indent();
	{
		//-- Write negative yaw data.
		if (m_yawLeftAnimationTemplate && m_yawLeftAnimationTemplate->supportsWriting())
		{
			writer.writeLineFormat("<%s>", cs_yawLeftChoiceElementName);
			writer.indent();
			{
				m_yawLeftAnimationTemplate->writeXml(writer);
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_yawLeftChoiceElementName);
		}

		//-- Write positive yaw data.
		if (m_yawRightAnimationTemplate && m_yawRightAnimationTemplate->supportsWriting())
		{
			writer.writeLineFormat("<%s>", cs_yawRightChoiceElementName);
			writer.indent();
			{
				m_yawRightAnimationTemplate->writeXml(writer);
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_yawRightChoiceElementName);
		}

		//-- Handle not-rotating data.
		if (m_noYawAnimationTemplate && m_noYawAnimationTemplate->supportsWriting())
		{
			writer.writeLineFormat("<%s>", cs_yawNoneChoiceElementName);
			writer.indent();
			{
				m_noYawAnimationTemplate->writeXml(writer);
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_yawNoneChoiceElementName);
		}
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_yawAnimationElementName);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *YawSkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::setYawLeftAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Ignore if re-assigning same template.
	if (m_yawLeftAnimationTemplate == animationTemplate)
		return;

	//-- Release existing template.
	if (m_yawLeftAnimationTemplate)
	{
		m_yawLeftAnimationTemplate->release();
		m_yawLeftAnimationTemplate = 0;
	}

	//-- Assign new entry and fetch reference.
	m_yawLeftAnimationTemplate = animationTemplate;
	if (m_yawLeftAnimationTemplate)
		m_yawLeftAnimationTemplate->fetch();
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::setYawRightAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Ignore if re-assigning same template.
	if (m_yawRightAnimationTemplate == animationTemplate)
		return;

	//-- Release existing template.
	if (m_yawRightAnimationTemplate)
	{
		m_yawRightAnimationTemplate->release();
		m_yawRightAnimationTemplate = 0;
	}

	//-- Assign new entry and fetch reference.
	m_yawRightAnimationTemplate = animationTemplate;
	if (m_yawRightAnimationTemplate)
		m_yawRightAnimationTemplate->fetch();
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::setNoYawAnimationTemplate(const SkeletalAnimationTemplate *animationTemplate)
{
	//-- Ignore if re-assigning same template.
	if (m_noYawAnimationTemplate == animationTemplate)
		return;

	//-- Release existing template.
	if (m_noYawAnimationTemplate)
	{
		m_noYawAnimationTemplate->release();
		m_noYawAnimationTemplate = 0;
	}

	//-- Assign new entry and fetch reference.
	m_noYawAnimationTemplate = animationTemplate;
	if (m_noYawAnimationTemplate)
		m_noYawAnimationTemplate->fetch();
}

// ======================================================================
// class YawSkeletalAnimationTemplate: private static member functions
// ======================================================================

void YawSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("YawSkeletalAnimationTemplate not installed."));
	
	ms_installed = false;

	const bool result = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_YWAT);
	DEBUG_FATAL(!result, ("failed to deregister speed animation temlpate's create function."));
	UNREF(result);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_yawAnimationElementName);

	delete ms_uiFactory;
	ms_uiFactory = 0;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *YawSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	DEBUG_FATAL(!ms_installed, ("YawSkeletalAnimationTemplate not installed."));

	return new YawSkeletalAnimationTemplate(name, iff);
}

SkeletalAnimationTemplate *YawSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!ms_installed, ("YawSkeletalAnimationTemplate not installed."));
	return new YawSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================

YawSkeletalAnimationTemplate::YawSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_yawLeftAnimationTemplate(0),
	m_yawRightAnimationTemplate(0),
	m_noYawAnimationTemplate(0)
{
	iff.enterForm(TAG_YWAT);

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
					DEBUG_FATAL(true, ("unsupported YawSkeletalAnimationTemplate format [%s]", buffer));
				}
		}

	iff.exitForm(TAG_YWAT);
}

// ----------------------------------------------------------------------

YawSkeletalAnimationTemplate::YawSkeletalAnimationTemplate(CrcString const &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_yawLeftAnimationTemplate(0),
	m_yawRightAnimationTemplate(0),
	m_noYawAnimationTemplate(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("YawSkeletalAnimationTemplate: tried to load an unsupported version %d.", versionNumber));
	}
}

// ----------------------------------------------------------------------

YawSkeletalAnimationTemplate::~YawSkeletalAnimationTemplate()
{
	//-- Release all animation templates.
	if (m_yawLeftAnimationTemplate)
	{
		m_yawLeftAnimationTemplate->release();
		m_yawLeftAnimationTemplate = 0;
	}

	if (m_yawRightAnimationTemplate)
	{
		m_yawRightAnimationTemplate->release();
		m_yawRightAnimationTemplate = 0;
	}

	if (m_noYawAnimationTemplate)
	{
		m_noYawAnimationTemplate->release();
		m_noYawAnimationTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Fetch the Yaw CCW entry.
		if (iff.enterForm(TAG_YNEG, true))
		{
			IS_NULL(m_yawLeftAnimationTemplate);
			m_yawLeftAnimationTemplate = SkeletalAnimationTemplateList::fetch(iff);

			iff.exitForm(TAG_YNEG);
		}

		//-- Fetch the Yaw CW entry.
		if (iff.enterForm(TAG_YPOS, true))
		{
			IS_NULL(m_yawRightAnimationTemplate);
			m_yawRightAnimationTemplate = SkeletalAnimationTemplateList::fetch(iff);

			iff.exitForm(TAG_YPOS);
		}

		//-- Fetch the Yaw CW entry.
		if (iff.enterForm(TAG_NONE, true))
		{
			IS_NULL(m_noYawAnimationTemplate);
			m_noYawAnimationTemplate = SkeletalAnimationTemplateList::fetch(iff);

			iff.exitForm(TAG_NONE);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void YawSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	treeNode.assertIsElement(cs_yawAnimationElementName);

	XmlTreeNode nextNode = treeNode.getFirstChildElementNode();
	if (!_stricmp(nextNode.getName(), cs_yawLeftChoiceElementName))
	{
		//-- Load animation.
		IS_NULL(m_yawLeftAnimationTemplate);
		m_yawLeftAnimationTemplate = SkeletalAnimationTemplateList::fetch(nextNode.getFirstChildElementNode(), versionNumber);

		//-- Move to next element.
		nextNode = nextNode.getNextSiblingElementNode();
	}

	if (!_stricmp(nextNode.getName(), cs_yawRightChoiceElementName))
	{
		//-- Load animation.
		IS_NULL(m_yawRightAnimationTemplate);
		m_yawRightAnimationTemplate = SkeletalAnimationTemplateList::fetch(nextNode.getFirstChildElementNode(), versionNumber);

		//-- Move to next element.
		nextNode = nextNode.getNextSiblingElementNode();
	}

	if (!_stricmp(nextNode.getName(), cs_yawNoneChoiceElementName))
	{
		//-- Load animation.
		IS_NULL(m_noYawAnimationTemplate);
		m_noYawAnimationTemplate = SkeletalAnimationTemplateList::fetch(nextNode.getFirstChildElementNode(), versionNumber);

		//-- Move to next element.
		nextNode = nextNode.getNextSiblingElementNode();
	}
}

// ======================================================================
