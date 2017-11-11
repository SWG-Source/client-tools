// ======================================================================
//
// TimeScaleSkeletalAnimationTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "clientSkeletalAnimation/TimeScaleSkeletalAnimation.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedXml/XmlTreeNode.h"

// ======================================================================

namespace TimeScaleSkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_TSCL = TAG(T,S,C,L);

	char const *const cs_timeScaleAnimationElementName    = "timeScaleAnimation";
	char const *const cs_frameRateMultiplierAttributeName = "frameRateMultiplier";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                                s_installed;
	SkeletalAnimationTemplateUiFactory *s_uiFactory;
}

using namespace TimeScaleSkeletalAnimationTemplateNamespace;

// ======================================================================
// class TimeScaleSkeletalAnimationTemplate: PUBLIC STATIC
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(TimeScaleSkeletalAnimationTemplate, true, 0, 0, 0);

// ======================================================================

void TimeScaleSkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(s_installed, ("TimeScaleSkeletalAnimationTemplate already installed."));

	//-- Setup memory block manager.	
	installMemoryBlockManager();

	//-- Register IFF reader.
	bool const registerSuccess = SkeletalAnimationTemplateList::registerCreateFunction(TAG_TSCL, create);
	DEBUG_FATAL(!registerSuccess, ("failed to register binding."));
	UNREF(registerSuccess);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_timeScaleAnimationElementName, xmlCreate);

	s_installed = true;
	ExitChain::add(remove, "TimeScaleSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	s_uiFactory = uiFactory;
}

// ======================================================================
// class TimeScaleSkeletalAnimationTemplate: PUBLIC
// ======================================================================

SkeletalAnimation *TimeScaleSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	if (m_baseAnimationTemplate)
	{
		SkeletalAnimation *baseSkeletalAnimation = m_baseAnimationTemplate->fetchSkeletalAnimation(animationEnvironment, transformNameMap);
		if (baseSkeletalAnimation)
		{
			SkeletalAnimation *const newAnimation = new TimeScaleSkeletalAnimation(this, baseSkeletalAnimation);
			newAnimation->fetch();

			// Release local references.
			baseSkeletalAnimation->release();

			return newAnimation;
		}
		else
		{
			DEBUG_WARNING(true, ("TimeScaleSkeletalAnimationTemplate::fetchSkeletalAnimation(): base animation template produced a NULL animation."));
			return 0;
		}
	}
	else
	{
		DEBUG_WARNING(true, ("TimeScaleSkeletalAnimationTemplate::fetchSkeletalAnimation() called with NULL base animation template."));
		return 0;
	}
}

// ----------------------------------------------------------------------

bool TimeScaleSkeletalAnimationTemplate::supportsWriting() const
{
	return (m_baseAnimationTemplate && m_baseAnimationTemplate->supportsWriting());
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::write(Iff & iff) const
{
	DEBUG_FATAL(!supportsWriting(), ("TimeScaleSkeletalAnimationTemplate: this class or instance doesn't support writing."));

	iff.insertForm(TAG_TSCL);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(m_timeScale);
			iff.exitChunk(TAG_INFO);

			NOT_NULL(m_baseAnimationTemplate);
			m_baseAnimationTemplate->write(iff);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_TSCL);
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	if (!m_baseAnimationTemplate || !m_baseAnimationTemplate->supportsWriting())
	{
		DEBUG_WARNING(true, ("TimeScaleSkeletalAnimationTemplate: the base animation template doesn't support writing."));
		return;
	}

	writer.writeLineFormat("<%s %s='%f'>", cs_timeScaleAnimationElementName, cs_frameRateMultiplierAttributeName, m_timeScale);
	writer.indent();
	{
		m_baseAnimationTemplate->writeXml(writer);
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_timeScaleAnimationElementName);
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *TimeScaleSkeletalAnimationTemplate::getConstUiFactory() const
{
	return s_uiFactory;
}

// ----------------------------------------------------------------------

TimeScaleSkeletalAnimationTemplate::TimeScaleSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_timeScale(1.0f),
	m_oneOverTimeScale(1.0f),
	m_baseAnimationTemplate(0)
{
}

// ======================================================================
// class TimeScaleSkeletalAnimationTemplate: PROTECTED
// ======================================================================

TimeScaleSkeletalAnimationTemplate::~TimeScaleSkeletalAnimationTemplate()
{
	if (m_baseAnimationTemplate)
	{
		m_baseAnimationTemplate->release();
		m_baseAnimationTemplate = 0;
	}
}

// ======================================================================
// class TimeScaleSkeletalAnimationTemplate: PRIVATE STATIC
// ======================================================================

void TimeScaleSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("TimeScaleSkeletalAnimationTemplate not installed."));
	s_installed = false;

	IGNORE_RETURN(SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_TSCL));
	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_timeScaleAnimationElementName);

	delete s_uiFactory;
	s_uiFactory = 0;

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *TimeScaleSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	DEBUG_FATAL(!s_installed, ("TimeScaleSkeletalAnimationTemplate not installed."));
	return new TimeScaleSkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *TimeScaleSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	DEBUG_FATAL(!s_installed, ("TimeScaleSkeletalAnimationTemplate not installed."));
	return new TimeScaleSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class TimeScaleSkeletalAnimationTemplate: PRIVATE
// ======================================================================

TimeScaleSkeletalAnimationTemplate::TimeScaleSkeletalAnimationTemplate(CrcLowerString const &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_timeScale(1.0f),
	m_oneOverTimeScale(1.0f),
	m_baseAnimationTemplate(0)
{
	iff.enterForm(TAG_TSCL);

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
					DEBUG_FATAL(true, ("TimeScaleSkeletalAnimationTemplate: unsupported version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_TSCL);
}

// ----------------------------------------------------------------------

TimeScaleSkeletalAnimationTemplate::TimeScaleSkeletalAnimationTemplate(CrcString const &name, XmlTreeNode const &treeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_timeScale(1.0f),
	m_oneOverTimeScale(1.0f),
	m_baseAnimationTemplate(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, versionNumber);
			break;

		default:
			FATAL(true, ("TimeScaleSkeletalAnimationTemplate: tried to load unsupported version %d.", versionNumber));
	}
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);
			setTimeScale(iff.read_float());
		iff.exitChunk(TAG_INFO);

		m_baseAnimationTemplate = SkeletalAnimationTemplateList::fetch(iff);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	treeNode.assertIsElement(cs_timeScaleAnimationElementName);

	//-- Load the frame rate multiplier.
	float frameRateMultiplier = 1.0f;
	treeNode.getElementAttributeAsFloat(cs_frameRateMultiplierAttributeName, frameRateMultiplier);
	setTimeScale(frameRateMultiplier);

	//-- Load the animation.
	m_baseAnimationTemplate = SkeletalAnimationTemplateList::fetch(treeNode.getFirstChildElementNode(), versionNumber);
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::setTimeScale(float timeScale)
{
	if (timeScale <= 0.0f)
	{
		DEBUG_WARNING(true, ("setTimeScale(): invalid scale [%g], cannot accept values <= 0.", timeScale));
		return;
	}

	m_timeScale        = timeScale;
	m_oneOverTimeScale = 1.0f / timeScale;
}

// ----------------------------------------------------------------------

float TimeScaleSkeletalAnimationTemplate::getTimeScale() const
{
	return m_timeScale;
}

// ----------------------------------------------------------------------

float TimeScaleSkeletalAnimationTemplate::getOneOverTimeScale() const
{
	return m_oneOverTimeScale;
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::setBaseAnimationTemplate(SkeletalAnimationTemplate const *animationTemplate)
{
	if (animationTemplate)
		animationTemplate->fetch();

	if (m_baseAnimationTemplate)
		m_baseAnimationTemplate->release();

	m_baseAnimationTemplate = animationTemplate;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate const *TimeScaleSkeletalAnimationTemplate::fetchBaseAnimationTemplate() const
{
	if (m_baseAnimationTemplate)
		m_baseAnimationTemplate->fetch();

	return m_baseAnimationTemplate;
}

// ----------------------------------------------------------------------

void TimeScaleSkeletalAnimationTemplate::garbageCollect() const
{
	//-- Forward on call to subordinate templates.
	if (m_baseAnimationTemplate)
		m_baseAnimationTemplate->garbageCollect();
}

// ======================================================================
