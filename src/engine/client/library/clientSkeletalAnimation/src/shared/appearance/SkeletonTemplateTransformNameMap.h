// ======================================================================
//
// SkeletonTemplateTransformNameMap.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDE_SkeletonTemplateTransformNameMap_H
#define INCLUDE_SkeletonTemplateTransformNameMap_H

// ======================================================================

#include "clientSkeletalAnimation/TransformNameMap.h"
class BasicSkeletonTemplate;

// ======================================================================

class SkeletonTemplateTransformNameMap: public TransformNameMap
{
public:

	explicit SkeletonTemplateTransformNameMap(const BasicSkeletonTemplate &skeletonTemplate);
	virtual ~SkeletonTemplateTransformNameMap();

	virtual void                  findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const;
	virtual int                   getTransformIndex(CrcString const &name) const;

	virtual int                   getTransformCount() const;
	virtual CrcString const      &getTransformName(int index) const;

private:

	// disabled
	SkeletonTemplateTransformNameMap();
	SkeletonTemplateTransformNameMap(const SkeletonTemplateTransformNameMap&);
	SkeletonTemplateTransformNameMap &operator =(const SkeletonTemplateTransformNameMap&);

private:

	const BasicSkeletonTemplate &m_skeletonTemplate;

};

// ======================================================================

#endif
