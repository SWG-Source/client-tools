// ======================================================================
//
// Skeleton.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/Skeleton.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletonTransformNameMap.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientSkeletalAnimation/TransformModifier.h"
#include "clientSkeletalAnimation/PoseModelTransform.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <vector>

// ======================================================================

namespace SkeletonNamespace
{
	typedef std::vector<std::pair<std::string, std::string> >  AttachmentChain;
	typedef std::vector<AttachmentChain>                       AttachmentChainVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if PRODUCTION == 0
	void  reportStatistics();
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

#if PRODUCTION == 0
	bool  s_reportStatistics;

	int   s_calculateJointToRootTransformsCallCount;
	int   s_calculateBindPoseModelToRootTransformsCallCount;
	int   s_calculateExtentCallCount;

	int   s_calculateJointToRootTransformsEvalCount;
	int   s_calculateBindPoseModelToRootTransformsEvalCount;
	int   s_calculateExtentEvalCount;

#endif

}

using namespace SkeletonNamespace;

// ======================================================================

#if PRODUCTION == 0

void SkeletonNamespace::reportStatistics()
{
	//-- Report.
	REPORT_PRINT(true, ("Skeleton:(called/evaluated): extent=[%d/%d],jointToRoot=[%d/%d],bindPoseModelToRoot=[%d/%d]\n", 
		s_calculateExtentCallCount, s_calculateExtentEvalCount, 
		s_calculateJointToRootTransformsCallCount, s_calculateJointToRootTransformsEvalCount, 
		s_calculateBindPoseModelToRootTransformsCallCount, s_calculateBindPoseModelToRootTransformsEvalCount));

	//-- Print.
	s_calculateExtentCallCount                        = 0;
	s_calculateJointToRootTransformsCallCount         = 0;
	s_calculateBindPoseModelToRootTransformsCallCount = 0;

	s_calculateExtentEvalCount                        = 0;
	s_calculateJointToRootTransformsEvalCount         = 0;
	s_calculateBindPoseModelToRootTransformsEvalCount = 0;
}

#endif

// ======================================================================

class Skeleton::Hardpoint
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	Hardpoint(CrcString const &hardpointName, const Transform &localToParent, int parentLocalTransformIndex);

	CrcString const &getName() const;
	Transform const &getLocalToParent() const;
	int              getParentLocalTransformIndex() const;

private:

	PersistentCrcString  m_name;
	Transform            m_localToParent;
	int                  m_parentLocalTransformIndex;

private:
	// disabled
	Hardpoint();
	Hardpoint(Hardpoint const & rhs);              //lint -esym(754, Hardpoint::Hardpoint) // not referenced // correct: defensive hiding
	Hardpoint & operator=(Hardpoint const & rhs);
};

// ======================================================================

class Skeleton::LocalShaderPrimitive: public ShaderPrimitive
{
public:

	LocalShaderPrimitive(const SkeletalAppearance2 &appearance, const Skeleton &skeleton);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float elapsedTime);
	virtual const Vector        getPosition_w() const;
	virtual const StaticShader &prepareToView() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

private:

	const SkeletalAppearance2 &m_appearance;
	const Skeleton            &m_skeleton;
	StaticShader              *m_shader;

private:
	// disabled
	LocalShaderPrimitive();

	//lint -esym(754, LocalShaderPrimitive::operator=)
	//lint -esym(1714, LocalShaderPrimitive::operator=)
	LocalShaderPrimitive &operator =(const LocalShaderPrimitive&); 
};

// ======================================================================

class Skeleton::Segment
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static int transformCountAccumulate(int sumSoFar, const Segment *segment);

public:

	Segment(const BasicSkeletonTemplate &skeletonTemplate, Segment *parentSegment, int parentLocalTransformIndex);
	~Segment();

	void                         findLocalTransformIndex(CrcString const &name, int &localTransformIndex, bool &foundIt) const;
	void                         findParentIndex(int childSegmentLocalIndex, const Segment *&parentSegment, int &parentSegmentLocalIndex, bool &foundParentIndex) const;
	CrcString const             &getTransformName(int localTransformIndex) const;

	void                         setupBindPoseModelToJointTransforms(const AttachmentChain &attachmentChain, const Transform &parentModelToJointTransform);

	void                         addHardpoint(CrcString const &hardpointName, int localParentIndex, const Transform &hardpointToParent);
	void                         removeAllHardpoints();

	void                         calculateHardpointToRootTransforms(int firstSegmentTransformIndex, Transform *jointToRootTransforms) const;

	void                         drawHardpointsNow(const Transform &skeletonToWorld, const Vector &scale, int firstSegmentTransformIndex, const Transform *jointToRootTransforms) const;

	const BasicSkeletonTemplate &getSkeletonTemplate() const;

	bool                         isAttached() const;

	Segment                     &getParentSegment();
	int                          getParentLocalTransformIndex() const;

	//-- -TRF- these probably can go away if we move some of the Skeleton functionality into Segment member functions

	int                          getFirstTransformIndex() const;
	int                          getLastTransformIndex() const;

	void                         setFirstTransformIndex(int index);
	void                         setLastTransformIndex(int index);

	int                          getParentGlobalTransformIndex() const;
	void                         setParentGlobalTransformIndex(int index);

	const TransformVector       &getBindPoseModelToJointTransforms() const;

	const BasicSkeletonTemplate::ModelToJointInfo *getModelToJointInfo() const;

	void                         buildTransformResolverJointIndexMap(const TransformAnimationResolver &resolver);
	const std::vector<int>      &getTransformResolverJointIndexMap() const;

private:

	typedef std::vector<Hardpoint*>                                 HardpointVector;
	typedef std::map<CrcString const*, int, LessPointerComparator>  HardpointNameMap;

private:

	// disabled
	Segment();
	Segment(const Segment&);             //lint -esym(754, Segment::Segment)   // not referenced // defensive hiding
	Segment &operator =(const Segment&); //lint -esym(754, Segment::operator=) // not referenced // defensive hiding

private:

	const BasicSkeletonTemplate              &m_skeletonTemplate;

	Segment                                  *m_parentSegment;
	int                                       m_parentLocalTransformIndex;

	int                                       m_firstTransformIndex;
	int                                       m_lastTransformIndex;

	int                                       m_parentTransformIndex;

	const TransformVector                    *m_bindPoseModelToJointTransforms;	
	const BasicSkeletonTemplate::ModelToJointInfo *m_modelToJointInfo;

	HardpointVector                          *m_hardpoints;
	HardpointNameMap                         *m_hardpointNameToLocalIndexMap;

	std::vector<int>                          m_transformResolverJointIndexMap;

};

// ======================================================================
// class Skeleton::Hardpoint
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(Skeleton::Hardpoint, true, 0, 0, 0);

// ======================================================================

Skeleton::Hardpoint::Hardpoint(CrcString const &hardpointName, const Transform &localToParent, int parentLocalTransformIndex):
	m_name(hardpointName),
	m_localToParent(localToParent),
	m_parentLocalTransformIndex(parentLocalTransformIndex)
{
}

// ----------------------------------------------------------------------

inline CrcString const &Skeleton::Hardpoint::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline const Transform &Skeleton::Hardpoint::getLocalToParent() const
{
	return m_localToParent;
}

// ----------------------------------------------------------------------

inline int Skeleton::Hardpoint::getParentLocalTransformIndex() const
{
	return m_parentLocalTransformIndex;
}

// ======================================================================
// class Skeleton::LocalShaderPrimitive
// ======================================================================

Skeleton::LocalShaderPrimitive::LocalShaderPrimitive(const SkeletalAppearance2 &appearance, const Skeleton &skeleton)
:	ShaderPrimitive(),
	m_appearance(appearance),
	m_skeleton(skeleton),
	m_shader(safe_cast<StaticShader*>(ShaderTemplateList::get3dVertexColorStaticShader().getShaderTemplate().fetchModifiableShader()))
{
	m_shader->setPhaseTag(TAG(L,A,S,T));
}

// ----------------------------------------------------------------------

Skeleton::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	m_shader->release();
	m_shader = 0;
}

// ----------------------------------------------------------------------

float Skeleton::LocalShaderPrimitive::alter(real elapsedTime)
{
	//m_shader->alter(elapsedTime);
	UNREF(elapsedTime);
	return AlterResult::cms_keepNoAlter;
}

// ----------------------------------------------------------------------

const StaticShader &Skeleton::LocalShaderPrimitive::prepareToView() const
{
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

const Vector Skeleton::LocalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w().getPosition_p();
}
	
// ----------------------------------------------------------------------

float Skeleton::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_appearance.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int Skeleton::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

// ----------------------------------------------------------------------

void Skeleton::LocalShaderPrimitive::prepareToDraw() const
{
	Graphics::setObjectToWorldTransformAndScale(m_appearance.getTransform_w(), Vector::xyz111);
}

// ----------------------------------------------------------------------

void Skeleton::LocalShaderPrimitive::draw() const
{
	m_skeleton.drawNow(m_appearance.getTransform_w(), Vector::xyz111);
}

// ======================================================================
// class Skeleton::Segment
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(Skeleton::Segment, true, 0, 0, 0);

// ======================================================================

inline int Skeleton::Segment::transformCountAccumulate(int sumSoFar, const Segment *segment)
{
	NOT_NULL(segment);

	const int jointCount     = segment->m_skeletonTemplate.getJointCount();
	const int hardpointCount = (segment->m_hardpoints ? static_cast<int>(segment->m_hardpoints->size()) : 0);

	return sumSoFar + jointCount + hardpointCount;
}

// ======================================================================

Skeleton::Segment::Segment(const BasicSkeletonTemplate &skeletonTemplate, Segment *parentSegment, int parentLocalTransformIndex)
:	m_skeletonTemplate(skeletonTemplate),
	m_parentSegment(parentSegment),
	m_parentLocalTransformIndex(parentLocalTransformIndex),
	m_firstTransformIndex(-1),
	m_lastTransformIndex(-1),
	m_parentTransformIndex(-1),
	m_bindPoseModelToJointTransforms(0),
	m_modelToJointInfo(0),
	m_hardpoints(0),
	m_hardpointNameToLocalIndexMap(0),
	m_transformResolverJointIndexMap(skeletonTemplate.getJointCount(), -1)
{
	DEBUG_FATAL(!parentSegment && (parentLocalTransformIndex >= 0), ("looks like parent transform index set [%d] with NULL parent segment", parentLocalTransformIndex));

	//-- Fetch local reference.
	skeletonTemplate.fetch();
}

// ----------------------------------------------------------------------

Skeleton::Segment::~Segment()
{
	delete m_hardpointNameToLocalIndexMap;

	if (m_hardpoints)
	{
		std::for_each(m_hardpoints->begin(), m_hardpoints->end(), PointerDeleter());
		delete m_hardpoints;
	}

	m_bindPoseModelToJointTransforms = 0;

	if (m_modelToJointInfo)
	{
		m_skeletonTemplate.releaseBindPoseModelToJointTransforms(m_modelToJointInfo);
		m_modelToJointInfo = 0;
	}

	m_skeletonTemplate.release();

	m_parentSegment = 0;
}

// ----------------------------------------------------------------------

void Skeleton::Segment::findLocalTransformIndex(CrcString const &name, int &localTransformIndex, bool &foundIt) const
{
	foundIt = false;

	//-- check if name is a joint
	m_skeletonTemplate.findJointIndex(name, &localTransformIndex, &foundIt);

	if (!foundIt && m_hardpointNameToLocalIndexMap)
	{
		//-- check if it is one of our hardpoints
		const HardpointNameMap::const_iterator findIt = m_hardpointNameToLocalIndexMap->find(&name);
		if (findIt != m_hardpointNameToLocalIndexMap->end())
		{
			foundIt             = true;
			localTransformIndex = findIt->second;
		}
	}
}

// ----------------------------------------------------------------------

void Skeleton::Segment::findParentIndex(int childSegmentLocalIndex, const Segment *&parentSegment, int &parentSegmentLocalIndex, bool &foundParentIndex) const
{
	DEBUG_FATAL(childSegmentLocalIndex < 0, ("bad childSegmentLocalIndex %d", childSegmentLocalIndex));

	foundParentIndex = false;

	if (childSegmentLocalIndex < m_skeletonTemplate.getJointCount())
	{
		// this is a joint
		const int localParentIndex = m_skeletonTemplate.getJointParentIndexArray()[childSegmentLocalIndex];
		if (localParentIndex >= 0)
		{
			//-- this joint has a local parent (i.e. its not the root)
			foundParentIndex        = true;
			parentSegment           = this;
			parentSegmentLocalIndex = localParentIndex;
			return;
		}
		else
		{
			//-- this joint is the root of the local segment.  If the segment is attached, the parent
			//   will be a transform in the parent segment.
			if (m_parentSegment)
			{
				foundParentIndex        = true;
				parentSegment           = m_parentSegment;
				parentSegmentLocalIndex = m_parentLocalTransformIndex;
			}
			else
			{
				//-- no parent
				return;
			}
		}
	}
	else
	{
		// this is a hardpoint
		if (!m_hardpoints)
			return;

		const int hardpointIndex = childSegmentLocalIndex - m_skeletonTemplate.getJointCount();
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, hardpointIndex, static_cast<int>(m_hardpoints->size()));

		foundParentIndex        = true;
		parentSegment           = this;
		parentSegmentLocalIndex = (*m_hardpoints)[static_cast<size_t>(hardpointIndex)]->getParentLocalTransformIndex();

		return;
	}
}

// ----------------------------------------------------------------------

CrcString const &Skeleton::Segment::getTransformName(int localTransformIndex) const
{
	const int jointCount = m_skeletonTemplate.getJointCount();

	//-- handle joints
	if (localTransformIndex < jointCount)
	{
		// transform is a joint
		return m_skeletonTemplate.getJointName(localTransformIndex);
	}

	// must be a hardpoint
	NOT_NULL(m_hardpoints);

	// transform is a hardpoint
	const int hardpointIndex = localTransformIndex - jointCount;
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, hardpointIndex, static_cast<int>(m_hardpoints->size()));

	const Hardpoint *const hardpoint = (*m_hardpoints)[static_cast<size_t>(hardpointIndex)];
	return hardpoint->getName();
}

// ----------------------------------------------------------------------

void Skeleton::Segment::setupBindPoseModelToJointTransforms(const AttachmentChain &attachmentChain, const Transform &parentModelToJointTransform)
{
	m_modelToJointInfo = m_skeletonTemplate.fetchBindPoseModelToJointTransforms(attachmentChain, parentModelToJointTransform, &m_bindPoseModelToJointTransforms);
}

// ----------------------------------------------------------------------

void Skeleton::Segment::addHardpoint(CrcString const &hardpointName, int localParentIndex, const Transform &hardpointToParent)
{
	if (!m_hardpoints || !m_hardpointNameToLocalIndexMap)
	{
		DEBUG_FATAL(m_hardpoints, ("expecting null m_hardpoints"));
		m_hardpoints = new HardpointVector();

		DEBUG_FATAL(m_hardpointNameToLocalIndexMap, ("expecting null m_hardpointNameToLocalIndexMap"));
		m_hardpointNameToLocalIndexMap = new HardpointNameMap();
	}

	//-- find insertion point for hardpoint name
	HardpointNameMap::iterator itLowerBound = m_hardpointNameToLocalIndexMap->lower_bound(&hardpointName);
	
	// check if hardpoint already exists
	if ((itLowerBound != m_hardpointNameToLocalIndexMap->end()) && !(m_hardpointNameToLocalIndexMap->key_comp()(&hardpointName, itLowerBound->first)))
	{
		// this hardpoint is in the list already
		DEBUG_WARNING(true, ("trying to add hardpoint [%s] to same skeleton segment multiple times", hardpointName.getString()));
		return; //lint !e527 // unreachable // not in release
	}

	//-- create the hardpoint
	const int hardpointLocalIndex = m_skeletonTemplate.getJointCount() + static_cast<int>(m_hardpoints->size());
	m_hardpoints->push_back(new Hardpoint(hardpointName, hardpointToParent, localParentIndex));

	//-- add to name map
	IGNORE_RETURN(m_hardpointNameToLocalIndexMap->insert(itLowerBound, HardpointNameMap::value_type(&(m_hardpoints->back()->getName()), hardpointLocalIndex)));
}

// ----------------------------------------------------------------------

void Skeleton::Segment::removeAllHardpoints()
{
	if (m_hardpoints)
	{
		std::for_each(m_hardpoints->begin(), m_hardpoints->end(), PointerDeleter());
		m_hardpoints->clear();
	}

	if (m_hardpointNameToLocalIndexMap)
		m_hardpointNameToLocalIndexMap->clear();
}

// ----------------------------------------------------------------------

void Skeleton::Segment::calculateHardpointToRootTransforms(int firstSegmentTransformIndex, Transform *jointToRootTransforms) const
{
	if (m_hardpoints)
	{
		const size_t baseHardpointIndex = static_cast<size_t>(firstSegmentTransformIndex + m_skeletonTemplate.getJointCount());

		size_t localHardpointIndex = 0;

		const HardpointVector::const_iterator itEnd = m_hardpoints->end();
		for (HardpointVector::const_iterator it = m_hardpoints->begin(); it != itEnd; ++it, ++localHardpointIndex)
		{
			NOT_NULL(*it);
			const Hardpoint &hardpoint = *(*it);

			const Transform &parentToRoot = jointToRootTransforms[firstSegmentTransformIndex + hardpoint.getParentLocalTransformIndex()];
			Transform &destTransform      = jointToRootTransforms[baseHardpointIndex + localHardpointIndex];

			destTransform.multiply(parentToRoot, hardpoint.getLocalToParent());
		}
	}
}

// ----------------------------------------------------------------------

void Skeleton::Segment::drawHardpointsNow(const Transform &skeletonToWorld, const Vector &scale, int firstSegmentTransformIndex, const Transform *jointToRootTransforms) const
{
	if (m_hardpoints)
	{
		Transform hardpointToWorld(Transform::IF_none);

		// global index starts at the end of the skeleton joints
		size_t globalTransformIndex = static_cast<size_t>(firstSegmentTransformIndex + m_skeletonTemplate.getJointCount());

		const HardpointVector::const_iterator itEnd = m_hardpoints->end();
		for (HardpointVector::const_iterator it = m_hardpoints->begin(); it != itEnd; ++it, ++globalTransformIndex)
		{

			// adjust hardpointToRoot for scale
			Transform        hardpointToRoot = jointToRootTransforms[globalTransformIndex];
			const Vector    &oldPosition     = hardpointToRoot.getPosition_p();

			hardpointToRoot.setPosition_p(Vector(scale.x * oldPosition.x, scale.y * oldPosition.y, scale.z * oldPosition.z));

			// build hardpoint to world transform
			hardpointToWorld.multiply(skeletonToWorld, hardpointToRoot);

			// render a frame
			Graphics::setObjectToWorldTransformAndScale(hardpointToWorld, Vector::xyz111);
			Graphics::drawFrame(0.12f);
		}
	}
}

// ----------------------------------------------------------------------

inline const BasicSkeletonTemplate &Skeleton::Segment::getSkeletonTemplate() const
{
	return m_skeletonTemplate;
}

// ----------------------------------------------------------------------

inline bool Skeleton::Segment::isAttached() const
{
	return (m_parentSegment != NULL);
}

// ----------------------------------------------------------------------

inline Skeleton::Segment &Skeleton::Segment::getParentSegment()
{
	NOT_NULL(m_parentSegment);

	return *m_parentSegment;
}

// ----------------------------------------------------------------------

inline int Skeleton::Segment::getParentLocalTransformIndex() const
{
	DEBUG_FATAL(!m_parentSegment, ("no local transform index if not attached"));

	return m_parentLocalTransformIndex;
}

// ----------------------------------------------------------------------

inline int Skeleton::Segment::getFirstTransformIndex() const
{
	return m_firstTransformIndex;
}

// ----------------------------------------------------------------------

inline int Skeleton::Segment::getLastTransformIndex() const
{
	return m_lastTransformIndex;
}

// ----------------------------------------------------------------------

inline void Skeleton::Segment::setFirstTransformIndex(int index)
{
	m_firstTransformIndex = index;
}

// ----------------------------------------------------------------------

inline void Skeleton::Segment::setLastTransformIndex(int index)
{
	m_lastTransformIndex = index;
}

// ----------------------------------------------------------------------

inline int Skeleton::Segment::getParentGlobalTransformIndex() const
{
	return m_parentTransformIndex;
}

// ----------------------------------------------------------------------

inline void Skeleton::Segment::setParentGlobalTransformIndex(int index)
{
	m_parentTransformIndex = index;
}

// ----------------------------------------------------------------------

inline const Skeleton::TransformVector &Skeleton::Segment::getBindPoseModelToJointTransforms() const
{
	NOT_NULL(m_bindPoseModelToJointTransforms);

	return *m_bindPoseModelToJointTransforms;
}

// ----------------------------------------------------------------------

inline const BasicSkeletonTemplate::ModelToJointInfo *Skeleton::Segment::getModelToJointInfo() const
{
	return m_modelToJointInfo;
}

// ----------------------------------------------------------------------

void Skeleton::Segment::buildTransformResolverJointIndexMap(const TransformAnimationResolver &resolver)
{
	const int jointCount = m_skeletonTemplate.getJointCount();
	for (int i = 0; i < jointCount; ++i)
	{
		//-- Retrieve the index for this skeleton template joint name from the animation resolver.
		m_transformResolverJointIndexMap[static_cast<std::vector<int>::size_type>(i)] = -1;
		const bool indexFound = resolver.findTransformIndex(m_skeletonTemplate.getJointName(i), m_transformResolverJointIndexMap[static_cast<std::vector<int>::size_type>(i)]);
		UNREF(indexFound);
	}
}

// ----------------------------------------------------------------------

inline const std::vector<int> &Skeleton::Segment::getTransformResolverJointIndexMap() const
{
	return m_transformResolverJointIndexMap;
}

// ======================================================================
// class Skeleton
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(Skeleton, true, 0, 0, 0);

// ======================================================================

void Skeleton::install()
{
	DEBUG_FATAL(s_installed, ("Skeleton already installed."));

	installMemoryBlockManager();

#if PRODUCTION == 0
	DebugFlags::registerFlag(s_reportStatistics, "ClientSkeletalAnimation/Skeleton", "reportStatistics", reportStatistics);
#endif

	// Child classes go on exit chain.
	Hardpoint::install();
	Segment::install();
	
	s_installed = true;
	ExitChain::add(remove, "Skeleton");
}

// ======================================================================

Skeleton::Skeleton(const BasicSkeletonTemplate &rootSkeletonTemplate, TransformAnimationResolver &animationResolver) :
	m_animationResolver(animationResolver),
	m_transformCount(rootSkeletonTemplate.getJointCount()),
	m_skeletonSegments(new SegmentVector()),
	m_jointToRootTransformBackingStore(0),
	m_jointToRootTransforms(0),
	m_bindPoseModelToRootTransformBackingStore(0),
	m_bindPoseModelToRootTransforms(0),
	m_frameLastJointToRootCalculate(-1),
	m_frameLastBindPoseModelToRootCalculate(-1),
	m_frameLastExtentCalculate(-1),
	m_transformNameMap(0),
	m_extent(new BoxExtent()),
	m_modifyingSkeleton(false),
	m_shaderPrimitive(0),
	m_scaleTransform(new Transform),
	m_scale(1.0f),
	m_transformModifierMap(0)
{
	//-- Setup the m_jointToRootTransform array.
	allocateTransformArrays(m_transformCount);

	//-- reserve container space
	m_skeletonSegments->reserve(1);

	//-- rebuild skeleton definition from skeleton segment container
	beginSkeletonModification();

		//-- add initial segment
		m_skeletonSegments->push_back(new Segment(rootSkeletonTemplate, 0, -1));

	endSkeletonModification();

	m_transformNameMap = new SkeletonTransformNameMap(this);

	//-- Initialize scale transform.
	m_scaleTransform->setToScale(Vector(m_scale, m_scale, m_scale));
} //lint !e429 // custodial pointer segment not freed or returned

// ----------------------------------------------------------------------

Skeleton::~Skeleton()
{
	delete m_transformModifierMap;

	delete m_scaleTransform;
	delete m_shaderPrimitive;

	delete m_extent;
	delete m_transformNameMap;

	delete [] m_bindPoseModelToRootTransformBackingStore;
	m_bindPoseModelToRootTransforms = 0;

	delete [] m_jointToRootTransformBackingStore;
	m_jointToRootTransforms = 0;



	std::for_each(m_skeletonSegments->begin(), m_skeletonSegments->end(), PointerDeleter());
	delete m_skeletonSegments;
}

// ----------------------------------------------------------------------

void Skeleton::beginSkeletonModification()
{
	DEBUG_FATAL(m_modifyingSkeleton, ("already modifying skeleton"));
	m_modifyingSkeleton = true;

	//-- Drop any transform modifiers since transform indices will change on skeleton modifications.
	if (m_transformModifierMap)
		m_transformModifierMap->clear();
}

// ----------------------------------------------------------------------

void Skeleton::endSkeletonModification()
{
	DEBUG_FATAL(!m_modifyingSkeleton, ("not modifying skeleton"));

	m_modifyingSkeleton = false;

	//-- rebuild skeleton global transform IDs.  this must happen if
	//   skeleton segments and/or hardpoints are added.
	rebuildLinkedSkeletonDefinition();
}

// ----------------------------------------------------------------------
/**
 * Add all the bones in BasicSkeletonTemplate to the skeleton, rooting the new bones at
 * the specified transform that already exists in the skeleton.
 */

int Skeleton::attachSkeletonSegment(const BasicSkeletonTemplate &skeletonTemplate, CrcString const &attachmentTransformName)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::attachSkeletonSegment");

	DEBUG_FATAL(!m_modifyingSkeleton, ("must call attachSkeletonSegment() between beginSkeletonModification()/endSkeletonModification() call"));

	int      parentLocalTransformIndex = -1;
	Segment *parentSegment             = 0;

	const bool attachesToParent = attachmentTransformName.getString() && *attachmentTransformName.getString();
	if (attachesToParent)
	{
		//-- find segment with given transform name (could be joint or hardpoint)
		bool foundIt = false;

		const SegmentVector::iterator itEnd = m_skeletonSegments->end();
		for (SegmentVector::iterator it = m_skeletonSegments->begin(); it != itEnd; ++it)
		{
			Segment &segment = *(*it);
			NOT_NULL(*it);

			// query segment for transform index
			segment.findLocalTransformIndex(attachmentTransformName, parentLocalTransformIndex, foundIt);
			if (foundIt)
			{
				// -TRF- we don't support attaching segments to hardpoints yet.  To do that, we need to
				//       handle anonymous bindPoseModelToJointTransform registration (i.e. per-skeleton
				//       bindPoseModelToJointTransform objects!)
				DEBUG_FATAL(parentLocalTransformIndex >= segment.getSkeletonTemplate().getJointCount(), ("attaching skeleton segments to hardpoints not yet supported"));

				parentSegment = &segment;
				break;
			}
		}

		DEBUG_WARNING(!foundIt, ("failed to find attachment parent transform [%s] for skeleton\n", attachmentTransformName.getString()));
		DEBUG_FATAL(parentLocalTransformIndex < 0, ("bad local transform index %d", parentLocalTransformIndex));
	}

	// -TRF- note there is a potential optimization here.  we know the new segment has a joint that
	//       groups all the bones for the new skeleton.  we also know this joint will pass through
	//       with no effect (i.e. is identity).  we could optimize this by having all the children
	//       of skeletonTemplate attach directly to attachmentTransformName rather than keeping the
	//       root of skeletonTemplate.  however, nothing else must attach to the root of skeletonTemplate
	//       since the root would not actually appear in the skeleton.

	//-- create new segment
	const int newSegmentIndex = static_cast<int>(m_skeletonSegments->size());
	m_skeletonSegments->push_back(new Segment(skeletonTemplate, parentSegment, parentLocalTransformIndex));

	return newSegmentIndex; //lint !e429 // custodial pointer segment not freed or returned
}

// ----------------------------------------------------------------------

void Skeleton::removeSkeletonSegment(int skeletonSegmentId)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, skeletonSegmentId, static_cast<int>(m_skeletonSegments->size()));
	DEBUG_FATAL(!m_modifyingSkeleton, ("must call removeSkeletonSegment() between beginSkeletonModification()/endSkeletonModification() call"));

	SegmentVector::iterator it = m_skeletonSegments->begin();
	std::advance(it, skeletonSegmentId);

	delete *it;
	IGNORE_RETURN(m_skeletonSegments->erase(it));
}

// ----------------------------------------------------------------------

void Skeleton::removeAllSegments()
{
	DEBUG_FATAL(!m_modifyingSkeleton, ("must call removeAllSegments() between beginSkeletonModification()/endSkeletonModification() call"));

	std::for_each(m_skeletonSegments->begin(), m_skeletonSegments->end(), PointerDeleter());
	m_skeletonSegments->clear();
}

// ----------------------------------------------------------------------

void Skeleton::findSegmentLocalTransformIndex(CrcString const &transformName, int &segmentIndex, int &segmentLocalTransformIndex, bool &foundIt) const
{
	foundIt = false;

	//-- scan for the transform name in all skeleton segments
	segmentIndex = 0;

	const SegmentVector::const_iterator itEnd = m_skeletonSegments->end();
	for (SegmentVector::const_iterator it = m_skeletonSegments->begin(); it != itEnd; ++it, ++segmentIndex)
	{
		(*it)->findLocalTransformIndex(transformName, segmentLocalTransformIndex, foundIt);

		if (foundIt)
			return;
	}

	foundIt = false;
}

// ----------------------------------------------------------------------

void Skeleton::findTransformIndex(CrcString const &name, int *transformIndex, bool *found) const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::findTransformIndex");

	NOT_NULL(transformIndex);
	NOT_NULL(found);

	DEBUG_FATAL(m_modifyingSkeleton, ("invalid to call between beginSkeletonModification()/endSkeletonModification() call"));

	int  segmentIndex               = -1;
	int  segmentLocalTransformIndex = -1;
	
	findSegmentLocalTransformIndex(name, segmentIndex, segmentLocalTransformIndex, *found);

	if (*found)
	{
		DEBUG_FATAL(segmentIndex < 0, ("bad segmentIndex %d", segmentIndex));
		DEBUG_FATAL(segmentLocalTransformIndex < 0, ("bad index %d", segmentLocalTransformIndex));

		// convert local transform index to global transform index
		*transformIndex = (*m_skeletonSegments)[static_cast<size_t>(segmentIndex)]->getFirstTransformIndex() + segmentLocalTransformIndex;
	}
}

// ----------------------------------------------------------------------

int Skeleton::getTransformIndex(CrcString const &name) const
{
	bool found          = false;
	int  transformIndex = -1;

	findTransformIndex(name, &transformIndex, &found);

	FATAL(!found, ("failed to find transform named [%s]", name.getString()));
	return transformIndex; //lint !e527 // unreachable // yes, for MSVC
}

// ----------------------------------------------------------------------
/**
 * Find the global skeleton transform index for the immediate parent
 * transform of the specified child transform.
 *
 * @param childIndex  The skeleton global transform index of the transform
 *                    for which the parent transform index should be retrieved.
 *
 * @return  The skeleton global transform index for the parent transform.
 */

int Skeleton::getParentTransformIndex(int childIndex) const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::getParentTransformIndex");

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, childIndex, m_transformCount);

	//-- find segment of the child
	const int      childSegmentIndex = findSegmentIndex(childIndex);
	const Segment *segment           = (*m_skeletonSegments)[static_cast<size_t>(childSegmentIndex)];
	NOT_NULL(segment);

	const Segment *parentSegment = 0;
	int            parentSegmentLocalIndex = -1;
	bool           foundParentIndex = false;

	const int childSegmentLocalIndex = childIndex - segment->getFirstTransformIndex();
	segment->findParentIndex(childSegmentLocalIndex, parentSegment, parentSegmentLocalIndex, foundParentIndex);

	if (foundParentIndex)
	{
		// found the parent of the specified child
		NOT_NULL(parentSegment);
		DEBUG_FATAL(parentSegmentLocalIndex < 0, ("bogus parentSegmentLocalIndex %d", parentSegmentLocalIndex));

		return parentSegment->getFirstTransformIndex() + parentSegmentLocalIndex;
	}
	else
	{
		// the child did not have a parent
		return -1;
	}
}

// ----------------------------------------------------------------------

CrcString const &Skeleton::getTransformName(int transformIndex) const
{
	DEBUG_FATAL(m_modifyingSkeleton, ("invalid to call between beginSkeletonModification()/endSkeletonModification() call"));

	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::getTransformName");

	//-- find which segment the index belongs to
	const SegmentVector::const_iterator itEnd = m_skeletonSegments->end();
	for (SegmentVector::const_iterator it = m_skeletonSegments->begin(); it != itEnd; ++it)
	{
		Segment *const segment = NON_NULL(*it);

		// check if this index belongs to this skeleton template
		if ((transformIndex < segment->getFirstTransformIndex()) || (transformIndex > segment->getLastTransformIndex()))
		{
			// not it
			continue;
		}

		//-- convert global transform index into segment-local index
		const int localTransformIndex = transformIndex - segment->getFirstTransformIndex();

		//-- get the transform name
		return segment->getTransformName(localTransformIndex);
	}

	//-- scan hardpoints

	FATAL(true, ("failed to find transform name for given index [%d]", transformIndex));
	return *reinterpret_cast<CrcString*>(0); //lint !e527 !e413 // unreachable // yes, for MSVC
}

// ----------------------------------------------------------------------

const TransformNameMap &Skeleton::getTransformNameMap() const
{
	NOT_NULL(m_transformNameMap);
	return *m_transformNameMap;
}

// ----------------------------------------------------------------------

void Skeleton::addHardpoint(CrcString const &hardpointName, CrcString const &parentName, const Transform &hardpointToParent)
{
	DEBUG_FATAL(!m_modifyingSkeleton, ("must call addHardpoint() between beginSkeletonModification()/endSkeletonModification() call"));

	//-- find parent segment
	int   parentSegmentIndex        = -1;
	int   parentLocalTransformIndex = -1;
	bool  foundIt                   = false;
	
	findSegmentLocalTransformIndex(parentName, parentSegmentIndex, parentLocalTransformIndex, foundIt);
	if (!foundIt)
	{
		DEBUG_WARNING(true, ("hardpoint parent [%s] not found in skeleton, dropping hardpoint", parentName.getString()));
		return;
	}

	Segment *const segment = (*m_skeletonSegments)[static_cast<size_t>(parentSegmentIndex)];

	segment->addHardpoint(hardpointName, parentLocalTransformIndex, hardpointToParent);
}

// ----------------------------------------------------------------------

void Skeleton::removeAllHardpoints()
{
	DEBUG_FATAL(!m_modifyingSkeleton, ("must call removeAllHardpoints() between beginSkeletonModification()/endSkeletonModification() call"));

	std::for_each(m_skeletonSegments->begin(), m_skeletonSegments->end(), VoidMemberFunction(&Segment::removeAllHardpoints));
}

// ----------------------------------------------------------------------

int Skeleton::findSegmentIndex(int globalTransformIndex) const
{
	DEBUG_FATAL(m_modifyingSkeleton, ("invalid to call between beginSkeletonModification()/endSkeletonModification() call"));

	const size_t segmentCount = m_skeletonSegments->size();
	for (size_t i = 0; i < segmentCount; ++i)
	{
		const Segment &segment = *NON_NULL((*m_skeletonSegments)[i]);
		if ((globalTransformIndex >= segment.getFirstTransformIndex()) && (globalTransformIndex <= segment.getLastTransformIndex()))
			return static_cast<int>(i);
	}

	DEBUG_FATAL(true, ("failed to find skeleton segment for transform index %d", globalTransformIndex));
	return 0; //lint !e527 // unreachable // reachable in release
}

// ----------------------------------------------------------------------

void Skeleton::rebuildLinkedSkeletonDefinition()
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::rebuildLinkedSkeletonDefinition");

	m_transformCount = std::accumulate(m_skeletonSegments->begin(), m_skeletonSegments->end(), 0, Segment::transformCountAccumulate);

	//-- resize containers associated with transform count
	allocateTransformArrays(m_transformCount);

	//-- walk through each skeleton segment and build required data
	// NOTE: this code assumes an earlier segment never attaches to a later segment.
	//       that implies that attached skeleton segments should be added after the
	//       the skeleton segment to which they attach.

	AttachmentChainVector attachmentChains;
	attachmentChains.resize(m_skeletonSegments->size());

	const size_t segmentCount = m_skeletonSegments->size();

	int nextSegmentBaseTransformIndex = 0;
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
	{
		//-- get the skeleton segment
		Segment &segment = *NON_NULL((*m_skeletonSegments)[segmentIndex]);

		//-- find the parent transform index if this segment is attached to another.
		//   also get the bind pose modelToJoint transforms for this skeleton segment hierarchy.
		const BasicSkeletonTemplate::ModelToJointInfo *oldModelToJointInfo = segment.getModelToJointInfo();

		if (segment.isAttached())
		{
			//-- calculate global transform index of parent
			segment.setParentGlobalTransformIndex(segment.getParentSegment().getFirstTransformIndex() + segment.getParentLocalTransformIndex());

			//-- setup attachment chain
			// get index of parent segment
			const SegmentVector::iterator findIt = std::find(m_skeletonSegments->begin(), m_skeletonSegments->end(), &segment.getParentSegment());
			DEBUG_FATAL(findIt == m_skeletonSegments->end(), ("failed to find parent segment in segment list"));

			const size_t parentSegmentIndex = static_cast<size_t>(std::distance(m_skeletonSegments->begin(), findIt));

			// set attachment chain to that of parent
 			AttachmentChain &attachmentChain = attachmentChains[segmentIndex];
			attachmentChain = attachmentChains[parentSegmentIndex];

			// add in the current attachment info
			// -TRF- get the getTransformName() call out of this; possibly rework attachment chain to use local transform index
			const char *const parentSkeletonTemplateName = segment.getParentSegment().getSkeletonTemplate().getName().getString();
			attachmentChain.push_back(AttachmentChain::value_type(parentSkeletonTemplateName, getTransformName(segment.getParentGlobalTransformIndex()).getString()));

			//-- fetch the modelToJoint transforms for this skeleton template configuration
			const Transform &parentModelToJointTransform = segment.getParentSegment().getBindPoseModelToJointTransforms()[static_cast<size_t>(segment.getParentLocalTransformIndex())];

			segment.setupBindPoseModelToJointTransforms(attachmentChains[segmentIndex], parentModelToJointTransform);
		}
		else
		{
			// segment is not attached to a parent skeleton
			segment.setupBindPoseModelToJointTransforms(attachmentChains[segmentIndex], Transform::identity);
		}

		//-- release old model to joint transforms
		if (oldModelToJointInfo)
			segment.getSkeletonTemplate().releaseBindPoseModelToJointTransforms(oldModelToJointInfo);

		//-- get number of transforms in skeleton template
		const int segmentTransformCount = Segment::transformCountAccumulate(0, &segment);

		//-- setup transform index range and adjust transform count
		segment.setFirstTransformIndex(nextSegmentBaseTransformIndex);

		nextSegmentBaseTransformIndex += segmentTransformCount;
		segment.setLastTransformIndex(nextSegmentBaseTransformIndex - 1);

		//-- Rebuild the segment's transform resolver joint index lookup
		segment.buildTransformResolverJointIndexMap(m_animationResolver);
	}
}

// ----------------------------------------------------------------------
/**
 * Immediately render joint frames like Maya.
 *
 * This function has the side effect of calculating the joint-to-root
 * transforms.  This potentially is a repetition of work.
 *
 * This is implemented as a separate function because Maya renders
 * its joint frames after the preMultiply but before the rotation.
 * The simple way for us to render the frame would be after the entire
 * rotation multiply, which is after the animation rotation and post-multiply.
 */

void Skeleton::drawJointFramesNow(const Transform &skeletonToWorld, const Vector &scale) const
{
	NOT_NULL(m_skeletonSegments);
	NOT_NULL(m_jointToRootTransforms);

	Transform   renderFrameToWorldTransform(Transform::IF_none);
	Transform   renderFrameToParentTransform(Transform::IF_none);
	Transform   renderJointToParentTransform(Transform::IF_none);

	Quaternion  animationResolverRotation;
	Vector      animationResolverTranslation;

	//-- sweep through the skeleton segments
	const size_t segmentCount = m_skeletonSegments->size();
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
	{
		const Segment *const segment = (*m_skeletonSegments)[segmentIndex];
		NOT_NULL(segment);

		const std::vector<int> &jointIndexMap = segment->getTransformResolverJointIndexMap();

		//-- handle segment skeleton joints
		const BasicSkeletonTemplate &skeletonTemplate = segment->getSkeletonTemplate();
		const int                    jointCount       = skeletonTemplate.getJointCount();

		const int        *localParentIndexArray  = NON_NULL(skeletonTemplate.getJointParentIndexArray());
		const Quaternion *preMultiplyRotations   = NON_NULL(skeletonTemplate.getPreMultiplyRotations());
		const Quaternion *postMultiplyRotations  = NON_NULL(skeletonTemplate.getPostMultiplyRotations());

		const Vector     *bindPoseTranslations   = NON_NULL(skeletonTemplate.getBindPoseTranslations());
		const Quaternion *bindPoseRotations      = NON_NULL(skeletonTemplate.getBindPoseRotations());

		for (int localJointIndex = 0; localJointIndex < jointCount; ++localJointIndex)
		{
			const int globalTransformIndex            = localJointIndex + segment->getFirstTransformIndex();

			//-- Get animation resolver's transform components for this joint.
			const int animationResolverTransformIndex = jointIndexMap[static_cast<std::vector<int>::size_type>(localJointIndex)];
			DEBUG_FATAL(animationResolverTransformIndex < 0, ("animation resolver has not entry for appearance [%s],joint [%s],transform count [%d].", m_animationResolver.getSkeletalAppearanceTemplate().getName(), segment->getTransformName(localJointIndex).getString(), m_transformCount));

			m_animationResolver.getTransformComponents(animationResolverTransformIndex, animationResolverRotation, animationResolverTranslation);

			//-- Build the jointToParent transform

			// Animations are relative to bind pose.  Calculate absolute animated rotation and translation
			const Quaternion animatedRotation      = animationResolverRotation * bindPoseRotations[localJointIndex];
			const Vector localToParentTranslation  = bindPoseTranslations[localJointIndex] + animationResolverTranslation;

			// Apply pre-animated and post-animated rotations
			// -TRF- quat multiply is expensive: since many joints have identity pre-multiplies, should check for this at export time and flag them.
			const Quaternion localToParentRotation = postMultiplyRotations[localJointIndex] * (animatedRotation * preMultiplyRotations[localJointIndex]);

			// construct the transform
			Transform jointToParentTransform(Transform::IF_none);

			localToParentRotation.getTransformPreserveTranslation(&jointToParentTransform);
			jointToParentTransform.setPosition_p(localToParentTranslation);

			//-- build jointToRoot through parentToRoot * jointToParent
			// get dest transform location
			Transform &destTransform = m_jointToRootTransforms[static_cast<size_t>(globalTransformIndex)];

			// get parent transform index
			const int localParentIndex = localParentIndexArray[localJointIndex];
			DEBUG_FATAL(localParentIndex >= localJointIndex, ("invalid: skeleton template has child transform existing before parent transform [%d/%d]", localParentIndex, localJointIndex));

			Transform *parentToRootTransform = 0;

			if (localParentIndex < 0)
			{
				// this is the root joint for this skeleton template
				if (segment->getParentGlobalTransformIndex() >= 0)
					parentToRootTransform = m_jointToRootTransforms + segment->getParentGlobalTransformIndex();
				else
					parentToRootTransform = m_scaleTransform;
			}
			else
			{
				// this is a typical non-root joint for the skeleton template
				const size_t parentTransformIndex = static_cast<size_t>(segment->getFirstTransformIndex() + localParentIndex);
				parentToRootTransform             = m_jointToRootTransforms + parentTransformIndex;
			}

			//-- setup joint rendering
			preMultiplyRotations[localJointIndex].getTransform(&renderJointToParentTransform);
			renderJointToParentTransform.setPosition_p(localToParentTranslation);

			if (!parentToRootTransform)
			{
				destTransform                = jointToParentTransform;
				renderFrameToParentTransform = renderJointToParentTransform;
			}
			else
			{
				destTransform.multiply(*parentToRootTransform, jointToParentTransform);
				renderFrameToParentTransform.multiply(*parentToRootTransform, renderJointToParentTransform);
			}

			//-- render joint
			renderFrameToWorldTransform.multiply(skeletonToWorld, renderFrameToParentTransform);

			Graphics::setObjectToWorldTransformAndScale(renderFrameToWorldTransform, scale);
			Graphics::drawFrame(0.05f);
		}
	}
}

// ----------------------------------------------------------------------

void Skeleton::calculateJointToRootTransforms() const
{
	NOT_NULL(m_skeletonSegments);
	NOT_NULL(m_jointToRootTransforms);

	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::calculateJointToRootTransforms");

	const int frameNumber = Os::getNumberOfUpdates();
	if (m_frameLastJointToRootCalculate == frameNumber)
	{
		// we've already calculated this
		return;
	}

#if PRODUCTION == 0
	++s_calculateJointToRootTransformsEvalCount;
#endif

	//-- Get alter's elapsed time.  Needed for transform modifiers.
	float const elapsedTime = getAnimationResolver().getMostRecentAlterElapsedTime();

	Quaternion  animationResolverRotation;
	Vector      animationResolverTranslation;

	//-- sweep through the skeleton segments
	const size_t segmentCount = m_skeletonSegments->size();
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
	{
		const Segment *const segment = (*m_skeletonSegments)[segmentIndex];
		NOT_NULL(segment);

		const std::vector<int> &jointIndexMap = segment->getTransformResolverJointIndexMap();

		//-- handle segment skeleton joints
		const BasicSkeletonTemplate &skeletonTemplate = segment->getSkeletonTemplate();
		const int                    jointCount       = skeletonTemplate.getJointCount();

		const int        *localParentIndexArray  = NON_NULL(skeletonTemplate.getJointParentIndexArray());
		const Quaternion *preMultiplyRotations   = NON_NULL(skeletonTemplate.getPreMultiplyRotations());
		const Quaternion *postMultiplyRotations  = NON_NULL(skeletonTemplate.getPostMultiplyRotations());

		const Vector     *bindPoseTranslations   = NON_NULL(skeletonTemplate.getBindPoseTranslations());
		const Quaternion *bindPoseRotations      = NON_NULL(skeletonTemplate.getBindPoseRotations());

		//-- Setup transform modifier application during main loop.
		TransformModifierMap::iterator  modifierEndIt;
		TransformModifierMap::iterator  modifierIt;
		TransformModifierMap::iterator *modifierItPtr;

		if (m_transformModifierMap && !m_transformModifierMap->empty())
		{
			modifierEndIt = m_transformModifierMap->end();
			modifierIt    = m_transformModifierMap->begin();
			modifierItPtr = &modifierIt;
		}
		else
		{
			// No modifiers.
			modifierItPtr = 0;
		}

		for (int localJointIndex = 0; localJointIndex < jointCount; ++localJointIndex)
		{
			const int globalTransformIndex = localJointIndex + segment->getFirstTransformIndex();

			//-- Get animation resolver's transform components for this joint.
			const int animationResolverTransformIndex = jointIndexMap[static_cast<std::vector<int>::size_type>(localJointIndex)];
			if (animationResolverTransformIndex < 0)
			{
				// Somehow we've got an asset that thinks a specified transform is needed but the skeleton template doesn't provide it.
				// Skip this transform and hope for the best.
				// @todo detect what is causing this and generate a warning at load time of the other asset.
				DEBUG_WARNING(true, ("animation resolver has no entry for appearance [%s],joint [%s], skeleton transform count [%d].", m_animationResolver.getSkeletalAppearanceTemplate().getName(), segment->getTransformName(localJointIndex).getString(), m_transformCount));
				continue;
			}

			m_animationResolver.getTransformComponents(animationResolverTransformIndex, animationResolverRotation, animationResolverTranslation);

			//-- Build the jointToParent transform.

			// animations are relative to bind pose.  calculate absolute animated rotation and translation
			const Quaternion animatedRotation      = animationResolverRotation * bindPoseRotations[localJointIndex];
			const Vector localToParentTranslation  = bindPoseTranslations[localJointIndex] + animationResolverTranslation;

			// apply pre-animated and post-animated rotations
			// -TRF- quat multiply is expensive: since many joints have identity pre-multiplies, should check for this at export time and flag them
			const Quaternion localToParentRotation = postMultiplyRotations[localJointIndex] * (animatedRotation * preMultiplyRotations[localJointIndex]);

			// construct the transform
			Transform jointToParentTransform(Transform::IF_none);

			localToParentRotation.getTransformPreserveTranslation(&jointToParentTransform);
			jointToParentTransform.setPosition_p(localToParentTranslation);

			//-- build jointToRoot through parentToRoot * jointToParent
			// get dest transform location
			Transform &destTransform = m_jointToRootTransforms[globalTransformIndex];

			// get parent transform index
			const int localParentIndex = localParentIndexArray[localJointIndex];
			DEBUG_FATAL(localParentIndex >= localJointIndex, ("invalid: skeleton template has child transform existing before parent transform [%d/%d]", localParentIndex, localJointIndex));

			Transform const *parentToRootTransform = 0;

			if (localParentIndex < 0)
			{
				// this is the root joint for this skeleton template
				if (segment->getParentGlobalTransformIndex() < 0)
				{
					// this is the root of the entire skeleton. no parentToRoot
					parentToRootTransform = m_scaleTransform;
				}
				else
				{
					// link root of this skeleton segment to joint of parent skeleton
					parentToRootTransform = &(m_jointToRootTransforms[segment->getParentGlobalTransformIndex()]);
				}
			}
			else
			{
				// this is a typical non-root joint for the skeleton template
				size_t const parentTransformIndex  = static_cast<size_t>(segment->getFirstTransformIndex() + localParentIndex);
				parentToRootTransform = &(m_jointToRootTransforms[parentTransformIndex]);
			}

			// build the joint to root (i.e. joint to object) transform.
			NOT_NULL(parentToRootTransform);
			destTransform.multiply(*parentToRootTransform, jointToParentTransform);

			//-- Apply transform modifiers.
			while (modifierItPtr && ((*modifierItPtr)->first == globalTransformIndex))
			{
				// Get modifier.
				TransformModifier *const modifier = (*modifierItPtr)->second;
				NOT_NULL(modifier);

				// Apply modifier.
				bool const transformModified = modifier->modifyTransform(elapsedTime, *this, skeletonTemplate.getJointName(localJointIndex), *parentToRootTransform, jointToParentTransform, destTransform);
				UNREF(transformModified);

				// Move to next modifier.
				++modifierIt;
				if (modifierIt == modifierEndIt)
				{
					// Clear the modifier pointer so we know we're done.
					modifierItPtr = 0;
				}
			}
		}

		//-- handle segment hardpoints
		segment->calculateHardpointToRootTransforms(segment->getFirstTransformIndex(), m_jointToRootTransforms);
	}

	//-- remember that we calculated this for this frame
	m_frameLastJointToRootCalculate = frameNumber;
}

// ----------------------------------------------------------------------

void Skeleton::calculateBindPoseModelToRootTransforms() const
{
	// -TRF- NOTE: we do not support bind pose model to root calculation for
	//             hardpoints yet (these potentially require per-skeleton
	//             bindPoseModelToJoint transforms).

	NOT_NULL(m_skeletonSegments);
	NOT_NULL(m_bindPoseModelToRootTransforms);
	NOT_NULL(m_jointToRootTransforms);

	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::calculateBindPoseModelToRootTransforms");

	const int frameNumber = Os::getNumberOfUpdates();
	if (m_frameLastBindPoseModelToRootCalculate == frameNumber)
	{
		// we've already calculated this
		return;
	}

#if PRODUCTION == 0
	++s_calculateBindPoseModelToRootTransformsEvalCount;
#endif

	//-- make sure JointToRootTransforms are calculated already
	calculateJointToRootTransforms();

	//-- sweep through the skeleton segments
	const size_t segmentCount = m_skeletonSegments->size();
	for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
	{
		const Segment *const    segment                = NON_NULL((*m_skeletonSegments)[segmentIndex]);
		const int               jointCount             = segment->getSkeletonTemplate().getJointCount();
		const TransformVector  &modelToJointTransforms = segment->getBindPoseModelToJointTransforms();

		for (int localJointIndex = 0; localJointIndex < jointCount; ++localJointIndex)
		{
			//-- build the bindPoseModel -> Animated Joint
			const size_t        modelTransformIndex             = static_cast<size_t>(segment->getFirstTransformIndex() + localJointIndex);
			PoseModelTransform &destTransform                   = m_bindPoseModelToRootTransforms[modelTransformIndex];
			const Transform    &bindPoseModelToJointTransform   = modelToJointTransforms[static_cast<size_t>(localJointIndex)];
			const Transform    &jointToRootTransform            = m_jointToRootTransforms[modelTransformIndex];

			Transform tempXF(Transform::IF_none);
			tempXF.multiply(jointToRootTransform, bindPoseModelToJointTransform);
			destTransform=tempXF;
		}
	}

	//-- sweep through the hardpoints
	// -TRF- to do

	//-- remember we calculated this already for this frame
	m_frameLastBindPoseModelToRootCalculate = frameNumber;
}

// ----------------------------------------------------------------------

void Skeleton::addShaderPrimitives(const SkeletalAppearance2 &appearance) const
{
	//-- ensure shader primitive exists
	if (!m_shaderPrimitive)
	{
		//-- we assume there is a fixed 1-1 correspondence between skeleton and SkeletalAppearance
		m_shaderPrimitive = new LocalShaderPrimitive(appearance, *this);
	}

	ShaderPrimitiveSorter::add(*m_shaderPrimitive);
}

// ----------------------------------------------------------------------

void Skeleton::drawNow(const Transform &skeletonToWorld, const Vector &scale) const
{
	NOT_NULL(m_skeletonSegments);
	NOT_NULL(m_jointToRootTransforms);

	//-- make sure we've computed our transforms this frame
	calculateJointToRootTransforms();

	//-- render the skeleton hierarchy
	Transform  jointToWorldTransform(Transform::IF_none);

	//-- rifle through each segment.  we do this so we keep track
	//   of what transforms are joints and what are hardpoints.  otherwise
	//   we would need some kind of auxilliary data structure (= more space!).

	{
		const size_t segmentCount = m_skeletonSegments->size();
		for (size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
		{
			const Segment *const    segment            = NON_NULL((*m_skeletonSegments)[segmentIndex]);
			const int *const        localParentIndices = NON_NULL(segment->getSkeletonTemplate().getJointParentIndexArray());
			
			for (int globalTransformIndex = segment->getFirstTransformIndex(); globalTransformIndex <= segment->getLastTransformIndex(); ++globalTransformIndex)
			{
				const Transform *parentLocalToRoot = 0;

				//-- set parentLocalToRoot transform (the parent of the current transform)
				if (globalTransformIndex == segment->getFirstTransformIndex())
				{
					// this is the local root joint for the skeleton segment, handle attachment to other skeleton
					if (segment->getParentGlobalTransformIndex() < 0)
					{
						// this is the global root of the entire skeleton, there is nothing to draw here
						continue;
					}

					// start the line at the location of this segment's parent's tranform
					parentLocalToRoot = m_jointToRootTransforms + segment->getParentGlobalTransformIndex();
				}
				else
				{
					// this is a typical non-root transform
					const size_t localChildIndex  = static_cast<size_t>(globalTransformIndex - segment->getFirstTransformIndex());
					const bool   isSkeletonJoint  = (static_cast<int>(localChildIndex) < segment->getSkeletonTemplate().getJointCount());

					// do not process hardpoints
					if (isSkeletonJoint)
					{
						const int    localParentIndex = localParentIndices[localChildIndex];
						DEBUG_FATAL(localParentIndex < 0, ("we're processing what should be a non-root joint but has parent index of [%d]", localParentIndex));
					
						const size_t globalParentIndex = static_cast<size_t>(segment->getFirstTransformIndex() + localParentIndex);

						parentLocalToRoot = m_jointToRootTransforms + globalParentIndex;
					}
				}

				//-- only render when parentLocalToRoot is set.  Hardpoints will not set this.
				if (parentLocalToRoot)
				{
#if 0
					//-- render a frame for the joint
					jointToWorldTransform.multiply(skeletonToWorld, *parentLocalToRoot);

					Graphics::setObjectToWorldTransformAndScale(jointToWorldTransform, scale);
					Graphics::drawFrame(0.05f);
					Graphics::setObjectToWorldTransformAndScale(skeletonToWorld, scale);
#endif

					//-- set this child joint's childLocalToRoot
					Transform *const childLocalToRoot = m_jointToRootTransforms + globalTransformIndex;
					NOT_NULL(childLocalToRoot);

					//-- add axes for zeroAnimation joint orientation
					//-TRF- to do
					
					//-- find center of joint for parent
					const Vector parentVector = parentLocalToRoot->rotateTranslate_l2p(Vector::zero);

					//-- find center of joint for child
					const Vector childVector = childLocalToRoot->rotateTranslate_l2p(Vector::zero);

					//-- render bone from joint to parent
					Graphics::drawLine(parentVector, childVector, VectorArgb::solidWhite);
				}
			}
		}

		// render the joint frames like Maya
		drawJointFramesNow(skeletonToWorld, scale);
	}

	//-- render each hardpoint
	//   we do the skeleton bones first because we'll change the ObjectToWorld transform for
	//   each hardpoint.
	{
		const SegmentVector::const_iterator endIt = m_skeletonSegments->end();
		for (SegmentVector::const_iterator it = m_skeletonSegments->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			const Segment &segment = *(*it);

			segment.drawHardpointsNow(skeletonToWorld, scale, segment.getFirstTransformIndex(), m_jointToRootTransforms);
		}
	}
}

// ----------------------------------------------------------------------

const Transform *Skeleton::getJointToRootTransformArray() const
{
	NOT_NULL(m_jointToRootTransforms);

	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::getJointToRootTransformArray");

#if PRODUCTION == 0
	++s_calculateJointToRootTransformsCallCount;
#endif

	// this caches values for frame
	calculateJointToRootTransforms();

	return m_jointToRootTransforms;
}

// ----------------------------------------------------------------------

const PoseModelTransform *Skeleton::getBindPoseModelToRootTransforms() const
{
	NOT_NULL(m_bindPoseModelToRootTransforms);

	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::getBindPoseModelToRootTransforms");

#if PRODUCTION == 0
	++s_calculateBindPoseModelToRootTransformsCallCount;
#endif

	// this caches values for frame
	calculateBindPoseModelToRootTransforms();

	return m_bindPoseModelToRootTransforms;
}

// ----------------------------------------------------------------------
/**
 *
 * Calculate extent of skeleton in object space.
 */

void Skeleton::calculateExtent() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::calculateExtent");

	//-- check if we've already calculated the extents this frame
	const int frameNumber = Os::getNumberOfUpdates();
	if (m_frameLastExtentCalculate == frameNumber)
	{
		// we've already calculated this
		return;
	}

#if PRODUCTION == 0
	++s_calculateExtentEvalCount;
#endif

	//-- we need the joint to root calculated
	calculateJointToRootTransforms();
	
	NOT_NULL(m_extent);
	NOT_NULL(m_jointToRootTransforms);

	//-- calculate the extent
	if (m_transformCount)
	{
		//-- set extent to the projection of point through root transform
		// assumes root transform is at index 0.  this has to be the case since
		// we enforce all parents must precede their children in the hierarchy.
		const Transform &rootTransform = m_jointToRootTransforms[0];
		const Vector     rootPosition  = rootTransform.rotateTranslate_l2p(Vector::zero);
		m_extent->set(rootPosition, rootPosition, rootPosition, CONST_REAL(0));

		// note: we're spinning through all transforms, including any hardpoints
		for (int transformIndex = 1; transformIndex < m_transformCount; ++transformIndex)
		{
			const Transform &transform = m_jointToRootTransforms[transformIndex];
			const Vector     position  = transform.rotateTranslate_l2p(Vector::zero);
			m_extent->updateMinAndMax(position);
		}
	}
	else
	{
		m_extent->set(Vector::zero, Vector::zero, Vector::zero, CONST_REAL(2));
	}
	m_extent->calculateCenterAndRadius();

	//-- remember we've done the calculation this frame
	m_frameLastExtentCalculate = frameNumber;
}

// ----------------------------------------------------------------------

const BoxExtent *Skeleton::getExtent() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("Skeleton::getExtent");

#if PRODUCTION == 0
	++s_calculateExtentCallCount;
#endif

	//-- calculate the extent (lazy evaluation)
	calculateExtent();

	return m_extent;
}

// ----------------------------------------------------------------------

void Skeleton::setScale(float scale)
{
	if (scale > 0.0f)
	{
		m_scale = scale;
		m_scaleTransform->setToScale(Vector(scale, scale, scale));

		m_frameLastJointToRootCalculate         = -1;
		m_frameLastBindPoseModelToRootCalculate = -1;
		m_frameLastExtentCalculate              = -1;
	}
	else
	{
		DEBUG_WARNING(true, ("tried to set Skeleton scale <= 0 [%g].", scale));
	}
}

// ----------------------------------------------------------------------

void Skeleton::clearAllTransformModifiers()
{
	if (m_transformModifierMap)
		m_transformModifierMap->clear();
}

// ----------------------------------------------------------------------

void Skeleton::attachTransformModifier(int transformIndex, TransformModifier *transformModifier)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, getTransformCount());
	NOT_NULL(transformModifier);

	//-- Create modifier container as necessary.
	if (!m_transformModifierMap)
		m_transformModifierMap = new TransformModifierMap;

#ifdef _DEBUG
	//-- Check if modifier is already applied to this joint.  
	//   It is likely an error to have multiple modifiers affect the same joint
	//   since the interface currently provides no way to specify the order of applied modifiers.
	TransformModifierMap::iterator const findIt = m_transformModifierMap->find(transformIndex);
	DEBUG_FATAL(findIt != m_transformModifierMap->end(), ("attachTransformModifier(): tried to attach multiple modifiers to joint [%s], appearance [%s].", 
		getTransformName(transformIndex).getString(), 
		m_animationResolver.getSkeletalAppearanceTemplate().getName() ? m_animationResolver.getSkeletalAppearanceTemplate().getName() : "<NULL appearance template name>"));
#endif

	IGNORE_RETURN(m_transformModifierMap->insert(TransformModifierMap::value_type(transformIndex, transformModifier)));
}

// ----------------------------------------------------------------------

SkeletalAppearance2 &Skeleton::getSkeletalAppearance()
{
	return m_animationResolver.getSkeletalAppearance();
}

// ----------------------------------------------------------------------

SkeletalAppearance2 const &Skeleton::getSkeletalAppearance() const
{
	return m_animationResolver.getSkeletalAppearance();
}

// ======================================================================
// class Skeleton: PRIVATE STATIC
// ======================================================================

void Skeleton::remove()
{
	DEBUG_FATAL(!s_installed, ("Skeleton not installed."));
	s_installed = false;

	removeMemoryBlockManager();
}

// ======================================================================
// class Skeleton: PRIVATE
// ======================================================================

void Skeleton::allocateTransformArrays(int transformCount)
{
	//-- Free existing storage.
	if (m_jointToRootTransformBackingStore)
	{
		delete [] m_jointToRootTransformBackingStore;
		m_jointToRootTransformBackingStore = 0;
		m_jointToRootTransforms            = 0;
	}

	if (m_bindPoseModelToRootTransformBackingStore)
	{
		delete [] m_bindPoseModelToRootTransformBackingStore;
		m_bindPoseModelToRootTransformBackingStore = 0;
		m_bindPoseModelToRootTransforms            = 0;
	}

	//-------------------------------------------------------
	//-- Create new storage, aligned to 32 bytes.
	// jointToRoot
	const size_t dataSize = static_cast<size_t>(transformCount) * sizeof(Transform) + 31;
	m_jointToRootTransformBackingStore = new uint8[dataSize];

	m_jointToRootTransforms = reinterpret_cast<Transform*>(POINTER_ALIGN_32(m_jointToRootTransformBackingStore));

	//-------------------------------------------------------
	// bindPoseModelToRoot
	const size_t poseDataSize = static_cast<size_t>(transformCount) * sizeof(PoseModelTransform) + 31;
	m_bindPoseModelToRootTransformBackingStore = new uint8[poseDataSize];

	m_bindPoseModelToRootTransforms = reinterpret_cast<PoseModelTransform*>(POINTER_ALIGN_32(m_bindPoseModelToRootTransformBackingStore));
	//-------------------------------------------------------

	m_frameLastExtentCalculate              = -1;
	m_frameLastJointToRootCalculate         = -1;
	m_frameLastBindPoseModelToRootCalculate = -1;
}

//----------------------------------------------------------------------

const Transform * Skeleton::findTransform(CrcString const &name) const
{
	//-- Retrieve the transform for the given name.
	bool found         = false;
	int transformIndex = 0;

	findTransformIndex (name, &transformIndex, &found);

	if (found)
		return &getJointToRootTransformArray()[transformIndex];
	else
		return 0;
}

// ==================================================================
