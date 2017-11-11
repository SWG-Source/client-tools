// ======================================================================
//
// SkeletonTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"

#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "sharedFoundation/CrcLowerString.h"

// ======================================================================
// class SkeletonTemplate
// ======================================================================

void SkeletonTemplate::fetch() const
{
	++m_referenceCount;
	// DEBUG_REPORT_LOG(true, ("SKEL: fetch(%s), ref = %d.\n", getName().getString(), m_referenceCount));
}

// ----------------------------------------------------------------------

void SkeletonTemplate::release() const
{
	--m_referenceCount;

	// DEBUG_REPORT_LOG(true, ("SKEL: release(%s), ref = %d.\n", getName().getString(), m_referenceCount));
	if (m_referenceCount < 1)
	{
		SkeletonTemplateList::stopTracking(*this);
		delete const_cast<SkeletonTemplate*>(this);
	}
}

// ======================================================================

SkeletonTemplate::SkeletonTemplate(CrcString const &name) :
	m_referenceCount(0),
	m_name(name)
{
}

// ----------------------------------------------------------------------

SkeletonTemplate::~SkeletonTemplate()
{
	DEBUG_WARNING(m_referenceCount, ("suspicious SkeletonTemplate delete with non-zero ref count [%d]", m_referenceCount));
}

// ======================================================================
