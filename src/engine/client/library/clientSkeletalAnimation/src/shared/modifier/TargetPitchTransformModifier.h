// ======================================================================
//
// TargetPitchTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TargetPitchTransformModifier_H
#define INCLUDED_TargetPitchTransformModifier_H

// ======================================================================

class Camera;
class MemoryBlockManager;

#include "clientSkeletalAnimation/TransformModifier.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class TargetPitchTransformModifier: public TransformModifier
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef Camera const *(*GetCameraFunction)();

public:

	static void                          install();
	static void                          setGetCameraFunction(GetCameraFunction function);

public:

	TargetPitchTransformModifier();

	virtual bool modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o);

private:

	static void  remove();

private:

	// Disabled.
	TargetPitchTransformModifier(TargetPitchTransformModifier const&);
	TargetPitchTransformModifier &operator =(TargetPitchTransformModifier const&);

};

// ======================================================================

#endif
