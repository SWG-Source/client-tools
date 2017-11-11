// ======================================================================
//
// BasicMeshGeneratorTemplate.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"

// #include "sharedFoundation/CrcLowerString.h"

// ======================================================================

int BasicMeshGeneratorTemplate::getDetailCount() const
{
	//-- This MeshGeneratorTemplate is never more than a single detail level.
	return 1;
}

// ----------------------------------------------------------------------

const BasicMeshGeneratorTemplate *BasicMeshGeneratorTemplate::fetchBasicMeshGeneratorTemplate(int detailIndex) const
{
	//-- Ensure caller is requesting the one and only BasicMeshGeneratorTemplate.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, 1);
	UNREF(detailIndex);

	//-- Fetch reference for caller.
	fetch();

	//-- Return this to caller.
	return this;
}

// ======================================================================
/**
 * Construct a new BasicMeshGeneratorTemplate instance with a given pathName.
 *
 * @param name  the TreeFile pathName from which the instance was created.
 */

BasicMeshGeneratorTemplate::BasicMeshGeneratorTemplate(CrcString const &name) :
	MeshGeneratorTemplate(name)
{
}

// ======================================================================
