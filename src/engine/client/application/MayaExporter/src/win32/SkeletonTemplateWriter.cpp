// ======================================================================
//
// SkeletonTemplateWriter.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "SkeletonTemplateWriter.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <vector>

// ======================================================================

namespace
{
	const Tag TAG_BPRO = TAG(B,P,R,O);
	const Tag TAG_BPTR = TAG(B,P,T,R);
	const Tag TAG_JROR = TAG(J,R,O,R);
	const Tag TAG_PRNT = TAG(P,R,N,T);
	const Tag TAG_RPRE = TAG(R,P,R,E);
	const Tag TAG_RPST = TAG(R,P,S,T);
	const Tag TAG_SKTM = TAG(S,K,T,M);
}

// ======================================================================
// lint supression
// ======================================================================

// error 754: info: local structure member unreferenced
//lint -esym(754, Joint::Joint)
//lint -esym(754, Joint::operator=)

// ======================================================================
// embedded member definition
// ======================================================================

struct SkeletonTemplateWriter::Joint
{
public:

	Joint(
		int                 parentIndex, 
		const char         *name, 
		const Quaternion   &preMultiplyRotation,
		const Quaternion   &postMultiplyRotation,
		const Vector       &bindPoseTranslation, 
		const Quaternion   &bindPoseRotation,
		JointRotationOrder  rotationOrder
		);
	~Joint();

public:

	int                 m_parentIndex;
	char               *m_name;
	Quaternion          m_preMultiplyRotation;
	Quaternion          m_postMultiplyRotation;
	Vector              m_bindPoseTranslation;
	Quaternion          m_bindPoseRotation;
	JointRotationOrder  m_rotationOrder;

private:

	Joint();
	Joint(const Joint&);
	Joint &operator =(const Joint&);

};

// ======================================================================

struct SkeletonTemplateWriter::JointContainer
{
public:

	typedef std::vector<Joint*>  Container;

public:

	Container  m_container;

};

// ======================================================================
// class SkeletonTemplateWriter::Joint
// ======================================================================

SkeletonTemplateWriter::Joint::Joint(
	int                 parentIndex, 
	const char         *name, 
	const Quaternion   &preMultiplyRotation,
	const Quaternion   &postMultiplyRotation,
	const Vector       &bindPoseTranslation, 
	const Quaternion   &bindPoseRotation,
	JointRotationOrder  rotationOrder
	)
:
	m_parentIndex(parentIndex),
	m_name(NON_NULL(DuplicateString(name))),
	m_preMultiplyRotation(preMultiplyRotation),
	m_postMultiplyRotation(postMultiplyRotation),
	m_bindPoseTranslation(bindPoseTranslation), 
	m_bindPoseRotation(bindPoseRotation),
	m_rotationOrder(rotationOrder)
{
}

// ----------------------------------------------------------------------

SkeletonTemplateWriter::Joint::~Joint()
{
	delete [] m_name;
}

// ======================================================================
// class SkeletonTemplateWriter

SkeletonTemplateWriter::SkeletonTemplateWriter()
:
	m_joints(0)
{
	m_joints = NON_NULL(new JointContainer());
	m_joints->m_container.reserve(INITIAL_JOINT_RESERVE);
}

// ----------------------------------------------------------------------

SkeletonTemplateWriter::~SkeletonTemplateWriter()
{
	NOT_NULL(m_joints);

	const size_t jointCount = m_joints->m_container.size();
	for (size_t i = 0; i < jointCount; ++i)
	{
		Joint *const deadJoint = m_joints->m_container[i];
		delete deadJoint;
	}
	delete m_joints;
}

// ----------------------------------------------------------------------
/**
 *
 * Add a joint to the skeleton segment.
 *
 * Parent joints must be added before their child joints.
 */

bool SkeletonTemplateWriter::addJoint(
	int                 parentIndex, 
	const char         *jointName, 
	const Quaternion   &preMultiplyRotation,
	const Quaternion   &postMultiplyRotation, 
	const Vector       &bindPoseTranslation, 
	const Quaternion   &bindPoseRotation,
	JointRotationOrder  jointRotationOrder,
	int                *newIndex
	)
{
	NOT_NULL(m_joints);

	//-- validate args
	if (!jointName || !*jointName)
	{
		REPORT_LOG_PRINT(true, ("null or zero-length joint name\n"));
		return false;
	}

	if (!newIndex)
	{
		REPORT_LOG_PRINT(true, ("null newIndex arg\n"));
		return false;
	}

	// joint with negative parent index = root joint
	if (parentIndex < -1)
		parentIndex = -1;
	if ((parentIndex >= 0) && (static_cast<size_t>(parentIndex) >= m_joints->m_container.size()))
	{
		REPORT_LOG_PRINT(true, ("parentIndex [%d] out of valid range [0..%u)", parentIndex, m_joints->m_container.size()));
		return false;
	}

	//-- create the joint
	Joint *const joint = new Joint(parentIndex, jointName, preMultiplyRotation, postMultiplyRotation, bindPoseTranslation, bindPoseRotation, jointRotationOrder);
	if (!joint) //lint !e774 // boolean always evaluates to false // not with our mem manager
	{
		REPORT_LOG_PRINT(true, ("failed to create new joint\n"));
		return false;
	}

	//-- add joint to container
	const size_t unsignedIndex = m_joints->m_container.size();
	m_joints->m_container.push_back(joint);

	//-- return new joint index
	*newIndex = static_cast<int>(unsignedIndex);

	return true; //lint !e429 // custodial pointer joint not freed or returned // okay, on list
}

// ----------------------------------------------------------------------

void SkeletonTemplateWriter::write(Iff *iff) const
{
	NOT_NULL(iff);
	NOT_NULL(m_joints);

	const size_t jointCount = m_joints->m_container.size();

	iff->insertForm(TAG_SKTM);
		iff->insertForm(TAG_0002);

			//-- write general info
			iff->insertChunk(TAG_INFO);
				iff->insertChunkData(static_cast<int32>(jointCount));
			iff->exitChunk(TAG_INFO);

			//-- write joint names
			iff->insertChunk(TAG_NAME);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkString(joint->m_name);
				}
			}
			iff->exitChunk(TAG_NAME);

			//-- write joint parent hierarchy info
			iff->insertChunk(TAG_PRNT);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkData(static_cast<int32>(joint->m_parentIndex));
				}
			}
			iff->exitChunk(TAG_PRNT);

			//-- write pre multiply rotations
			iff->insertChunk(TAG_RPRE);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkFloatQuaternion(joint->m_preMultiplyRotation);
				}
			}
			iff->exitChunk(TAG_RPRE);

			//-- write post multiply rotations
			iff->insertChunk(TAG_RPST);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkFloatQuaternion(joint->m_postMultiplyRotation);
				}
			}
			iff->exitChunk(TAG_RPST);

			//-- write bind-pose joint translation
			iff->insertChunk(TAG_BPTR);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkFloatVector(joint->m_bindPoseTranslation);
				}
			}
			iff->exitChunk(TAG_BPTR);

			//-- write bind pose rotation
			iff->insertChunk(TAG_BPRO);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkFloatQuaternion(joint->m_bindPoseRotation);
				}
			}
			iff->exitChunk(TAG_BPRO);

			//-- write bind-pose joint rotation order
			iff->insertChunk(TAG_JROR);
			{
				for (size_t i = 0; i < jointCount; ++i)
				{
					const Joint *const joint = m_joints->m_container[i];
					iff->insertChunkData(static_cast<uint32>(joint->m_rotationOrder));
				}
			}
			iff->exitChunk(TAG_JROR);

		iff->exitForm(TAG_0002);
	iff->exitForm(TAG_SKTM);
}

// ----------------------------------------------------------------------

int SkeletonTemplateWriter::getJointCount() const
{
	NOT_NULL(m_joints);
	return static_cast<int>(m_joints->m_container.size());
}

// ======================================================================
