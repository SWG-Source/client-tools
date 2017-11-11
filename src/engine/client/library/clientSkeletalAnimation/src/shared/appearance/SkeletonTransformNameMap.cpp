// ======================================================================
//
// SkeletonTransformNameMap.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletonTransformNameMap.h"

#include "clientSkeletalAnimation/Skeleton.h"

// ======================================================================

SkeletonTransformNameMap::SkeletonTransformNameMap(const Skeleton *skeleton):	
	TransformNameMap(), //lint !e1769 // (Info -- Member or base class 'TransformNameMap' has no constructor) // that's okay, it might at some point
	m_skeleton(skeleton)
{
	NOT_NULL(skeleton);
}

// ----------------------------------------------------------------------

SkeletonTransformNameMap::~SkeletonTransformNameMap()
{
	m_skeleton = 0;
}

// ----------------------------------------------------------------------

void SkeletonTransformNameMap::findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const
{
	m_skeleton->findTransformIndex(name, transformIndex, found);
}

// ----------------------------------------------------------------------

int SkeletonTransformNameMap::getTransformIndex(CrcString const &name) const
{
	return m_skeleton->getTransformIndex(name);
}

// ----------------------------------------------------------------------

int SkeletonTransformNameMap::getTransformCount() const
{
	return m_skeleton->getTransformCount();
}

// ----------------------------------------------------------------------

CrcString const &SkeletonTransformNameMap::getTransformName(int index) const
{
	return m_skeleton->getTransformName(index);
}

// ======================================================================
