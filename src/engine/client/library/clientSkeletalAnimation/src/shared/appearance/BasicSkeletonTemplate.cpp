// ======================================================================
//
// BasicSkeletonTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"

#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "clientSkeletalAnimation/SkeletonTemplateTransformNameMap.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

const Tag TAG_BPMJ = TAG(B,P,M,J);
const Tag TAG_BPRO = TAG(B,P,R,O);
const Tag TAG_BPTR = TAG(B,P,T,R);
const Tag TAG_JROR = TAG(J,R,O,R);
const Tag TAG_PRNT = TAG(P,R,N,T);
const Tag TAG_RPRE = TAG(R,P,R,E);
const Tag TAG_RPST = TAG(R,P,S,T);
const Tag TAG_SKTM = TAG(S,K,T,M);

// ======================================================================

bool BasicSkeletonTemplate::ms_installed;

// ======================================================================

struct BasicSkeletonTemplate::ModelToJointInfo
{
public:

	explicit ModelToJointInfo(const AttachmentChain &attachmentChain);

public:

	AttachmentChain  m_attachmentChain;
	TransformVector  m_transforms;
	int              m_referenceCount;

private:
	// disabled
	ModelToJointInfo();
	ModelToJointInfo(const ModelToJointInfo&);             //lint -esym(754, ModelToJointInfo::ModelToJointInfo) // not referenced // defensive hiding 
	ModelToJointInfo &operator =(const ModelToJointInfo&); //lint -esym(754, ModelToJointInfo::operator=)        // not referenced // defensive hiding
};

// ======================================================================

namespace
{
	class ModelToJointInfoLessComparator
	{
	public:

		bool operator()(const BasicSkeletonTemplate::AttachmentChain *lhs, const BasicSkeletonTemplate::ModelToJointInfo *rhs) const
			{
				return *lhs < rhs->m_attachmentChain;
			}

		bool operator()(const BasicSkeletonTemplate::ModelToJointInfo *lhs, const BasicSkeletonTemplate::AttachmentChain *rhs) const
			{
				return lhs->m_attachmentChain < *rhs;
			}

	};
}

// ======================================================================
// struct BasicSkeletonTemplate::ModelToJointInfo
// ======================================================================

BasicSkeletonTemplate::ModelToJointInfo::ModelToJointInfo(const AttachmentChain &attachmentChain)
:	m_attachmentChain(attachmentChain),
	m_transforms(),
	m_referenceCount(0)
{
}

// ======================================================================
// class BasicSkeletonTemplate
// ======================================================================

void BasicSkeletonTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("BasicSkeletonTemplate already installed"));

	SkeletonTemplateList::registerCreateFunction(TAG_SKTM, createSkeletonTemplate);

	ms_installed = true;
	ExitChain::add(remove, "BasicSkeletonTemplate");
}

// ======================================================================

void BasicSkeletonTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("BasicSkeletonTemplate not installed"));

	SkeletonTemplateList::deregisterCreateFunction(TAG_SKTM);

	ms_installed = false;
}

// ----------------------------------------------------------------------

SkeletonTemplate *BasicSkeletonTemplate::createSkeletonTemplate(Iff &iff, CrcString const &name)
{
	return new BasicSkeletonTemplate(&iff, name);
}

// ======================================================================

BasicSkeletonTemplate::BasicSkeletonTemplate(Iff *iff, CrcString const &name) :	
	SkeletonTemplate(name),
	m_jointCount(0),
	m_jointNames(new PersistentCrcStringVector),
	m_jointNameIndexMap(new JointNameIndexMap),
	m_jointParentIndices(new IntVector),
	m_preMultiplyRotations(new QuaternionVector),
	m_postMultiplyRotations(new QuaternionVector),
	m_bindPoseTranslations(new VectorVector),
	m_bindPoseRotations(new QuaternionVector),
	m_jointRotationOrder(new JointRotationOrderVector),
	m_modelToJointInfo(new ModelToJointInfoVector),
	m_transformNameMap(0)
{
	DEBUG_FATAL(!ms_installed, ("BasicSkeletonTemplate not installed"));

	NOT_NULL(iff);
	
	iff->enterForm(TAG_SKTM);

		DEBUG_FATAL(!iff->isCurrentForm(), ("expecting version form"));
		switch(iff->getCurrentName())
		{
			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char formName[5];
					ConvertTagToString(iff->getCurrentName(), formName);
					FATAL(true, ("unknown SkeletalTemplate version [%s]", formName));
				}
		}

	iff->exitForm(TAG_SKTM);

	DEBUG_WARNING(DataLint::isEnabled() && (m_jointCount < 2), ("skeleton [%s] has fewer than 2 joints, invalid appearance extents will arise.", name.getString()));
}

// ----------------------------------------------------------------------

BasicSkeletonTemplate::~BasicSkeletonTemplate()
{
	delete m_transformNameMap;

	std::for_each(m_modelToJointInfo->begin(), m_modelToJointInfo->end(), PointerDeleter());
	delete m_modelToJointInfo;

	delete m_jointRotationOrder;
	delete m_bindPoseRotations;
	delete m_bindPoseTranslations;
	delete m_postMultiplyRotations;
	delete m_preMultiplyRotations;

	delete m_jointParentIndices;
	delete m_jointNameIndexMap;

	std::for_each(m_jointNames->begin(), m_jointNames->end(), PointerDeleter());
	delete m_jointNames;
}

// ----------------------------------------------------------------------

void BasicSkeletonTemplate::load_0001(Iff *iff)
{
#ifdef _DEBUG
	const int maxExpectedJointCount = 250;
#endif

	NOT_NULL(iff);

	iff->enterForm(TAG_0001);

		//-- get general info
		iff->enterChunk(TAG_INFO);
			m_jointCount = iff->read_int32();
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_jointCount, maxExpectedJointCount);
		iff->exitChunk(TAG_INFO);

		//-- get joint names
		iff->enterChunk(TAG_NAME);
		{
			// load joint names, build joint name lookup map
			m_jointNames->reserve(static_cast<size_t>(m_jointCount));

			for (int i = 0; i < m_jointCount; ++i)
			{
				char jointName[MAX_PATH];
				iff->read_string(jointName, MAX_PATH-1);

				PersistentCrcString *const crcJointName = new PersistentCrcString(jointName, true);
				m_jointNames->push_back(crcJointName);

				IGNORE_RETURN(m_jointNameIndexMap->insert(JointNameIndexMap::value_type(crcJointName, i)));
			} //lint !e429 // custodial pointer not freed or returned
		}
		iff->exitChunk(TAG_NAME);

		//-- get joint parent information
		iff->enterChunk(TAG_PRNT);
		{
			m_jointParentIndices->reserve(static_cast<size_t>(m_jointCount));
			for (int i = 0; i < m_jointCount; ++i)
				m_jointParentIndices->push_back(iff->read_int32());
		}
		iff->exitChunk(TAG_PRNT);

		//-- get pre-multiply rotations
		iff->enterChunk(TAG_RPRE);
		{
			// -TRF- expensive constructor
			m_preMultiplyRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_preMultiplyRotations)[0]);
		}
		iff->exitChunk(TAG_RPRE);

		//-- get post-multiply rotations
		iff->enterChunk(TAG_RPST);
		{
			// -TRF- expensive constructor
			m_postMultiplyRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_postMultiplyRotations)[0]);
		}
		iff->exitChunk(TAG_RPST);

		//-- get bind-pose joint translations
		iff->enterChunk(TAG_BPTR);
		{
			m_bindPoseTranslations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatVector(m_jointCount, &(*m_bindPoseTranslations)[0]);
		}
		iff->exitChunk(TAG_BPTR);

		//-- get bind pose rotations
		iff->enterChunk(TAG_BPRO);
		{
			m_bindPoseRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_bindPoseRotations)[0]);
		}
		iff->exitChunk(TAG_BPRO);

		//-- get bind-pose model-to-joint transforms
		iff->enterChunk(TAG_BPMJ);
		{
			// we ignore these now, build on fly as necessary
			for (int i = 0; i < m_jointCount; ++i)
				IGNORE_RETURN(iff->read_floatTransform());
		}
		iff->exitChunk(TAG_BPMJ);

		//-- get bind-pose rotation order
		iff->enterChunk(TAG_JROR);
		{
			m_jointRotationOrder->reserve(static_cast<size_t>(m_jointCount));
			for (int i = 0; i < m_jointCount; ++i)
				m_jointRotationOrder->push_back(static_cast<JointRotationOrder>(iff->read_uint32()));
		}
		iff->exitChunk(TAG_JROR);

	iff->exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void BasicSkeletonTemplate::load_0002(Iff *iff)
{
#ifdef _DEBUG
	const int maxExpectedJointCount = 250;
#endif

	NOT_NULL(iff);

	iff->enterForm(TAG_0002);

		//-- get general info
		iff->enterChunk(TAG_INFO);
			m_jointCount = iff->read_int32();
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_jointCount, maxExpectedJointCount);
		iff->exitChunk(TAG_INFO);

		//-- get joint names
		iff->enterChunk(TAG_NAME);
		{
			// load joint names, build joint name lookup map
			m_jointNames->reserve(static_cast<size_t>(m_jointCount));

			for (int i = 0; i < m_jointCount; ++i)
			{
				char jointName[MAX_PATH];
				iff->read_string(jointName, MAX_PATH-1);

				PersistentCrcString *const crcJointName = new PersistentCrcString(jointName, true);
				m_jointNames->push_back(crcJointName);

				IGNORE_RETURN(m_jointNameIndexMap->insert(JointNameIndexMap::value_type(crcJointName, i)));
			} //lint !e429 // custodial pointer not freed or returned
		}
		iff->exitChunk(TAG_NAME);

		//-- get joint parent information
		iff->enterChunk(TAG_PRNT);
		{
			m_jointParentIndices->reserve(static_cast<size_t>(m_jointCount));
			for (int i = 0; i < m_jointCount; ++i)
				m_jointParentIndices->push_back(iff->read_int32());
		}
		iff->exitChunk(TAG_PRNT);

		//-- get pre-multiply rotations
		iff->enterChunk(TAG_RPRE);
		{
			// -TRF- expensive constructor
			m_preMultiplyRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_preMultiplyRotations)[0]);
		}
		iff->exitChunk(TAG_RPRE);

		//-- get post-multiply rotations
		iff->enterChunk(TAG_RPST);
		{
			// -TRF- expensive constructor
			m_postMultiplyRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_postMultiplyRotations)[0]);
		}
		iff->exitChunk(TAG_RPST);

		//-- get bind-pose joint translations
		iff->enterChunk(TAG_BPTR);
		{
			m_bindPoseTranslations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatVector(m_jointCount, &(*m_bindPoseTranslations)[0]);
		}
		iff->exitChunk(TAG_BPTR);

		//-- get bind pose rotations
		iff->enterChunk(TAG_BPRO);
		{
			m_bindPoseRotations->resize(static_cast<size_t>(m_jointCount));
			iff->read_floatQuaternion(m_jointCount, &(*m_bindPoseRotations)[0]);
		}
		iff->exitChunk(TAG_BPRO);

		//-- get bind-pose rotation order
		iff->enterChunk(TAG_JROR);
		{
			m_jointRotationOrder->reserve(static_cast<size_t>(m_jointCount));
			for (int i = 0; i < m_jointCount; ++i)
				m_jointRotationOrder->push_back(static_cast<JointRotationOrder>(iff->read_uint32()));
		}
		iff->exitChunk(TAG_JROR);

	iff->exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

const Quaternion *BasicSkeletonTemplate::getPreMultiplyRotations() const
{
	return &(*m_preMultiplyRotations)[0];
}

// ------------------------------------------------------------------

const Quaternion *BasicSkeletonTemplate::getPostMultiplyRotations() const
{
	return &(*m_postMultiplyRotations)[0];
}

// ------------------------------------------------------------------

const Vector *BasicSkeletonTemplate::getBindPoseTranslations() const
{
	return &(*m_bindPoseTranslations)[0];
}

// ------------------------------------------------------------------

const Quaternion *BasicSkeletonTemplate::getBindPoseRotations() const
{
	return &(*m_bindPoseRotations)[0];
}

// ----------------------------------------------------------------------

const JointRotationOrder *BasicSkeletonTemplate::getJointRotationOrderArray() const
{
	return &(*m_jointRotationOrder)[0];
}

// ----------------------------------------------------------------------

const TransformNameMap &BasicSkeletonTemplate::getTransformNameMap() const
{
	if (!m_transformNameMap)
		m_transformNameMap = new SkeletonTemplateTransformNameMap(*this);

	return *m_transformNameMap;
}

// ----------------------------------------------------------------------

const int *BasicSkeletonTemplate::getJointParentIndexArray() const
{
	return &(*m_jointParentIndices)[0];
}

// ----------------------------------------------------------------------

int BasicSkeletonTemplate::getJointIndex(CrcString const &jointName) const
{
	bool  foundIt    = false;
	int   jointIndex = 0;

	findJointIndex(jointName, &jointIndex, &foundIt);
	DEBUG_FATAL(!foundIt, ("specified joint name not found"));

	return jointIndex;
}

// ----------------------------------------------------------------------

CrcString const &BasicSkeletonTemplate::getJointName(int jointIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, jointIndex, m_jointCount);
	NOT_NULL(m_jointNames);

	return *NON_NULL((*m_jointNames)[static_cast<size_t>(jointIndex)]);
}

// ----------------------------------------------------------------------

void BasicSkeletonTemplate::findJointIndex(CrcString const &jointName, int *jointIndex, bool *found) const
{
	NOT_NULL(found);
	NOT_NULL(m_jointNameIndexMap);

	JointNameIndexMap::const_iterator it = m_jointNameIndexMap->find(&jointName);

	const bool foundIt = (it != m_jointNameIndexMap->end());
	if (foundIt)
	{
		NOT_NULL(jointIndex);
		*jointIndex = (*it).second;
	}

	*found = foundIt;
}

// ----------------------------------------------------------------------

int BasicSkeletonTemplate::getDetailCount() const
{
	//-- indicate there is only one detail level associated with this
	//   SkeletonTemplate.  It is this instance itself.  The DetailSkeletonTemplate
	//   class has multiple instances.
	return 1;
}

// ----------------------------------------------------------------------

const BasicSkeletonTemplate *BasicSkeletonTemplate::fetchBasicSkeletonTemplate(int detailIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, 1);
	UNREF(detailIndex);

	//-- increment reference count
	fetch();

	//-- return this instance to caller
	return this;
}

// ----------------------------------------------------------------------
/**
 * Create a new Skeleton with this BasicSkeletonTemplate's skeleton as the
 * single initial skeleton segment.
 */

Skeleton *BasicSkeletonTemplate::createSkeleton(TransformAnimationResolver &animationResolver) const
{
	return new Skeleton(*this, animationResolver);
}

// ----------------------------------------------------------------------

int BasicSkeletonTemplate::pointerJointCountAccumulator(int currentCount, const BasicSkeletonTemplate *skeletonTemplate)
{
	if (!skeletonTemplate)
		return currentCount;
	else
		return currentCount + skeletonTemplate->m_jointCount;
}

// ----------------------------------------------------------------------

int BasicSkeletonTemplate::referenceJointCountAccumulator(int currentCount, const BasicSkeletonTemplate &skeletonTemplate)
{
	return currentCount + skeletonTemplate.m_jointCount;
}

// ----------------------------------------------------------------------

void BasicSkeletonTemplate::buildModelToJointTransforms(const Transform &inheritedModelToRootTransform, TransformVector &modelToJointTransforms) const
{
	modelToJointTransforms.resize(static_cast<size_t>(m_jointCount));

	Transform  localToParentTransform(Transform::IF_none);
	Transform  parentToLocalTransform(Transform::IF_none);
	for (size_t i = 0; i < static_cast<size_t>(m_jointCount); ++i)
	{
		//-- set rotation
		const Quaternion  rotation = (*m_postMultiplyRotations)[i] * (*m_bindPoseRotations)[i] * (*m_preMultiplyRotations)[i];
		rotation.getTransformPreserveTranslation(&localToParentTransform);

		//-- set translation
		localToParentTransform.setPosition_p((*m_bindPoseTranslations)[i]);

		//-- parentToLocal is inverse of localToParent
		parentToLocalTransform.invert(localToParentTransform);

		//-- build modelToLocal from  parentToLocal * modelToParent
		Transform &modelToJointTransform = modelToJointTransforms[i];
		
		const int parentIndex = (*m_jointParentIndices)[i];
		if (parentIndex < 0)
		{
			// this is a root node of the skeleton segment
			modelToJointTransform.multiply(parentToLocalTransform, inheritedModelToRootTransform);
		}
		else
		{
			DEBUG_FATAL(static_cast<size_t>(parentIndex) >= i, ("invalid, parent index = %d, this index = %u", parentIndex, i));
			modelToJointTransform.multiply(parentToLocalTransform, modelToJointTransforms[static_cast<size_t>(parentIndex)]);
		}
	}
}

// ----------------------------------------------------------------------

const BasicSkeletonTemplate::ModelToJointInfo *BasicSkeletonTemplate::fetchBindPoseModelToJointTransforms(const AttachmentChain &attachmentChain, const Transform &parentModelToJointTransform, const TransformVector **bindPoseModelToJointTransforms) const
{
	NOT_NULL(bindPoseModelToJointTransforms);

	ModelToJointInfo *modelToJointInfo = 0;

	ModelToJointInfoVector::iterator it = std::lower_bound(m_modelToJointInfo->begin(), m_modelToJointInfo->end(), &attachmentChain, ModelToJointInfoLessComparator());
	if (it != m_modelToJointInfo->end() && !ModelToJointInfoLessComparator()(&attachmentChain, *it))
	{
		// we already created data for this attachment chain
		modelToJointInfo = *it;
	}
	else
	{
		// didn't find the attachment chain
		//-- create the modelToJoint transform data for this attachment chain
		modelToJointInfo = new ModelToJointInfo(attachmentChain);
		buildModelToJointTransforms(parentModelToJointTransform, modelToJointInfo->m_transforms);

		//-- add to modelToJointInfo vector
		IGNORE_RETURN(m_modelToJointInfo->insert(it, modelToJointInfo));
	}

	NOT_NULL(modelToJointInfo);

	*bindPoseModelToJointTransforms = &modelToJointInfo->m_transforms;
	++(modelToJointInfo->m_referenceCount);
	return modelToJointInfo;
}

// ----------------------------------------------------------------------

void BasicSkeletonTemplate::releaseBindPoseModelToJointTransforms(const ModelToJointInfo *modelToJointInfo) const
{
	NOT_NULL(modelToJointInfo);

	//-- get a non-const pointer
	ModelToJointInfo *const mtji = const_cast<ModelToJointInfo*>(modelToJointInfo);

	//-- decrement ref count
	--(mtji->m_referenceCount);
	if (mtji->m_referenceCount < 1)
	{
		//-- no longer used, delete this data
		ModelToJointInfoVector::iterator it = std::find(m_modelToJointInfo->begin(), m_modelToJointInfo->end(), mtji);
		DEBUG_FATAL(it == m_modelToJointInfo->end(), ("failed to find modelToJointInfo 0x%08xd", mtji));
		DEBUG_FATAL(mtji->m_referenceCount < 0, ("negative reference count [%d]", mtji->m_referenceCount));

		IGNORE_RETURN(m_modelToJointInfo->erase(it));
		delete mtji;
	}
}

// ======================================================================
