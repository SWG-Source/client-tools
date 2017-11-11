// ======================================================================
//
// KeyframeSkeletalAnimation.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimation.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimationTemplate.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================
// lint supression
// ======================================================================

// error 754: local structure member unreferenced - I'm shutting this off for all the templates
//lint -e754

// error 578: hiding another definition
//lint -esym(578, time)

// ======================================================================

MemoryBlockManager *KeyframeSkeletalAnimation::ms_skeletalAnimationMemoryBlockManager;
bool                              KeyframeSkeletalAnimation::ms_testInvariants;

// ======================================================================

class KeyframeSkeletalAnimation::QuaternionOperation
{
public:

	virtual      ~QuaternionOperation();
	virtual void  evaluateCurrentTime(float animationFrameNumber, Quaternion &result) const = 0;

};

// ======================================================================

class KeyframeSkeletalAnimation::StaticQuaternionOperation: public KeyframeSkeletalAnimation::QuaternionOperation
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit StaticQuaternionOperation(const Quaternion &quaternion);
	virtual void  evaluateCurrentTime(float animationFrameNumber, Quaternion &result) const;

private:

	// disabled
	StaticQuaternionOperation();

private:

	Quaternion  m_quaternion;

};

// ======================================================================

class KeyframeSkeletalAnimation::VectorOperation
{
public:

	virtual      ~VectorOperation();
	virtual void  evaluateCurrentTime(float animationFrameNumber, Vector &result) const = 0;

};

// ======================================================================

class KeyframeSkeletalAnimation::StaticVectorOperation: public KeyframeSkeletalAnimation::VectorOperation
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	explicit StaticVectorOperation(const Vector &position);
	virtual void  evaluateCurrentTime(float animationFrameNumber, Vector &result) const;

private:

	// disabled
	StaticVectorOperation();

private:

	Vector  m_vector;

};

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum Component
	{
		C_xTranslation,
		C_yTranslation,
		C_zTranslation,
		C_xRotation,
		C_yRotation,
		C_zRotation
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class StaticEvaluator
	{
	public:
		StaticEvaluator(Component component, const KeyframeSkeletalAnimation &skeletalAnimation, const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo);

		real evaluate(float animationFrameNumber) const
			{
				UNREF(animationFrameNumber);
				return m_value;
			}

	private:

		real m_value;

	private:
		// disabled
		StaticEvaluator();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class KeyframeEvaluator
	{
	public:
		KeyframeEvaluator(Component component, const KeyframeSkeletalAnimation &skeletalAnimation, const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo);
		~KeyframeEvaluator();

		real evaluate(float animationFrameNumber) const;

	private:

		const KeyframeSkeletalAnimationTemplate::RealKeyDataVector *m_keyDataVector;
		mutable int                                                 m_lastLowerKeyframeIndex;

	private:
		// disabled
		KeyframeEvaluator();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class KeyframeRotationOperation: public KeyframeSkeletalAnimation::QuaternionOperation
	{
	private:

		static MemoryBlockManager *ms_memoryBlockManager;

	private:

		const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &m_keyDataVector;
		mutable int                                                       m_lastLowerKeyframeIndex;

	public:

		static void install();
		static void remove();
		static void *operator new(size_t size);
		static void  operator delete(void *data, size_t size);

	public:

		explicit KeyframeRotationOperation(const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &keyDataVector);

		virtual void evaluateCurrentTime(float animationFrameNumber, Quaternion &result) const;

	private:

		// disabled
		KeyframeRotationOperation();
		KeyframeRotationOperation(const KeyframeRotationOperation &);
		KeyframeRotationOperation &operator =(const KeyframeRotationOperation&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	template <class EvaluatorX, class EvaluatorY, class EvaluatorZ>
	class KeyframeTranslationOperation: public KeyframeSkeletalAnimation::VectorOperation
	{
	private:

		static MemoryBlockManager *ms_memoryBlockManager;

	private:

		EvaluatorX                       m_xEvaluator;
		EvaluatorY                       m_yEvaluator;
		EvaluatorZ                       m_zEvaluator;

	public:

		static void install()
			{
				ms_memoryBlockManager = new MemoryBlockManager("KeyframeTranslationOperation", true, sizeof(KeyframeTranslationOperation<EvaluatorX, EvaluatorY, EvaluatorZ>), 0, 0, 0);
			}

		static void remove()
			{
				delete ms_memoryBlockManager;
				ms_memoryBlockManager = 0;
			}

		static void *operator new(size_t size)
			{
				UNREF(size);
				NOT_NULL(ms_memoryBlockManager);
				DEBUG_FATAL(size != sizeof(KeyframeTranslationOperation<EvaluatorX, EvaluatorY, EvaluatorZ>), ("appears that derived class is trying to new with our allocator"));
				return ms_memoryBlockManager->allocate();
			}

		static void  operator delete(void *data, size_t size)
			{
				UNREF(size);
				NOT_NULL(ms_memoryBlockManager);
				DEBUG_FATAL(size != sizeof(KeyframeTranslationOperation<EvaluatorX, EvaluatorY, EvaluatorZ>), ("appears that derived class is trying to new with our allocator"));
				ms_memoryBlockManager->free(data);
			}

	public:

		KeyframeTranslationOperation(const KeyframeSkeletalAnimation &skeletalAnimation, const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo) :
				VectorOperation(), //lint !e1769 // no base class constructor // yes, maybe today, but...
				m_xEvaluator(C_xTranslation, skeletalAnimation, transformInfo),
				m_yEvaluator(C_yTranslation, skeletalAnimation, transformInfo),
				m_zEvaluator(C_zTranslation, skeletalAnimation, transformInfo)
			{
			}

		virtual void evaluateCurrentTime(float animationFrameNumber, Vector &result) const
			{
				result.x = m_xEvaluator.evaluate(animationFrameNumber);
				result.y = m_yEvaluator.evaluate(animationFrameNumber);
				result.z = m_zEvaluator.evaluate(animationFrameNumber);
			}

	private:

		// disabled
		KeyframeTranslationOperation();
		KeyframeTranslationOperation(const KeyframeTranslationOperation&);
		KeyframeTranslationOperation &operator =(const KeyframeTranslationOperation&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef KeyframeSkeletalAnimation::VectorOperation *(*TranslationOperationCreator)(const void *skeletalAnimation, const void *transformInfo);

	typedef std::vector<char*>                                  StringContainer;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                         ms_installed;
	TranslationOperationCreator  ms_translationCreators[8];

	MemoryBlockManager *KeyframeRotationOperation::ms_memoryBlockManager;

	MemoryBlockManager *KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   StaticEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   StaticEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, StaticEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, StaticEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   KeyframeEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   KeyframeEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, KeyframeEvaluator>::ms_memoryBlockManager;
	MemoryBlockManager *KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, KeyframeEvaluator>::ms_memoryBlockManager;

	StringContainer *ms_mbmNames;
}

// ======================================================================
// class KeyframeSkeletalAnimation::QuaternionOperation
// ======================================================================

KeyframeSkeletalAnimation::QuaternionOperation::~QuaternionOperation()
{
}


// ======================================================================
// class KeyframeSkeletalAnimation::StaticQuaternionOperation
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(KeyframeSkeletalAnimation::StaticQuaternionOperation, true, 0, 0, 0);

// ======================================================================

KeyframeSkeletalAnimation::StaticQuaternionOperation::StaticQuaternionOperation(const Quaternion &quaternion) :
	m_quaternion(quaternion)
{
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::StaticQuaternionOperation::evaluateCurrentTime(float animationFrameNumber, Quaternion &result) const
{
	UNREF(animationFrameNumber);
	result = m_quaternion;
}

// ======================================================================
// class KeyframeSkeletalAnimation::VectorOperation
// ======================================================================

KeyframeSkeletalAnimation::VectorOperation::~VectorOperation()
{
}

// ======================================================================
// class KeyframeSkeletalAnimation::StaticVectorOperation
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(KeyframeSkeletalAnimation::StaticVectorOperation, true, 0, 0, 0);

// ======================================================================

KeyframeSkeletalAnimation::StaticVectorOperation::StaticVectorOperation(const Vector &position) :
	m_vector(position)
{
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::StaticVectorOperation::evaluateCurrentTime(float animationFrameNumber, Vector &result) const
{
	UNREF(animationFrameNumber);
	result = m_vector;
}

// ======================================================================
// class StaticEvaluator
// ======================================================================

StaticEvaluator::StaticEvaluator(Component component, const KeyframeSkeletalAnimation &skeletalAnimation, const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo) :
	m_value(CONST_REAL(0))
{
	const KeyframeSkeletalAnimationTemplate *const sat = NON_NULL(skeletalAnimation.getOurTemplate());

	switch (component)
	{
		case C_xTranslation:
			m_value = sat->getStaticXTranslation(transformInfo);
			break;
		case C_yTranslation:
			m_value = sat->getStaticYTranslation(transformInfo);
			break;
		case C_zTranslation:
			m_value = sat->getStaticZTranslation(transformInfo);
			break;
		case C_xRotation:
		case C_yRotation:
		case C_zRotation:
			DEBUG_FATAL(true, ("you shouldn't get here\n"));
			break;

		default:
			FATAL(true, ("unknown component value [%u]", component));
	}
}

// ======================================================================
// class KeyframeEvaluator
// ======================================================================

KeyframeEvaluator::KeyframeEvaluator(Component component, const KeyframeSkeletalAnimation &skeletalAnimation, const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo) :
	m_keyDataVector(0),
	m_lastLowerKeyframeIndex(0)
{
	const KeyframeSkeletalAnimationTemplate *const sat = NON_NULL(skeletalAnimation.getOurTemplate());

	switch (component)
	{
		case C_xTranslation:
			m_keyDataVector = &(sat->getXTranslationChannelData(transformInfo));
			break;

		case C_yTranslation:
			m_keyDataVector = &(sat->getYTranslationChannelData(transformInfo));
			break;

		case C_zTranslation:
			m_keyDataVector = &(sat->getZTranslationChannelData(transformInfo));
			break;

		case C_xRotation:
		case C_yRotation:
		case C_zRotation:
			DEBUG_FATAL(true, ("you should not get here.\n"));
			break;

		default:
			FATAL(true, ("unknown component value [%u]", component));
	}

	NOT_NULL(m_keyDataVector);
}

// ----------------------------------------------------------------------

KeyframeEvaluator::~KeyframeEvaluator()
{
	// we don't own this
	m_keyDataVector = 0;
}

// ----------------------------------------------------------------------

real KeyframeEvaluator::evaluate(float animationFrameNumber) const
{
	const int   keyCount = static_cast<int>(m_keyDataVector->size());
	int         lowerKeyframeIndex;

	//-- find closest keyframe occurring on or before animation frame number
	{
		// check if we should try to use our keyframe index helper
		if ((*m_keyDataVector)[static_cast<size_t>(m_lastLowerKeyframeIndex)].m_frameNumber <= animationFrameNumber)
			lowerKeyframeIndex = m_lastLowerKeyframeIndex;
		else
			lowerKeyframeIndex = 0;

		for (; lowerKeyframeIndex < keyCount - 1; ++lowerKeyframeIndex)
		{
			if ((*m_keyDataVector)[static_cast<size_t>(lowerKeyframeIndex + 1)].m_frameNumber > animationFrameNumber)
				break;
		}

		m_lastLowerKeyframeIndex = lowerKeyframeIndex;
	}

	//-- get frame distance to next key
	const real oneOverFrameDistance = (*m_keyDataVector)[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		return (*m_keyDataVector)[static_cast<size_t>(lowerKeyframeIndex)].m_keyValue;
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE((*m_keyDataVector)[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, animationFrameNumber, (*m_keyDataVector)[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float lowerKeyframeWeight = ((*m_keyDataVector)[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber - animationFrameNumber) * oneOverFrameDistance;
		const float blendedValue        = (lowerKeyframeWeight * (*m_keyDataVector)[static_cast<size_t>(lowerKeyframeIndex)].m_keyValue) + ((1.0f - lowerKeyframeWeight) * (*m_keyDataVector)[static_cast<size_t>(upperKeyframeIndex)].m_keyValue);

		return blendedValue;
	}
}

// ======================================================================
// class KeyframeRotationOperation
// ======================================================================

void KeyframeRotationOperation::install()
{
	ms_memoryBlockManager = new MemoryBlockManager("KeyframeRotationOperation", true, sizeof(KeyframeRotationOperation), 0, 0, 0);
}

// ----------------------------------------------------------------------

void KeyframeRotationOperation::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *KeyframeRotationOperation::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(KeyframeRotationOperation), ("appears that derived class is trying to new with our allocator"));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void KeyframeRotationOperation::operator delete(void *data, size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(KeyframeRotationOperation), ("appears that derived class is trying to new with our allocator"));
	ms_memoryBlockManager->free(data);
}

// ======================================================================

KeyframeRotationOperation::KeyframeRotationOperation(const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &keyDataVector) :
	QuaternionOperation(),
	m_keyDataVector(keyDataVector),
	m_lastLowerKeyframeIndex(0)
{
}

// ----------------------------------------------------------------------

void KeyframeRotationOperation::evaluateCurrentTime(float animationFrameNumber, Quaternion &result) const
{
	//-- determine the lower and upper keyframe index
	const int   keyCount = static_cast<int>(m_keyDataVector.size());
	int         lowerKeyframeIndex;

	//-- find closest keyframe occurring on or before animation frame number
	{
		// check if we should try to use our keyframe index helper
		if (m_keyDataVector[static_cast<size_t>(m_lastLowerKeyframeIndex)].m_frameNumber <= animationFrameNumber)
			lowerKeyframeIndex = m_lastLowerKeyframeIndex;
		else
			lowerKeyframeIndex = 0;

		for (; lowerKeyframeIndex < keyCount - 1; ++lowerKeyframeIndex)
		{
			if (m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex + 1)].m_frameNumber > animationFrameNumber)
				break;
		}

		m_lastLowerKeyframeIndex = lowerKeyframeIndex;
	}

	//-- get frame distance to next key
	const float oneOverFrameDistance = m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		result = m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation;
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, animationFrameNumber, m_keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float       upperKeyframeWeight   = (animationFrameNumber - m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber) * oneOverFrameDistance;
		const Quaternion &upperKeyframeRotation = m_keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_rotation;
		const Quaternion &lowerKeyframeRotation = m_keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation;

		//-- return the interpolation between the rotations based on weighting
		result = lowerKeyframeRotation.slerp(upperKeyframeRotation, upperKeyframeWeight);
	}
}

// ======================================================================
// creation-related stand-alone functions
// ======================================================================

// ----------------------------------------------------------------------

#if 0

template <class EvaluatorX, class EvaluatorY, class EvaluatorZ>
static KeyframeSkeletalAnimation::QuaternionOperation *CreateRotationOperation(void *skeletalAnimation, void *transformInfo)
{
	NOT_NULL(skeletalTemplate);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeRotationOperation<EvaluatorX, EvaluatorY, EvaluatorZ>(skeletalAnimationRef, transformInfoRef);
}

#endif

// ----------------------------------------------------------------------

template <class EvaluatorX, class EvaluatorY, class EvaluatorZ>
static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation(void *skeletalAnimation, void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<EvaluatorX, EvaluatorY, EvaluatorZ>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_SSS(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<StaticEvaluator, StaticEvaluator, StaticEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_KSS(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator, StaticEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_SKS(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<StaticEvaluator, KeyframeEvaluator, StaticEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_KKS(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, StaticEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_SSK(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<StaticEvaluator, StaticEvaluator, KeyframeEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_KSK(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator, KeyframeEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_SKK(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<StaticEvaluator, KeyframeEvaluator, KeyframeEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ----------------------------------------------------------------------

static KeyframeSkeletalAnimation::VectorOperation *CreateTranslationOperation_KKK(const void *skeletalAnimation, const void *transformInfo)
{
	NOT_NULL(skeletalAnimation);
	NOT_NULL(transformInfo);

	const KeyframeSkeletalAnimation                        &skeletalAnimationRef = *(reinterpret_cast<const KeyframeSkeletalAnimation*>(skeletalAnimation));
	const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfoRef     = *(reinterpret_cast<const KeyframeSkeletalAnimationTemplate::TransformInfo*>(transformInfo));

	return new KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, KeyframeEvaluator>(skeletalAnimationRef, transformInfoRef);
}

// ======================================================================
// class KeyframeSkeletalAnimation: public static member functions
// ======================================================================

void KeyframeSkeletalAnimation::install()
{
	DEBUG_FATAL(ms_installed, ("KeyframeSkeletalAnimation already installed"));

	KeyframeSkeletalAnimation::StaticQuaternionOperation::install();
	KeyframeSkeletalAnimation::StaticVectorOperation::install();

	//-- setup the translation operation node creation functions
	//   for these, , ..._XYZ is replaced with (K) for keyframed and (S) for static value
	ms_translationCreators[0] = CreateTranslationOperation_SSS;
	ms_translationCreators[1] = CreateTranslationOperation_KSS;
	ms_translationCreators[2] = CreateTranslationOperation_SKS;
	ms_translationCreators[3] = CreateTranslationOperation_KKS;
	ms_translationCreators[4] = CreateTranslationOperation_SSK;
	ms_translationCreators[5] = CreateTranslationOperation_KSK;
	ms_translationCreators[6] = CreateTranslationOperation_SKK;
	ms_translationCreators[7] = CreateTranslationOperation_KKK;

	ms_mbmNames            = new StringContainer;

	//-- install the rotation operation
	KeyframeRotationOperation::install();

	//-- install the translation operations
	KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   StaticEvaluator>::install();
	KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   StaticEvaluator>::install();
	KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, StaticEvaluator>::install();
	KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, StaticEvaluator>::install();
	KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   KeyframeEvaluator>::install();
	KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   KeyframeEvaluator>::install();
	KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, KeyframeEvaluator>::install();
	KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, KeyframeEvaluator>::install();

	ms_skeletalAnimationMemoryBlockManager = new MemoryBlockManager("KeyframeSkeletalAnimation", true, sizeof(KeyframeSkeletalAnimation), 0, 0, 0);

	DebugFlags::registerFlag(ms_testInvariants, "ClientSkeletalAnimation/Character", "checkKeyframeInvariants");

	ms_installed = true;
	ExitChain::add(remove, "KeyframeSkeletalAnimation");
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::remove()
{
	DEBUG_FATAL(!ms_installed, ("KeyframeSkeletalAnimation not installed"));

	IGNORE_RETURN( std::for_each(ms_mbmNames->begin(), ms_mbmNames->end(), ArrayPointerDeleter()) );
	delete ms_mbmNames;
	ms_mbmNames = 0;

	ms_installed = false;

	delete ms_skeletalAnimationMemoryBlockManager;
	ms_skeletalAnimationMemoryBlockManager = 0;

	KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   StaticEvaluator>::remove();
	KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   StaticEvaluator>::remove();
	KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, StaticEvaluator>::remove();
	KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, StaticEvaluator>::remove();
	KeyframeTranslationOperation<StaticEvaluator,   StaticEvaluator,   KeyframeEvaluator>::remove();
	KeyframeTranslationOperation<KeyframeEvaluator, StaticEvaluator,   KeyframeEvaluator>::remove();
	KeyframeTranslationOperation<StaticEvaluator,   KeyframeEvaluator, KeyframeEvaluator>::remove();
	KeyframeTranslationOperation<KeyframeEvaluator, KeyframeEvaluator, KeyframeEvaluator>::remove();

	KeyframeRotationOperation::remove();
}

// ----------------------------------------------------------------------

void *KeyframeSkeletalAnimation::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("KeyframeSkeletalAnimation not installed"));
	DEBUG_FATAL(size != sizeof(KeyframeSkeletalAnimation), ("derived types not supported with this operator new"));
	UNREF(size);

	return ms_skeletalAnimationMemoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::operator delete(void *data)
{
	ms_skeletalAnimationMemoryBlockManager->free(data);
}

// ======================================================================
// class KeyframeSkeletalAnimation: public member functions
// ======================================================================

bool KeyframeSkeletalAnimation::alterSingleCycle(float deltaTime, SkeletalAnimation *&replacementAnimation, float &deltaTimeRemaining)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("KeyframeSkeletalAnimation::alterSingleCycle");

	DEBUG_WARNING(ms_testInvariants && !validateInvariantsWarn(), ("KeyframeSkeletalAnimation: invariant check failure."));
	UNREF(replacementAnimation);

	//-- Set previous frame number.
	m_previousFrameNumber = m_currentFrameNumber;

	//-- Determine time to apply during this alter.  It will not chew up
	//   time past the end of a loop.
	m_currentCycleTime += deltaTime;
	if (m_currentCycleTime > m_cyclePeriod)
	{
		//-- Clip the cycle time to the end of a cycle and determine the time remaining that hasn't been applied.
		deltaTimeRemaining = m_currentCycleTime - m_cyclePeriod;
		m_currentCycleTime = m_cyclePeriod;

		//-- Catch case of single-frame animation (m_cyclePeriod == 0.0f).
		//   In this case, indicate that all specified time has been eaten up
		//   by the alter.
		if (m_cyclePeriod <= 0.0f)
			deltaTimeRemaining = 0.0f;
	}
	else
	{
		//-- This alter will chew up all delta time.
		deltaTimeRemaining = 0.0f;
	}

	//-- Determine new frame number.
	m_currentFrameNumber = m_currentCycleTime * m_playbackFramesPerSecond;
	m_currentFrameNumber = std::min(m_currentFrameNumber, m_frameCount);

	//-- Indicate this animation has every intention of continuing to play if asked.
	return true;
} //lint !e1764 // make replacementAnimation const ref // sorry, part of contract.

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimation::getTransformCount() const
{
	const int rotationCount = static_cast<int>(m_rotationOperations.size());
	DEBUG_FATAL(rotationCount != static_cast<int>(m_translationOperations.size()), ("rotation/translation vector has different # entries."));

	return rotationCount;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::evaluateTransformComponents(int index, Quaternion &rotation, Vector &translation)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("KeyframeSkeletalAnimation::evaluateTransformComponents");

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());

	m_rotationOperations[static_cast<QuaternionOperationVector::size_type>(index)]->evaluateCurrentTime(m_currentFrameNumber, rotation);
	m_translationOperations[static_cast<VectorOperationVector::size_type>(index)]->evaluateCurrentTime(m_currentFrameNumber, translation);
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimation::getTransformPriority(int index) const
{
	UNREF(index);

	// @todo implement priority support.
	return 0;
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimation::getLocomotionPriority() const
{
	// @todo implement priority support.
	return 0;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::startNewCycle()
{
	m_currentCycleTime    = 0.0f;
	m_previousFrameNumber = 0.0f;
	m_currentFrameNumber  = 0.0f;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::getScaledLocomotion(Quaternion &rotation, Vector &translation) const
{
	DEBUG_WARNING(ms_testInvariants && !validateInvariantsWarn(), ("KeyframeSkeletalAnimation: invariant check failure."));

	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	//-- Get the non-scaled locomotion that occurred during the frame.
	sat->getLocomotion(m_previousFrameNumber, m_currentFrameNumber, m_rotationStartKeyIndex, m_translationStartKeyIndex, rotation, translation);

	//-- Scale translation appropriately.  Rotation is ignored.
	translation *= m_scale;
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimation::getCycleScaledLocomotionDistance() const
{
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	int        tempRotationKey    = 0;
	int        tempTranslationKey = 0;
	Quaternion tempRotation;
	Vector     tempTranslation;

	sat->getLocomotion(0.0f, m_frameCount, tempRotationKey, tempTranslationKey, tempRotation, tempTranslation);

	return tempTranslation.magnitude() * m_scale;
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimation::getFrameCount() const
{
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	return sat->getFrameCount();
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimation::getRecordedFramesPerSecond() const
{
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	return sat->getFramesPerSecond();
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::setPlaybackFramesPerSecond(float playbackFramesPerSecond)
{
	if (playbackFramesPerSecond <= 0.0f)
	{
		// @todo put this back in later.
		// DEBUG_WARNING(true, ("invalid playback frames per second [%g], ignoring request.", playbackFramesPerSecond));
		return;
	}

	m_playbackFramesPerSecond   = playbackFramesPerSecond;
	m_ooPlaybackFramesPerSecond = (m_playbackFramesPerSecond > 0.0f) ? (1.0f / m_playbackFramesPerSecond) : 1.0f;

	//-- Calculate cycle period.
	if (m_frameCount > 0.0f)
	{
		m_cyclePeriod         = m_frameCount / static_cast<float>(playbackFramesPerSecond);
		m_currentCycleTime    = (m_frameCount > 0.0f ? (m_currentFrameNumber / m_frameCount) * m_cyclePeriod : 0.0f);
	}
	else
		m_cyclePeriod = 0.0f;
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimation::getPlaybackFramesPerSecond() const
{
	return m_playbackFramesPerSecond;
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimation::getMessageCount() const
{
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	return sat->getMessageCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &KeyframeSkeletalAnimation::getMessageName(int index) const
{
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	return sat->getMessageName(index);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimation::getSignaledMessages(stdvector<int>::fwd &signaledMessageIndices, stdvector<float>::fwd &elapsedTimeSinceSignal) const
{
	DEBUG_WARNING(ms_testInvariants && !validateInvariantsWarn(), ("KeyframeSkeletalAnimation: invariant check failure."));

	//-- Calculate integral range [begin message scan, endMessageScan), an (inclusive, exclusive) range.
	const int beginRange = static_cast<int>(ceil(static_cast<double>(m_previousFrameNumber)));
	const int endRange   = static_cast<int>(ceil(static_cast<double>(m_currentFrameNumber)));

	//-- Retrieve message indices emitted during this interval
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	NOT_NULL(sat);

	sat->getSignaledMessages(beginRange, endRange, signaledMessageIndices, elapsedTimeSinceSignal);
	DEBUG_FATAL(signaledMessageIndices.size() != elapsedTimeSinceSignal.size(), ("getSignaledMessages(): returned unbalanced data [%d/%d].", static_cast<int>(signaledMessageIndices.size()), static_cast<int>(elapsedTimeSinceSignal.size())));

	//-- ElapsedTimeSinceSignal actually contains the frame numbers of signals right now.  We need to convert from frames to time.
	size_t const entryCount = elapsedTimeSinceSignal.size();
	for (size_t i = 0; i < entryCount; ++i)
	{
		float const signalFrameNumber = elapsedTimeSinceSignal[i];

		float timeSinceSignal;
		if (signalFrameNumber <= m_currentFrameNumber) 
			timeSinceSignal = (m_currentFrameNumber - signalFrameNumber) * m_ooPlaybackFramesPerSecond;
		else
			timeSinceSignal = ((m_frameCount - signalFrameNumber) + m_currentFrameNumber) * m_ooPlaybackFramesPerSecond;

		elapsedTimeSinceSignal[i] = timeSinceSignal;
	}

}

// ----------------------------------------------------------------------

SkeletalAnimation *KeyframeSkeletalAnimation::resolveSkeletalAnimation()
{
	// I'm at a leaf animation, so return NULL.
	return 0;
}

// ======================================================================
// class KeyframeSkeletalAnimation: private member functions
// ======================================================================

KeyframeSkeletalAnimation::KeyframeSkeletalAnimation(const KeyframeSkeletalAnimationTemplate *skeletalAnimationTemplate, const AnimationEnvironment &animationEnvironment, const TransformNameMap &skeletonTransformNameMap) :
	SkeletalAnimation(skeletalAnimationTemplate),
	m_currentFrameNumber(0.0f),
	m_previousFrameNumber(0.0f),
	m_currentCycleTime(0.0f),
	m_cyclePeriod(1.0f),
	m_playbackFramesPerSecond(30.0f),
	m_ooPlaybackFramesPerSecond(1.0f / 30.0f),
	m_frameCount(static_cast<float>(NON_NULL(skeletalAnimationTemplate)->getFrameCount())),
	m_rotationOperations(static_cast<QuaternionOperationVector::size_type>(skeletonTransformNameMap.getTransformCount())),
	m_translationOperations(static_cast<VectorOperationVector::size_type>(skeletonTransformNameMap.getTransformCount())),
	m_rotationStartKeyIndex(0),
	m_translationStartKeyIndex(0),
	m_scale(animationEnvironment.getConstFloat(AnimationEnvironmentNames::cms_appearanceScale))
{
	DEBUG_FATAL(!ms_installed, ("KeyframeSkeletalAnimation not installed"));
	NOT_NULL(skeletalAnimationTemplate);

	KeyframeSkeletalAnimation::setPlaybackFramesPerSecond(KeyframeSkeletalAnimation::getRecordedFramesPerSecond());

	//-- Construct the operations.

	// Reserve appropriate size.
	const int transformCount = skeletonTransformNameMap.getTransformCount();

	// Lookup and create operations per transform in the skeleton (i.e. in the associated .skt).
	for (int i = 0; i < transformCount; ++i)
	{
		QuaternionOperation *& currentRotationOperation    = m_rotationOperations[static_cast<QuaternionOperationVector::size_type>(i)];
		VectorOperation     *& currentTranslationOperation = m_translationOperations[static_cast<VectorOperationVector::size_type>(i)];

		//-- Lookup the transform data in the animation file for the given .skt transform.
		const int animationTransformIndex = skeletalAnimationTemplate->getTransformIndex(skeletonTransformNameMap.getTransformName(i));
		if (animationTransformIndex < 0)
		{
			// This skeleton transform is not present in this animation file.  In this case the animation returns static zero values.
			currentRotationOperation    = new StaticQuaternionOperation(Quaternion::identity);
			currentTranslationOperation = new StaticVectorOperation(Vector::zero);
		}
		else
		{
			const KeyframeSkeletalAnimationTemplate::TransformInfo &transformInfo = skeletalAnimationTemplate->getTransformInfo(animationTransformIndex);

			//-- Create the rotation operation.
			// Determine proper operation to create based on animation/static properties of this transform
			if (skeletalAnimationTemplate->hasAnimatedRotation(transformInfo))
			{
				// Create a KeyframeRotationOperation
				const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &keyDataVector = skeletalAnimationTemplate->getAnimatedRotationData(transformInfo);
				currentRotationOperation = new KeyframeRotationOperation(keyDataVector);
			}
			else
			{
				// Create a static rotation operation
				const Quaternion &staticRotation = skeletalAnimationTemplate->getStaticRotation(transformInfo);
				currentRotationOperation = new StaticQuaternionOperation(staticRotation);
			}

			//-- Create the translation operation.
			// Determine proper operation to create based on animation/static properties of this transform
			unsigned creatorIndex;

			if (skeletalAnimationTemplate->animatesTranslationX(transformInfo))
				creatorIndex = 0x01;
			else
				creatorIndex = 0;

			if (skeletalAnimationTemplate->animatesTranslationY(transformInfo))
				creatorIndex |= 0x02;

			if (skeletalAnimationTemplate->animatesTranslationZ(transformInfo))
				creatorIndex |= 0x04;

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(creatorIndex), 8);

			//-- Create the operation.
			currentTranslationOperation = (*ms_translationCreators[creatorIndex])(this, &transformInfo); //lint !e826 // (Info -- Suspicious pointer-to-pointer conversion (area too small)) // Lint is confused.  This is a pointer assignment.
		}
	}
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimation::~KeyframeSkeletalAnimation()
{
	std::for_each(m_rotationOperations.begin(), m_rotationOperations.end(), PointerDeleter());
	std::for_each(m_translationOperations.begin(), m_translationOperations.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimation::validateInvariantsWarn() const
{
	//-- Ensure previous frame <= current frame.  If not, then the controller
	//   did not leave the animation in a good state.  The controller should always
	//   break animation crossing the loop threshold into (at least) two separate sets of alters.
	if (!(m_previousFrameNumber <= m_currentFrameNumber))
	{
		DEBUG_WARNING(true, ("previous frame number > current frame number, class invariant error. [%g/%g].", m_previousFrameNumber, m_currentFrameNumber));
		return false;
	}

	//-- Ensure previous frame is in valid range.
	if ((m_previousFrameNumber < 0.0f) || (m_previousFrameNumber > m_frameCount))
	{
		DEBUG_WARNING(true, ("previous frame [%g] out of valid range [0 .. %g].", m_previousFrameNumber, m_frameCount));
		return false;
	}

	//-- Ensure current frame is in valid range.
	if ((m_currentFrameNumber < 0.0f) || (m_currentFrameNumber > m_frameCount))
	{
		DEBUG_WARNING(true, ("current frame [%g] out of valid range [0 .. %g].", m_currentFrameNumber, m_frameCount));
		return false;
	}

	//-- Ensure there's an animation template.
	const KeyframeSkeletalAnimationTemplate *const sat = getOurTemplate();
	if (!sat)
	{
		DEBUG_WARNING(true, ("animation has NULL template."));
		return false;
	}

	//-- All tested invariants are fine.
	return true;
}

// ======================================================================
