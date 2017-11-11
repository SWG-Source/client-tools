// ======================================================================
//
// SkeletalAnimationTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

namespace SkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char  s_crashReportInfo[MAX_PATH * 2];
}

using namespace SkeletalAnimationTemplateNamespace;

// ======================================================================

void SkeletalAnimationTemplateNamespace::remove()
{
	CrashReportInformation::removeDynamicText(s_crashReportInfo);
}

// ======================================================================

void SkeletalAnimationTemplate::install()
{
	s_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText(s_crashReportInfo);

	ExitChain::add(remove, "SkeletalAnimationTemplate");
}

// ======================================================================

SkeletalAnimationTemplate::SkeletalAnimationTemplate(const CrcString &name)
:	m_name(name),
	m_referenceCount(0)
{
	//-- Keep track of most recently loaded skeletal animation template name
	//   for live crash tracking.
	IGNORE_RETURN(snprintf(s_crashReportInfo, sizeof(s_crashReportInfo) - 1, "SkeletalAnimationTemplate: %s\n", name.getString()));
	s_crashReportInfo[sizeof(s_crashReportInfo) - 1] = '\0';
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate::~SkeletalAnimationTemplate()
{
	DEBUG_WARNING(m_referenceCount != 0, ("invalid reference count [%d], should be zero", m_referenceCount));
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplate::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		// remove ourselves from the template list
		SkeletalAnimationTemplateList::stopTracking(this);

		// delete ourselves
		delete const_cast<SkeletalAnimationTemplate*>(this);
	}
}

// ----------------------------------------------------------------------

bool SkeletalAnimationTemplate::supportsWriting() const
{
	// By default, nothing supports writing.
	return false;
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplate::write(Iff & /* iff */) const
{
	// Shouldn't get here.  If we got here, some code either ignored the supportsWriting()
	// return value or forgot to override write().
	DEBUG_FATAL(true, ("tried to write a SkeletalAnimationTemplate using default write routine."));
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplate::writeXml(IndentedFileWriter &) const
{
	// Shouldn't get here.  If we got here, some code either ignored the supportsWriting()
	// return value or forgot to override write().
	DEBUG_FATAL(true, ("tried to write an XML version of a SkeletalAnimationTemplate-derived class using the default write routine."));
	// writer.writeLine("<bogusAnimation pleaseFix='true' />");
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *SkeletalAnimationTemplate::getConstUiFactory() const
{
	// Default behavior: return no UiFactory
	return 0;
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplate::garbageCollect() const
{
	// Do nothing.
}

// ======================================================================
