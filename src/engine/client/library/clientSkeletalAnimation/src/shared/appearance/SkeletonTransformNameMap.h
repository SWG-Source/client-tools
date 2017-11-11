// ======================================================================
//
// SkeletonTransformNameMap.h
// Copyright 2001 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletonTransformNameMap_H
#define INCLUDED_SkeletonTransformNameMap_H

// ======================================================================

#include "clientSkeletalAnimation/TransformNameMap.h"

class Skeleton;

// ======================================================================

class SkeletonTransformNameMap: public TransformNameMap
{
public:

	explicit SkeletonTransformNameMap(const Skeleton *skeleton);
	virtual  ~SkeletonTransformNameMap();

	virtual void                  findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const;
	virtual int                   getTransformIndex(CrcString const &name) const;

	virtual int                   getTransformCount() const;
	virtual CrcString const      &getTransformName(int index) const;

private:

	const Skeleton *m_skeleton;

private:

	// disabled
	SkeletonTransformNameMap();
	SkeletonTransformNameMap(const SkeletonTransformNameMap&);
	SkeletonTransformNameMap &operator =(const SkeletonTransformNameMap&);

};

// ======================================================================

#endif
