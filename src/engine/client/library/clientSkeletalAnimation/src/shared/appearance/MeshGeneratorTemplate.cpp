// ======================================================================
//
// MeshGeneratorTemplate.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"

#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"

// ======================================================================

bool MeshGeneratorTemplate::hasOnlyNonCollidableShaderTemplates() const
{
	//-- Defaults to false.
	return false;
}

// ----------------------------------------------------------------------

/**
 * Release the reference count on this instance.
 *
 * When the reference count becomes less than one, the instance
 * will be deleted.
 *
 * The caller must maintain a reference count for each independent reference
 * to this instance.  Failure to do so most likely will result in a piece
 * of code referencing an already-deleted instance.  Likewise, the caller
 * must ensure release() is called whenever a reference to this instance
 * no longer is needed.
 */

void MeshGeneratorTemplate::release() const
{
	--m_referenceCount;

	if (m_referenceCount < 1)
	{
		DEBUG_WARNING(m_referenceCount < 0, ("MeshGeneratorTemplate: bad reference counting code [%d].\n", m_referenceCount));

		MeshGeneratorTemplateList::stopTracking(*this);
		delete const_cast<MeshGeneratorTemplate*>(this);
	}
}

// ======================================================================
/**
 * Construct a new MeshGeneratorTemplate instance with a given pathName.
 *
 * @param name  the TreeFile pathName from which the instance was created.
 */

MeshGeneratorTemplate::MeshGeneratorTemplate(const CrcString &name) :
	m_name(name),
	m_referenceCount(0)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy the MeshGeneratorTemplate instance.
 *
 * This invocation releases all resources associated with the instance.
 *
 * Typically this should be called via release() when the reference count
 * falls below one reference.
 */

MeshGeneratorTemplate::~MeshGeneratorTemplate()
{
}

// ======================================================================
