// ======================================================================
//
// LookAtTransformModifier.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_LookAtTransformModifier_H
#define INCLUDED_LookAtTransformModifier_H

// ======================================================================

class Camera;
class MemoryBlockManager;
class SkeletalAppearance2;

#include "clientSkeletalAnimation/TransformModifier.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class LookAtTransformModifier: public TransformModifier
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	typedef Camera const *(*GetCameraFunction)();
	typedef bool (*AllowLookAtTargetFunction)(SkeletalAppearance2 const &lookerAppearance);

public:

	static void                     install();
	static void                     setGetCameraFunction(GetCameraFunction function);
	static void                     setAllowLookAtTargetFunction(AllowLookAtTargetFunction function);

public:

	LookAtTransformModifier();

	virtual bool modifyTransform(float elapsedTime, Skeleton const &skeleton, CrcString const &transformName, Transform const &transform_p2o, Transform const &transform_l2p, Transform &transform_l2o);

	void                     setMaster(LookAtTransformModifier *master);
	LookAtTransformModifier *getMaster();

private:

	static void  remove();

private:

	float                    getSlaveYawRotation() const;
	float                    calculateTotalYawCorrection(float elapsedTime, Skeleton const &skeleton, Transform const &transform_l2o) const;

	// Disabled.
	LookAtTransformModifier(LookAtTransformModifier const&);
	LookAtTransformModifier &operator =(LookAtTransformModifier const&);

private:

	LookAtTransformModifier *m_master;
	int                      m_evaluateFrameNumber;
	float                    m_totalYawCorrection;

};

// ======================================================================

inline void LookAtTransformModifier::setMaster(LookAtTransformModifier *master)
{
	DEBUG_FATAL(master == this, ("master == this, logic error."));
	m_master = master;
}

// ----------------------------------------------------------------------

inline LookAtTransformModifier *LookAtTransformModifier::getMaster()
{
	return m_master;
}

// ======================================================================

#endif
