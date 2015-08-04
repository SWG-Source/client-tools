// ======================================================================
//
// SkeletonTemplateWriter.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef SKELETON_TEMPLATE_WRITER_H
#define SKELETON_TEMPLATE_WRITER_H

// ======================================================================
// includes

#include "clientSkeletalAnimation/SkeletonTemplateDef.h"

// forward declarations

class Iff;
class Quaternion;
class Transform;
class Vector;

// ======================================================================

class SkeletonTemplateWriter
{
public:

	enum
	{
		INITIAL_JOINT_RESERVE = 64
	};

public:

	SkeletonTemplateWriter();
	~SkeletonTemplateWriter();

	bool addJoint(
		int                 parentIndex, 
		const char         *jointName, 
		const Quaternion   &preMultiplyRotation,
		const Quaternion   &postMultiplyRotation, 
		const Vector       &bindPoseTranslation, 
		const Quaternion   &bindPoseRotation,
		JointRotationOrder  jointRotationOrder,
		int                *newIndex
		);

	void write(Iff *iff) const;

	int  getJointCount() const;

private:

	struct Joint;
	struct JointContainer;

private:

	JointContainer *m_joints;

private:

	// disabled
	SkeletonTemplateWriter(const SkeletonTemplateWriter&);
	SkeletonTemplateWriter &operator =(const SkeletonTemplateWriter&);

};

// ======================================================================

#endif
