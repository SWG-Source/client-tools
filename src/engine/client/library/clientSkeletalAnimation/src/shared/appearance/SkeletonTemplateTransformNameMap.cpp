// ======================================================================
//
// SkeletonTemplateTransformNameMap.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletonTemplateTransformNameMap.h"

#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"

// ======================================================================

SkeletonTemplateTransformNameMap::SkeletonTemplateTransformNameMap(const BasicSkeletonTemplate &skeletonTemplate) :
	m_skeletonTemplate(skeletonTemplate)
{
	// Don't fetch --- if I do, the BasicSkeletonTemplate will never be releasable.  The BasicSkeletonTemplate
	// has a TransformNameMap, which causes a ref count of 1 that never goes away until the BST is deleted, but
	// the BST won't get deleted until all references are gone.

	// All the same, I don't want this name map to be used after the skeleton is killed.
	// Consider using a watcher here.

	//-- Fetch local reference.
	// m_skeletonTemplate.fetch();
}

// ----------------------------------------------------------------------

SkeletonTemplateTransformNameMap::~SkeletonTemplateTransformNameMap()
{
	//-- Release local reference.
	// m_skeletonTemplate.release();
}

// ----------------------------------------------------------------------

void SkeletonTemplateTransformNameMap::findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const
{
	m_skeletonTemplate.findJointIndex(name, transformIndex, found);
}

// ----------------------------------------------------------------------

int SkeletonTemplateTransformNameMap::getTransformIndex(CrcString const &name) const
{
	return m_skeletonTemplate.getJointIndex(name);
}

// ----------------------------------------------------------------------

int SkeletonTemplateTransformNameMap::getTransformCount() const
{
	return m_skeletonTemplate.getJointCount();
}

// ----------------------------------------------------------------------

CrcString const &SkeletonTemplateTransformNameMap::getTransformName(int index) const
{
	return m_skeletonTemplate.getJointName(index);
}

// ======================================================================
