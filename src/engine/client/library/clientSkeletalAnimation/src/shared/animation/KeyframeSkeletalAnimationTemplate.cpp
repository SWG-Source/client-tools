// ======================================================================
//
// KeyframeSkeletalAnimationTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/KeyframeSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <algorithm>
#include <limits>
#include <set>

// ======================================================================
// Lint supression
// ======================================================================

//lint -e18 // redeclared templates are conflicting // don't see it.

namespace KeyframeSkeletalAnimationTemplateNamespace
{
	const Tag TAG_AROT = TAG(A,R,O,T);
	const Tag TAG_ATRN = TAG(A,T,R,N);
	const Tag TAG_CHNL = TAG(C,H,N,L);
	const Tag TAG_KFAT = TAG(K,F,A,T);
	const Tag TAG_LOCR = TAG(L,O,C,R);
	const Tag TAG_LOCT = TAG(L,O,C,T);
	const Tag TAG_MESG = TAG(M,E,S,G);
	const Tag TAG_MSGS = TAG(M,S,G,S);
	const Tag TAG_QCHN = TAG(Q,C,H,N);
	const Tag TAG_SROT = TAG(S,R,O,T);
	const Tag TAG_STRN = TAG(S,T,R,N);
	const Tag TAG_XFIN = TAG(X,F,I,N);
	const Tag TAG_XFRM = TAG(X,F,R,M);

	bool s_rotationFix = true;
	float const s_rotationKeyEpsilon = 1.0f - (std::numeric_limits<float>::epsilon() * 2.0f);
	
	bool s_translationFix = true;
	float s_translationFixEpsilon = std::numeric_limits<float>::epsilon() * 2.0f;
	
	bool shouldCompressKeyFrame(float const previousValue, float const nextValue)
	{
		return !WithinEpsilonInclusive(previousValue, nextValue, s_translationFixEpsilon);
	}

}

using namespace KeyframeSkeletalAnimationTemplateNamespace;

// ======================================================================
// enclosed member declarations
// ======================================================================

// @todo revisit access levels on TransformInfo (why is everything private?  legacy from previously being an exposed class?)

class KeyframeSkeletalAnimationTemplate::TransformInfo
{
friend class KeyframeSkeletalAnimationTemplate;

	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	struct LessNameComparator
	{
		bool operator ()(const TransformInfo *lhs, const TransformInfo *rhs) const;
		bool operator ()(const CrcString *lhs, const TransformInfo *rhs) const;
		bool operator ()(const TransformInfo *lhs, const CrcString *rhs) const;
	};

public:

	CrcString const &getTransformName() const;

private:

	TransformInfo(
		const char *name, 
		bool        hasAnimatedRotation, 
		int         rotationChannelIndex, 
		uint        translationMask, 
		int         xTranslationChannelIndex, 
		int         yTranslationChannelIndex, 
		int         zTranslationChannelIndex);

private:

	PersistentCrcString  m_name;

	bool            m_hasAnimatedRotation;
	int             m_rotationChannelIndex;

	uint            m_translationMask;
	int             m_xTranslationChannelIndex;
	int             m_yTranslationChannelIndex;
	int             m_zTranslationChannelIndex;

private:

	// disabled
	TransformInfo();
	TransformInfo(TransformInfo const & rhs);
	TransformInfo & operator=(TransformInfo const & rhs);
};

// ======================================================================

class KeyframeSkeletalAnimationTemplate::TranslationChannel
{
public:

	TranslationChannel();

	const RealKeyDataVector &getKeyDataVector() const;

	void load_0002(Iff &iff);
	void load_0003(Iff &iff);

private:

	RealKeyDataVector  m_keyDataVector;

};

// ======================================================================

class KeyframeSkeletalAnimationTemplate::RotationChannel
{
public:

	RotationChannel();

	const QuaternionKeyDataVector &getKeyDataVector() const;
	QuaternionKeyDataVector       &getKeyDataVector();

	void load_0003(Iff &iff);

private:

	QuaternionKeyDataVector  m_keyDataVector;
};

// ======================================================================

class KeyframeSkeletalAnimationTemplate::Message
{
public:

	static Message *createFromIff_0002(Iff &iff);
	static Message *createFromIff_0003(Iff &iff);

public:

	const CrcLowerString &getName() const;
	const IntVector      &getSignaledFrameNumbers() const;

private:

	Message();

	void  load_0002(Iff &iff);
	void  load_0003(Iff &iff);

private:

	CrcLowerString  m_name;
	IntVector       m_signaledFrameNumbers;

};

// ----------------------------------------------------------------------

class KeyframeSkeletalAnimationTemplate::SignaledMessageReporter
{
public:

	SignaledMessageReporter(IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers, int beginFrameNumber, int endFrameNumber);

	void operator()(const Message *message);

private:

	// disabled
	SignaledMessageReporter();
	SignaledMessageReporter &operator =(const SignaledMessageReporter&); //lint -esym(754, SignaledMessageReporter::operator=) // not referenced // can't be auto-generated

private:

	int          m_messageIndex;

	IntVector   &m_signaledMessageIndices;
	FloatVector &m_signaledMessageFrameNumbers;
	int          m_beginFrameNumber;
	int          m_endFrameNumber;

};

// ======================================================================
// static member definitions
// ======================================================================

bool                                                  KeyframeSkeletalAnimationTemplate::ms_installed;
MemoryBlockManager                     *KeyframeSkeletalAnimationTemplate::ms_memoryBlockManager;
KeyframeSkeletalAnimationTemplate::CrcLowerStringSet *KeyframeSkeletalAnimationTemplate::ms_inefficientFormatNameSet;

// ======================================================================
// struct KeyframeSkeletalAnimationTemplate::RealKeyData
// ======================================================================

KeyframeSkeletalAnimationTemplate::RealKeyData::RealKeyData() :
m_frameNumber(0.0f),
m_keyValue(0.0f),
m_oneOverDistanceToNextKeyframe(1.0f)
{
}

KeyframeSkeletalAnimationTemplate::RealKeyData::RealKeyData(float frameNumber, float keyValue) :
	m_frameNumber(frameNumber),
	m_keyValue(keyValue),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// struct KeyframeSkeletalAnimationTemplate::QuaternionKeyData
// ======================================================================
KeyframeSkeletalAnimationTemplate::QuaternionKeyData::QuaternionKeyData() :
m_frameNumber(0.0f),
m_rotation(),
m_oneOverDistanceToNextKeyframe(1.0f)
{
}

KeyframeSkeletalAnimationTemplate::QuaternionKeyData::QuaternionKeyData(float frameNumber, const Quaternion &rotation) :
	m_frameNumber(frameNumber),
	m_rotation(rotation),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// struct KeyframeSkeletalAnimationTemplate::VectorKeyData
// ======================================================================

KeyframeSkeletalAnimationTemplate::VectorKeyData::VectorKeyData(float frameNumber, const Vector &vector) :
	m_frameNumber(frameNumber),
	m_vector(vector),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplate::TransformInfo
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(KeyframeSkeletalAnimationTemplate::TransformInfo, true, 0, 0, 0);

// ======================================================================

inline CrcString const &KeyframeSkeletalAnimationTemplate::TransformInfo::getTransformName() const
{
	return m_name;
}

// ======================================================================

inline KeyframeSkeletalAnimationTemplate::TransformInfo::TransformInfo(
	const char *name, 
	bool        newHasAnimatedRotation, 
	int         rotationChannelIndex, 
	uint        translationMask, 
	int         xTranslationChannelIndex, 
	int         yTranslationChannelIndex, 
	int         zTranslationChannelIndex
	) :
	m_name(name, false),
	m_hasAnimatedRotation(newHasAnimatedRotation),
	m_rotationChannelIndex(rotationChannelIndex),
	m_translationMask(translationMask),
	m_xTranslationChannelIndex(xTranslationChannelIndex),
	m_yTranslationChannelIndex(yTranslationChannelIndex),
	m_zTranslationChannelIndex(zTranslationChannelIndex)
{
}

// ======================================================================
// struct KeyframeSkeletalAnimationTemplate::TransformInfo::LessNameComparator
// ======================================================================

bool KeyframeSkeletalAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const TransformInfo *lhs, const TransformInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getTransformName() < rhs->getTransformName();
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const CrcString *lhs, const TransformInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return *lhs < rhs->getTransformName();
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const TransformInfo *lhs, const CrcString *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getTransformName() < *rhs;
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplate::Message
// ======================================================================

inline void KeyframeSkeletalAnimationTemplate::Message::load_0003(Iff &iff)
{
	// no format change
	load_0002(iff);
}

// ======================================================================

KeyframeSkeletalAnimationTemplate::Message *KeyframeSkeletalAnimationTemplate::Message::createFromIff_0002(Iff &iff)
{
	Message *const message = new Message();
	message->load_0002(iff);

	return message;
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplate::Message *KeyframeSkeletalAnimationTemplate::Message::createFromIff_0003(Iff &iff)
{
	Message *const message = new Message();
	message->load_0003(iff);

	return message;
}

// ======================================================================

const CrcLowerString &KeyframeSkeletalAnimationTemplate::Message::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::IntVector &KeyframeSkeletalAnimationTemplate::Message::getSignaledFrameNumbers() const
{
	return m_signaledFrameNumbers;
}

// ======================================================================

KeyframeSkeletalAnimationTemplate::Message::Message()
:	m_name(""),
	m_signaledFrameNumbers()
{
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::Message::load_0002(Iff &iff)
{
	iff.enterChunk(TAG_MESG);

		// load # frames during which this message is signaled
		const int signalCount = static_cast<int>(iff.read_int16());
		DEBUG_FATAL(signalCount < 1, ("bad signal count %d", signalCount));

		m_signaledFrameNumbers.reserve(static_cast<size_t>(signalCount));

		// load signal name
		char name[1024];
		
		iff.read_string(name, sizeof(name) - 1);
		m_name.setString(name);

		// @todo remove the load code that handles catching 3 or more frames of the same signaled message.
		// load signaled frames
		int  lastTriggerFrameNumber = std::numeric_limits<int>::min();
		int  contiguousCount        = 0;
		bool signaledWarning        = false;

		for (int i = 0; i < signalCount; ++i)
		{
			//-- Get message frame number.
			int  const triggerFrameNumber = static_cast<int>(iff.read_int16());

			//-- Determine if frame number is contiguous with previous frame number.
			bool const isContiguous = (triggerFrameNumber == (lastTriggerFrameNumber + 1));
			if (isContiguous)
				++contiguousCount;
			else
				contiguousCount = 0;

			//-- If we have less than 3 in a row, keep the message.  Otherwise discard.
			if (contiguousCount < 2)
				m_signaledFrameNumbers.push_back(triggerFrameNumber);
			else
			{
				DEBUG_WARNING(!signaledWarning, ("animation file [%s] triggered message [%s] three or more frames in a row, fix animation.", iff.getFileName(), name));
				signaledWarning = true;

				if (contiguousCount == 2)
				{
					// Ditch the repeat entry (the second one) since that likely wasn't meant.
					m_signaledFrameNumbers.pop_back();
				}
			}

			//-- Keep track of previous triggered frame number.
			lastTriggerFrameNumber = triggerFrameNumber;
		}

	iff.exitChunk(TAG_MESG);
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplate::SignaledMessageReporter
// ======================================================================

KeyframeSkeletalAnimationTemplate::SignaledMessageReporter::SignaledMessageReporter(IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers, int beginFrameNumber, int endFrameNumber):
	m_messageIndex(0),
	m_signaledMessageIndices(signaledMessageIndices),
	m_signaledMessageFrameNumbers(signaledMessageFrameNumbers),
	m_beginFrameNumber(beginFrameNumber),
	m_endFrameNumber(endFrameNumber)
{
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::SignaledMessageReporter::operator()(const Message *message)
{
	NOT_NULL(message);

	//-- find all the frames that this message is signaled between the begin and end scan frame number.
	//   This code assumes the signaled message indices is sorted in ascending order.
	const IntVector &signaledFrameNumbers = message->getSignaledFrameNumbers();

	// find the first signaled frame number entry that is >= begin scanned frames
	const IntVector::const_iterator lowerBoundIt = std::lower_bound(signaledFrameNumbers.begin(), signaledFrameNumbers.end(), m_beginFrameNumber);

	// find the last signaled frame number entry that is < end scanned frames.
	// subtract one from the target end frame number since upper_bound will return an iterator one past
	// where the specified number does or should exist.  Our goal is to get the distance between 
	// lowerBoundIt and upperBoundIt to equal the number of frames signaled between range [m_beginFrameNumber, m_endFrameNumber).
	const IntVector::const_iterator upperBoundIt = std::upper_bound(signaledFrameNumbers.begin(), signaledFrameNumbers.end(), m_endFrameNumber - 1);

	// add a signaled entry for this message index once for each time it was signaled between the begin and end scan frames.
	const int entryCount = std::distance(lowerBoundIt, upperBoundIt);
	if (entryCount > 0)
	{
		m_signaledMessageIndices.insert(m_signaledMessageIndices.end(), static_cast<IntVector::size_type>(entryCount), m_messageIndex);

		for (IntVector::const_iterator it = lowerBoundIt; it != upperBoundIt; ++it)
		{
			float const frameNumber = static_cast<float>(*it);
			m_signaledMessageFrameNumbers.push_back(frameNumber);
		}
	}

	//-- move on to next message index
	++m_messageIndex;
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplate::RotationChannel
// ======================================================================

KeyframeSkeletalAnimationTemplate::RotationChannel::RotationChannel() :
	m_keyDataVector()
{
}

// ----------------------------------------------------------------------

inline const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &KeyframeSkeletalAnimationTemplate::RotationChannel::getKeyDataVector() const
{
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

inline KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &KeyframeSkeletalAnimationTemplate::RotationChannel::getKeyDataVector()
{
	// @todo: remove when load support for 0002 animation is removed.
	//lint -esym(1536, RotationChannel::m_keyDataVector) // exposing low-access member // that's okay, that's the whole point of this class
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::RotationChannel::load_0003(Iff &iff)
{
	iff.enterChunk(TAG_QCHN);
	{
		//-- get key count
		int const keyCount = static_cast<int>(iff.read_int32());

		//-- reserve appropriate amount of space
		m_keyDataVector.reserve(static_cast<size_t>(keyCount));

		// load data
		{
			Quaternion lastRotation;

			for (int i = 0; i < keyCount; ++i)
			{
				float const frameNumber = static_cast<real>(iff.read_int32());
				Quaternion const rotation = iff.read_floatQuaternion();
				
				if (!s_rotationFix || i == 0 || i == (keyCount - 1) || rotation.dot(lastRotation) < s_rotationKeyEpsilon)
				{
					m_keyDataVector.push_back(QuaternionKeyData(frameNumber, rotation));
					lastRotation = rotation;
				}
			}
		}

		m_keyDataVector.resize(m_keyDataVector.size());

		//-- calculate one over distance between each keyframe and next
		calculateQuaternionDistanceToNextFrame(m_keyDataVector);
	}
	iff.exitChunk(TAG_QCHN);
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplate::TranslationChannel
// ======================================================================

KeyframeSkeletalAnimationTemplate::TranslationChannel::TranslationChannel() :
	m_keyDataVector()
{
}

// ----------------------------------------------------------------------

inline const KeyframeSkeletalAnimationTemplate::RealKeyDataVector &KeyframeSkeletalAnimationTemplate::TranslationChannel::getKeyDataVector() const
{
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::TranslationChannel::load_0002(Iff &iff)
{
	iff.enterChunk(TAG_CHNL);
	{
		//-- get key count
		int const keyCount = static_cast<int>(iff.read_int32());

		//-- reserve appropriate amount of space
		m_keyDataVector.reserve(static_cast<size_t>(keyCount));

		// load data
		{
			float lastValue = std::numeric_limits<float>::max();

			for (int i = 0; i < keyCount; ++i)
			{
				float const frameNumber = static_cast<real>(iff.read_int32());
				float const value = iff.read_float();

				if (!s_translationFix || ((i == 0) || (i == (keyCount - 1)) || shouldCompressKeyFrame(lastValue, value)))
				{
					m_keyDataVector.push_back(RealKeyData(frameNumber, value));
					lastValue = value;
				}
			}
		}

		// Resize array.
		m_keyDataVector.resize(m_keyDataVector.size());

		//-- calculate one over distance between each keyframe and next
		calculateFloatDistanceToNextFrame(m_keyDataVector);
	}
	iff.exitChunk(TAG_CHNL);
}

// ----------------------------------------------------------------------

inline void KeyframeSkeletalAnimationTemplate::TranslationChannel::load_0003(Iff &iff)
{
	load_0002(iff);
}

// ======================================================================
// KeyframeSkeletalAnimationTemplate public static member functions
// ======================================================================

void KeyframeSkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("KeyframeSkeletalAnimationTemplate already installed"));

	KeyframeSkeletalAnimationTemplate::TransformInfo::install();

	const bool success = SkeletalAnimationTemplateList::registerCreateFunction(TAG_KFAT, create);
	DEBUG_FATAL(!success, ("failed to register KeyframeSkeletalAnimationTemplate"));
	UNREF(success);

	ms_memoryBlockManager       = new MemoryBlockManager("KeyframeSkeletalAnimationTemplate", true, sizeof(KeyframeSkeletalAnimationTemplate), 0, 0, 0);
	ms_inefficientFormatNameSet = new CrcLowerStringSet();

	ms_installed = true;
	ExitChain::add(remove, "KeyframeSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void *KeyframeSkeletalAnimationTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("KeyframeSkeletalAnimationTemplate not installed"));
	DEBUG_FATAL(size != sizeof(KeyframeSkeletalAnimationTemplate), ("derived classes not supported by this operator new"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::operator delete(void *data)
{
	ms_memoryBlockManager->free(data);
}

// ----------------------------------------------------------------------

Quaternion KeyframeSkeletalAnimationTemplate::computeQuaternionFromKeys(const QuaternionKeyDataVector &keyDataVector, float frameTime, int &startKeyIndex)
{
	//-- determine the lower and upper keyframe index
	const int keyCount = static_cast<int>(keyDataVector.size());

	//-- find closest keyframe occurring on or before animation frame number
	int  lowerKeyframeIndex;
	{
		// check if we should try to use our keyframe index helper
		if (keyDataVector[static_cast<size_t>(startKeyIndex)].m_frameNumber <= frameTime)
			lowerKeyframeIndex = startKeyIndex;
		else
			lowerKeyframeIndex = 0;

		for (; lowerKeyframeIndex < keyCount - 1; ++lowerKeyframeIndex)
		{
			if (keyDataVector[static_cast<size_t>(lowerKeyframeIndex + 1)].m_frameNumber > frameTime)
				break;
		}

		startKeyIndex = lowerKeyframeIndex;
	}

	//-- get frame distance to next key
	const float oneOverFrameDistance = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		return keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation;
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, frameTime, keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float       upperKeyframeWeight   = (frameTime - keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber) * oneOverFrameDistance;
		const Quaternion &upperKeyframeRotation = keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_rotation;
		const Quaternion &lowerKeyframeRotation = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation;

		//-- return the interpolation between the rotations based on weighting
		return lowerKeyframeRotation.slerp(upperKeyframeRotation, upperKeyframeWeight);
	}
}

// ----------------------------------------------------------------------

Vector KeyframeSkeletalAnimationTemplate::computeVectorFromKeys(const VectorKeyDataVector &keyDataVector, float frameTime, int &startKeyIndex)
{
	const int keyCount = static_cast<int>(keyDataVector.size());

	//-- find closest keyframe occurring on or before animation frame number
	int lowerKeyframeIndex;
	{
		// check if we should try to use our keyframe index helper
		if (keyDataVector[static_cast<size_t>(startKeyIndex)].m_frameNumber <= frameTime)
			lowerKeyframeIndex = startKeyIndex;
		else
			lowerKeyframeIndex = 0;

		for (; lowerKeyframeIndex < keyCount - 1; ++lowerKeyframeIndex)
		{
			if (keyDataVector[static_cast<size_t>(lowerKeyframeIndex + 1)].m_frameNumber > frameTime)
				break;
		}

		startKeyIndex = lowerKeyframeIndex;
	}

	//-- get frame distance to next key
	const real oneOverFrameDistance = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		return keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_vector;
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, frameTime, keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float lowerKeyframeWeight = (keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber - frameTime) * oneOverFrameDistance;
		return (lowerKeyframeWeight * keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_vector) + ((1.0f - lowerKeyframeWeight) * keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_vector);
	}
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::calculateVectorDistanceToNextFrame(VectorKeyDataVector &keyDataVector)
{
	// calculate one over distance between each keyframe and next
	const VectorKeyDataVector::size_type keyCount = keyDataVector.size();

	for (VectorKeyDataVector::size_type i = 0; i < keyCount - 1; ++i)
	{
		VectorKeyData &keyData = keyDataVector[i];
		keyData.m_oneOverDistanceToNextKeyframe = 1.0f / (keyDataVector[i + 1].m_frameNumber - keyData.m_frameNumber);
	}

	keyDataVector[keyCount - 1].m_oneOverDistanceToNextKeyframe = 0.0f;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::calculateQuaternionDistanceToNextFrame(QuaternionKeyDataVector &keyDataVector)
{
	// calculate one over distance between each keyframe and next
	const QuaternionKeyDataVector::size_type keyCount = keyDataVector.size();

	for (QuaternionKeyDataVector::size_type i = 0; i < keyCount - 1; ++i)
	{
		QuaternionKeyData &keyData = keyDataVector[i];
		keyData.m_oneOverDistanceToNextKeyframe = 1.0f / (keyDataVector[i + 1].m_frameNumber - keyData.m_frameNumber);
	}

	keyDataVector[keyCount - 1].m_oneOverDistanceToNextKeyframe = 0.0f;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::calculateFloatDistanceToNextFrame(RealKeyDataVector &keyDataVector)
{
	// calculate one over distance between each keyframe and next
	const RealKeyDataVector::size_type keyCount = keyDataVector.size();

	for (RealKeyDataVector::size_type i = 0; i < keyCount - 1; ++i)
	{
		RealKeyData &keyData = keyDataVector[i];
		keyData.m_oneOverDistanceToNextKeyframe = 1.0f / (keyDataVector[i + 1].m_frameNumber - keyData.m_frameNumber);
	}

	keyDataVector[keyCount - 1].m_oneOverDistanceToNextKeyframe = 0.0f;
}


// ======================================================================
// KeyframeSkeletalAnimationTemplate public member functions
// ======================================================================

SkeletalAnimation *KeyframeSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	// create it
	KeyframeSkeletalAnimation *skeletalAnimation = new KeyframeSkeletalAnimation(this, animationEnvironment, transformNameMap);
	NOT_NULL(skeletalAnimation);

	// get initial reference for caller
	skeletalAnimation->fetch();

	return skeletalAnimation;
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimationTemplate::getTransformCount() const
{
	return static_cast<int>(m_transformInfoVector.size());
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::TransformInfo &KeyframeSkeletalAnimationTemplate::getTransformInfo(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());
	return *m_transformInfoVector[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimationTemplate::getTransformIndex(CrcString const &transformName) const
{
	//-- TransformInfo is stored in CrcLowerString-order, so we can do a binary search lookup.
	const std::pair<TransformInfoVector::const_iterator, TransformInfoVector::const_iterator> result = std::equal_range(m_transformInfoVector.begin(), m_transformInfoVector.end(), &transformName, TransformInfo::LessNameComparator()); //lint !e64 // type mismatch // a confused lint.
	if (result.first == result.second)
	{
		//-- Transform not present.
		return -1;
	}
	else
	{
		return static_cast<int>(std::distance(m_transformInfoVector.begin(), result.first));
	}
}

// ----------------------------------------------------------------------

CrcString const &KeyframeSkeletalAnimationTemplate::getName(const TransformInfo &transformInfo) const
{
	return transformInfo.m_name;
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::hasAnimatedRotation(const TransformInfo &transformInfo) const
{
	return transformInfo.m_hasAnimatedRotation;
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector &KeyframeSkeletalAnimationTemplate::getAnimatedRotationData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!transformInfo.m_hasAnimatedRotation, ("Transform [%s] doesn't animate rotation", transformInfo.m_name.getString()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_rotationChannelIndex, static_cast<int>(m_rotationChannels.size()));

	return m_rotationChannels[static_cast<size_t>(transformInfo.m_rotationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

const Quaternion &KeyframeSkeletalAnimationTemplate::getStaticRotation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(transformInfo.m_hasAnimatedRotation, ("Transform [%s] doesn't have static rotation.", transformInfo.m_name.getString()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_rotationChannelIndex, static_cast<int>(m_staticRotations.size()));

	return m_staticRotations[static_cast<size_t>(transformInfo.m_rotationChannelIndex)];
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::animatesTranslation(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_translationMask) != 0;
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::animatesTranslationX(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_xTranslation) != 0;
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::animatesTranslationY(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_yTranslation) != 0;
}

// ----------------------------------------------------------------------

bool KeyframeSkeletalAnimationTemplate::animatesTranslationZ(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_zTranslation) != 0;
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::RealKeyDataVector &KeyframeSkeletalAnimationTemplate::getXTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationX(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_xTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_xTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::RealKeyDataVector &KeyframeSkeletalAnimationTemplate::getYTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationY(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_yTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_yTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

const KeyframeSkeletalAnimationTemplate::RealKeyDataVector &KeyframeSkeletalAnimationTemplate::getZTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationZ(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_zTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_zTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimationTemplate::getStaticXTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationX(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_xTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_xTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimationTemplate::getStaticYTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationY(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_yTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_yTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

float KeyframeSkeletalAnimationTemplate::getStaticZTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationZ(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_zTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_zTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

int KeyframeSkeletalAnimationTemplate::getMessageCount() const
{
	if (m_messages)
		return static_cast<int>(m_messages->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

const CrcLowerString &KeyframeSkeletalAnimationTemplate::getMessageName(int index) const
{
	NOT_NULL(m_messages);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_messages->size()));

	return (*m_messages)[static_cast<size_t>(index)]->getName();
}

// ----------------------------------------------------------------------
/**
 * Return the index of each frame signaled during the the specified
 * time interval.
 *
 * The time interval considered includes the beginFrameNumber but does not
 * include the endFrameInterval (just like STL containers).
 *
 * If a message index is signaled multiple times during the range, the index
 * will appear multiple times in the return list.
 *
 * There is no particular ordering of message indices that are signaled
 * during the scan range.  The caller can make no assumptions about relative ordering
 * of signals based on placement within the array.
 *
 * @param beginFrameNumber        the 0-based frame number to start scanning for
 *                                signaled messages.  This frame number is inclusive
 *                                to the scanned range.
 * @param endFrameNumber          the 0-based frame number to stop scanning for
 *                                signaled messages.  This frame number is exclusive
 *                                to the scanned range.
 * @param signaledMessageIndices  indices for all messages signaled during the
 *                                specified range are returned in this parameter.
 *                                The vector is not clear()ed, so clear it prior
 *                                to the call if you don't want append semantics.
 *                                
 */

void KeyframeSkeletalAnimationTemplate::getSignaledMessages(int beginFrameNumber, int endFrameNumber, IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers) const
{
	if (!m_messages)
		return;

#if 0
	//-- handle case where we're collecting over no time
	if (endFrameNumber <= beginFrameNumber)
		return;
#endif

	std::for_each(m_messages->begin(), m_messages->end(), SignaledMessageReporter(signaledMessageIndices, signaledMessageFrameNumbers, beginFrameNumber, endFrameNumber));
}

// ======================================================================
// KeyframeSkeletalAnimationTemplate private static member functions
// ======================================================================

void KeyframeSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("KeyframeSkeletalAnimationTemplate not installed"));

	//-- report on inefficient format animations so they can be re-exported
	if (!ms_inefficientFormatNameSet->empty())
	{
		DEBUG_WARNING(true, ("[%u] animations loaded using an inefficient older version, please re-export:", ms_inefficientFormatNameSet->size()));

		const CrcLowerStringSet::iterator endIt = ms_inefficientFormatNameSet->end();
		for (CrcLowerStringSet::iterator it = ms_inefficientFormatNameSet->begin(); it != endIt; ++it)
		{
			DEBUG_WARNING(true, ("|- [%s]", it->getString()));
		}
	}
	delete ms_inefficientFormatNameSet;
	ms_inefficientFormatNameSet = 0;

	const bool success = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_KFAT);
	DEBUG_FATAL(!success, ("failed to deregister KeyframeSkeletalAnimationTemplate"));
	UNREF(success);

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *KeyframeSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	return new KeyframeSkeletalAnimationTemplate(name, iff);
}

// ======================================================================
// KeyframeSkeletalAnimationTemplate private member functions
// ======================================================================

KeyframeSkeletalAnimationTemplate::KeyframeSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff)
:	SkeletalAnimationTemplate(name),
	m_framesPerSecond(0),
	m_frameCount(0),
	m_transformInfoVector(),
	m_rotationChannels(),
	m_translationChannels(),
	m_staticRotations(),
	m_staticTranslations(),
	m_messages(0),
	m_locomotionRotationKeys(0),
	m_locomotionTranslationKeys(0),
	m_averageTranslationSpeed(0.0f),
	m_getLocomotionFunction(&KeyframeSkeletalAnimationTemplate::getLocomotionNone)
{
	iff.enterForm(TAG_KFAT);
	
		switch (iff.getCurrentName())
		{
			case TAG_0002:
				load_0002(iff);

				//-- this is an inefficient format, keep track so we can warn on exit
				IGNORE_RETURN(ms_inefficientFormatNameSet->insert(name));

				break;

			case TAG_0003:
				load_0003(iff);
				break;

			default:
			{
				char tagName[5];
				ConvertTagToString(iff.getCurrentName(), tagName);
				FATAL(true, ("unknown or unsupported KeyframeSkeletalAnimationTemplate version [%s]", tagName));
			}
		}

	iff.exitForm(TAG_KFAT);

	//-- determine which locomotion function to use based on locomotion data provided in data.
	if (m_locomotionRotationKeys && m_locomotionTranslationKeys)
		m_getLocomotionFunction = &KeyframeSkeletalAnimationTemplate::getLocomotionRotationTranslation;
	else if (m_locomotionRotationKeys)
		m_getLocomotionFunction = &KeyframeSkeletalAnimationTemplate::getLocomotionRotation;
	else if (m_locomotionTranslationKeys)
		m_getLocomotionFunction = &KeyframeSkeletalAnimationTemplate::getLocomotionTranslation;

	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogSktCreateDestroy(), ("KSAT: CREATE [%s].\n", SkeletalAnimationTemplate::getName().getString()));
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplate::~KeyframeSkeletalAnimationTemplate()
{
	if (m_messages)
	{
		std::for_each(m_messages->begin(), m_messages->end(), PointerDeleter());
		delete m_messages;
	}

	delete m_locomotionRotationKeys;
	delete m_locomotionTranslationKeys;

	std::for_each(m_transformInfoVector.begin(), m_transformInfoVector.end(), PointerDeleter());

	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogSktCreateDestroy(), ("KSAT: DESTROY [%s].\n", SkeletalAnimationTemplate::getName().getString()));
}

// ======================================================================
/**
 * This should go away when support for format 0002 goes away.
 */

struct KeyframeSkeletalAnimationTemplate::EvaluateRotationRequest
{
public:

	EvaluateRotationRequest(uint rotationMask, int xRotationChannelIndex, int yRotationChannelIndex, int zRotationChannelIndex);

	Quaternion evaluateRotation(int frameNumber, const TranslationChannelVector &animatedEulerRotationChannels, const FloatVector &staticEulerRotationChannels) const;

public:

	uint  m_rotationMask;
	int   m_xRotationChannelIndex;
	int   m_yRotationChannelIndex;
	int   m_zRotationChannelIndex;

private:

	float evaluateAnimatedRotation(int frameNumber, int channelIndex, const TranslationChannelVector &animatedEulerRotationChannels) const;

	// disabled
	EvaluateRotationRequest();

};

// ----------------------------------------------------------------------
/**
 * This should go away when support for format 0002 goes away.
 */

inline KeyframeSkeletalAnimationTemplate::EvaluateRotationRequest::EvaluateRotationRequest(uint rotationMask, int xRotationChannelIndex, int yRotationChannelIndex, int zRotationChannelIndex) :
	m_rotationMask(rotationMask),
	m_xRotationChannelIndex(xRotationChannelIndex),
	m_yRotationChannelIndex(yRotationChannelIndex),
	m_zRotationChannelIndex(zRotationChannelIndex)
{
}

// ----------------------------------------------------------------------
/**
 * This should go away when support for format 0002 goes away.
 */

float KeyframeSkeletalAnimationTemplate::EvaluateRotationRequest::evaluateAnimatedRotation(int frameNumber, int channelIndex, const TranslationChannelVector &animatedEulerRotationChannels) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, channelIndex, static_cast<int>(animatedEulerRotationChannels.size()));

	const RealKeyDataVector &keyDataVector = animatedEulerRotationChannels[static_cast<size_t>(channelIndex)].getKeyDataVector();

	const float animationFrameNumber = static_cast<float>(frameNumber);
	const int   keyCount             = static_cast<int>(keyDataVector.size());
	int         lowerKeyframeIndex   = 0;

	//-- find closest keyframe occurring on or before animation frame number
	for (; lowerKeyframeIndex < keyCount - 1; ++lowerKeyframeIndex)
	{
		if (keyDataVector[static_cast<size_t>(lowerKeyframeIndex + 1)].m_frameNumber > animationFrameNumber)
			break;
	}

	//-- get frame distance to next key
	const real oneOverFrameDistance = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		return keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_keyValue;
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, animationFrameNumber, keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float lowerKeyframeWeight = (keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber - animationFrameNumber) * oneOverFrameDistance;
		const float blendedValue        = (lowerKeyframeWeight * keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_keyValue) + ((1.0f - lowerKeyframeWeight) * keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_keyValue);

		return blendedValue;
	}
}

// ----------------------------------------------------------------------
/**
 * This should go away when support for format 0002 goes away.
 */

Quaternion KeyframeSkeletalAnimationTemplate::EvaluateRotationRequest::evaluateRotation(int frameNumber, const TranslationChannelVector &animatedEulerRotationChannels, const FloatVector &staticEulerRotationChannels) const
{
	const float angleX = (m_rotationMask & SATCCF_xRotation) == 0 ? staticEulerRotationChannels[static_cast<size_t>(m_xRotationChannelIndex)] : evaluateAnimatedRotation(frameNumber, m_xRotationChannelIndex, animatedEulerRotationChannels);
	const float angleY = (m_rotationMask & SATCCF_yRotation) == 0 ? staticEulerRotationChannels[static_cast<size_t>(m_yRotationChannelIndex)] : evaluateAnimatedRotation(frameNumber, m_yRotationChannelIndex, animatedEulerRotationChannels);
	const float angleZ = (m_rotationMask & SATCCF_zRotation) == 0 ? staticEulerRotationChannels[static_cast<size_t>(m_zRotationChannelIndex)] : evaluateAnimatedRotation(frameNumber, m_zRotationChannelIndex, animatedEulerRotationChannels);

	const Quaternion qx(angleX, Vector::unitX);
	const Quaternion qy(angleY, Vector::unitY);
	const Quaternion qz(angleZ, Vector::unitZ);

	return qz * (qy * qx);
}

// ----------------------------------------------------------------------
/**
 * This should go away when support for format 0002 goes away.
 */

void KeyframeSkeletalAnimationTemplate::buildQuaternionKeyframesFromEulers(const EvaluateRotationRequestVector &requestVector, int frameCount, const TranslationChannelVector &animatedEulerRotationChannels, const FloatVector &staticEulerRotationChannels, RotationChannelVector &rotationChannels)
{
	//-- prepare resulting vector
	rotationChannels.clear();
	rotationChannels.resize(requestVector.size());

	const double epsilon = 0.001;

	//-- handle each transform's rotation
	int requestIndex = 0;

	const EvaluateRotationRequestVector::const_iterator endIt = requestVector.end();
	for (EvaluateRotationRequestVector::const_iterator it = requestVector.begin(); it != endIt; ++it, ++requestIndex)
	{
		const EvaluateRotationRequest &request       = *it;
		QuaternionKeyDataVector       &keyDataVector = rotationChannels[static_cast<size_t>(requestIndex)].getKeyDataVector();

		//-- sanity check
		DEBUG_FATAL((request.m_rotationMask & SATCCF_rotationMask) == 0, ("this request for an animated rotation has no animated Euler channels.\n"));
	
		// handle first frame
		keyDataVector.push_back(QuaternionKeyData(0.0f, request.evaluateRotation(0, animatedEulerRotationChannels, staticEulerRotationChannels)));

		// handle intermediate frames
		for (int frameNumber = 1; frameNumber < frameCount; ++frameNumber)
		{
			// evaluate the frame's rotation
			const Quaternion  frameRotation = request.evaluateRotation(frameNumber, animatedEulerRotationChannels, staticEulerRotationChannels);

			// add keyframe if rotation has changed since last keyframe
			const Quaternion &lastFrameRotation = keyDataVector.back().m_rotation;
			if (
				(fabs(static_cast<double>(lastFrameRotation.w - frameRotation.w)) > epsilon) ||
				(fabs(static_cast<double>(lastFrameRotation.x - frameRotation.x)) > epsilon) ||
				(fabs(static_cast<double>(lastFrameRotation.y - frameRotation.y)) > epsilon) ||
				(fabs(static_cast<double>(lastFrameRotation.z - frameRotation.z)) > epsilon))
			{
				keyDataVector.push_back(QuaternionKeyData(static_cast<float>(frameNumber), frameRotation));
			}
		}

		// handle final frame
		keyDataVector.push_back(QuaternionKeyData(static_cast<float>(frameCount), request.evaluateRotation(frameCount, animatedEulerRotationChannels, staticEulerRotationChannels)));

		//-- fix up oneOverDistanceToNextFrame
		{
			const int keyCount = static_cast<int>(keyDataVector.size());

			for (int i = 0; i < keyCount - 1; ++i)
				keyDataVector[static_cast<size_t>(i)].m_oneOverDistanceToNextKeyframe = 1.0f / (keyDataVector[static_cast<size_t>(i) + 1].m_frameNumber - keyDataVector[static_cast<size_t>(i)].m_frameNumber);

			keyDataVector[static_cast<size_t>(keyCount - 1)].m_oneOverDistanceToNextKeyframe = 0.0f;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * This should go away when support for format 0002 goes away.
 */

void KeyframeSkeletalAnimationTemplate::buildStaticQuaternionsFromEulers(const EvaluateRotationRequestVector &staticRequestVector, const FloatVector &eulerStaticRotationChannels, QuaternionVector &staticRotations)
{
	//-- prepare resulting vector
	staticRotations.clear();
	staticRotations.reserve(staticRequestVector.size());

	//-- create the static rotations
	const EvaluateRotationRequestVector::const_iterator endIt = staticRequestVector.end();
	for (EvaluateRotationRequestVector::const_iterator it = staticRequestVector.begin(); it != endIt; ++it)
	{
		const EvaluateRotationRequest &request = *it;

		//-- sanity check
		DEBUG_FATAL((request.m_rotationMask & SATCCF_rotationMask) != 0, ("this request for static rotation has animation data.\n"));

		//-- get x, y and z components
		Quaternion qx(eulerStaticRotationChannels[static_cast<size_t>(request.m_xRotationChannelIndex)], Vector::unitX);
		Quaternion qy(eulerStaticRotationChannels[static_cast<size_t>(request.m_yRotationChannelIndex)], Vector::unitY);
		Quaternion qz(eulerStaticRotationChannels[static_cast<size_t>(request.m_zRotationChannelIndex)], Vector::unitZ);

		//-- rotations always assumed to be in time order of x first, then y, finally z
		staticRotations.push_back(qz * (qy * qx));
	}
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::load_0002(Iff &iff)
{
	EvaluateRotationRequestVector  animatedRequestVector;
	EvaluateRotationRequestVector  staticRequestVector;

	int transformInfoCount      = 0;
	int rotationChannelCount    = 0;
	int staticRotationCount     = 0;
	int translationChannelCount = 0;
	int staticTranslationCount  = 0;

	int newAnimatedRotationCount = 0;
	int newStaticRotationCount   = 0;

	iff.enterForm(TAG_0002);

		//-- load general animation information
		iff.enterChunk(TAG_INFO);
		{
			m_framesPerSecond       = iff.read_float();
			m_frameCount            = static_cast<int>(iff.read_int32());
			transformInfoCount      = static_cast<int>(iff.read_int32());

			rotationChannelCount    = static_cast<int>(iff.read_int32());
			staticRotationCount     = static_cast<int>(iff.read_int32());
			translationChannelCount = static_cast<int>(iff.read_int32());
			staticTranslationCount  = static_cast<int>(iff.read_int32());
		}
		iff.exitChunk(TAG_INFO);

		//-- reserve space for conversion
		animatedRequestVector.reserve(static_cast<size_t>(rotationChannelCount));
		staticRequestVector.reserve(static_cast<size_t>(staticRotationCount));

		//-- load transform info entries
		iff.enterForm(TAG_XFRM);
		{
			m_transformInfoVector.reserve(static_cast<size_t>(transformInfoCount));

			for (size_t i = 0; i < static_cast<size_t>(transformInfoCount); ++i)
			{
				iff.enterChunk(TAG_XFIN);
				{
					char nameBuffer[MAX_PATH];
					iff.read_string(nameBuffer, MAX_PATH-1);

					const uint rotationMask             = static_cast<uint>(iff.read_uint32());
					const int  xRotationChannelIndex    = static_cast<int>(iff.read_int32());
					const int  yRotationChannelIndex    = static_cast<int>(iff.read_int32());
					const int  zRotationChannelIndex    = static_cast<int>(iff.read_int32());

					// new rotation channel is animated if any of the Euler components are animated
					const bool newHasAnimatedRotation   = (rotationMask & SATCCF_rotationMask) != 0;
					const int  newRotationChannelIndex  = (newHasAnimatedRotation ? newAnimatedRotationCount++ : newStaticRotationCount++);

					if (newHasAnimatedRotation)
						animatedRequestVector.push_back(EvaluateRotationRequest(rotationMask, xRotationChannelIndex, yRotationChannelIndex, zRotationChannelIndex));
					else
						staticRequestVector.push_back(EvaluateRotationRequest(rotationMask, xRotationChannelIndex, yRotationChannelIndex, zRotationChannelIndex));

					const uint translationMask          = static_cast<uint>(iff.read_uint32());
					const int  xTranslationChannelIndex = static_cast<int>(iff.read_int32());
					const int  yTranslationChannelIndex = static_cast<int>(iff.read_int32());
					const int  zTranslationChannelIndex = static_cast<int>(iff.read_int32());

					m_transformInfoVector.push_back(new TransformInfo(
						nameBuffer,
						newHasAnimatedRotation,
						newRotationChannelIndex,
						translationMask,
						xTranslationChannelIndex,
						yTranslationChannelIndex,
						zTranslationChannelIndex));
				}
				iff.exitChunk(TAG_XFIN);
			}
		}
		iff.exitForm(TAG_XFRM);

		//-- handle rotation channels
		{		
			//-- handle animated rotation channels
			// load animated Euler rotation channels
			TranslationChannelVector  eulerAnimatedRotationChannels;

			const bool hasArotTag = iff.enterForm(TAG_AROT, true);
			DEBUG_FATAL(!hasArotTag && rotationChannelCount, ("rotationChannelCount [%d] with no data", rotationChannelCount));
			if (hasArotTag)
			{
				eulerAnimatedRotationChannels.resize(static_cast<size_t>(rotationChannelCount));

				for (size_t i = 0; i < static_cast<size_t>(rotationChannelCount); ++i)
					eulerAnimatedRotationChannels[i].load_0002(iff);

				iff.exitForm(TAG_AROT);
			}

			//-- load static rotation data
			FloatVector  eulerStaticRotationChannels;

			const bool hasSrotTag = iff.enterChunk(TAG_SROT, true);
			DEBUG_FATAL(!hasSrotTag && staticRotationCount, ("staticRotationCount [%d] with no data", staticRotationCount));
			if (hasSrotTag)
			{
				eulerStaticRotationChannels.reserve(static_cast<size_t>(staticRotationCount));
				
				for (size_t i = 0; i < static_cast<size_t>(staticRotationCount); ++i)
					eulerStaticRotationChannels.push_back(iff.read_float());

				iff.exitChunk(TAG_SROT);
			}

			// convert animated Euler rotations to keyframed Quaternion channels
			buildQuaternionKeyframesFromEulers(animatedRequestVector, m_frameCount, eulerAnimatedRotationChannels, eulerStaticRotationChannels, m_rotationChannels);

			// convert static Euler rotations to static Quaternion channels
			buildStaticQuaternionsFromEulers(staticRequestVector, eulerStaticRotationChannels, m_staticRotations);
		}

		//-- load animated translation channels
		const bool hasAtrnTag = iff.enterForm(TAG_ATRN, true);
		DEBUG_FATAL(!hasAtrnTag && translationChannelCount, ("translationChannelCount [%d] with no data", translationChannelCount));
		if (hasAtrnTag)
		{
			m_translationChannels.resize(static_cast<size_t>(translationChannelCount));

			for (size_t i = 0; i < static_cast<size_t>(translationChannelCount); ++i)
				m_translationChannels[i].load_0002(iff);

			iff.exitForm(TAG_ATRN);
		}

		//-- load static translation data
		const bool hasStrnTag = iff.enterChunk(TAG_STRN, true);
		DEBUG_FATAL(!hasStrnTag && staticTranslationCount, ("staticTranslationCount [%d] with no data", staticTranslationCount));
		if (hasStrnTag)
		{
			m_staticTranslations.reserve(static_cast<size_t>(staticTranslationCount));
			
			for (size_t i = 0; i < static_cast<size_t>(staticTranslationCount); ++i)
				m_staticTranslations.push_back(iff.read_float());

			iff.exitChunk(TAG_STRN);
		}

		//-- load animation messages
		if (iff.enterForm(TAG_MSGS, true))
		{
			// get # messages
			iff.enterChunk(TAG_INFO);

				const int messageCount = static_cast<int>(iff.read_int16());

			iff.exitChunk(TAG_INFO);

			// load the messages
			if (messageCount > 0)
			{
				m_messages = new MessageVector();
				m_messages->reserve(static_cast<size_t>(messageCount));

				for (int i = 0; i < messageCount; ++i)
					m_messages->push_back(Message::createFromIff_0002(iff));
			}

			iff.exitForm(TAG_MSGS);
		}

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::load_0003(Iff &iff)
{
	int transformInfoCount      = 0;
	int rotationChannelCount    = 0;
	int staticRotationCount     = 0;
	int translationChannelCount = 0;
	int staticTranslationCount  = 0;

	iff.enterForm(TAG_0003);

		//-- load general animation information
		iff.enterChunk(TAG_INFO);
		{
			m_framesPerSecond       = iff.read_float();
			m_frameCount            = static_cast<int>(iff.read_int32());
			transformInfoCount      = static_cast<int>(iff.read_int32());

			rotationChannelCount    = static_cast<int>(iff.read_int32());
			staticRotationCount     = static_cast<int>(iff.read_int32());
			translationChannelCount = static_cast<int>(iff.read_int32());
			staticTranslationCount  = static_cast<int>(iff.read_int32());
		}
		iff.exitChunk(TAG_INFO);

		//-- load transform info entries
		iff.enterForm(TAG_XFRM);
		{
			m_transformInfoVector.reserve(static_cast<size_t>(transformInfoCount));

			for (size_t i = 0; i < static_cast<size_t>(transformInfoCount); ++i)
			{
				iff.enterChunk(TAG_XFIN);
				{
					char nameBuffer[MAX_PATH];
					iff.read_string(nameBuffer, MAX_PATH-1);

					const bool newHasAnimatedRotations  = (iff.read_int8() == 0) ? false : true;
					const int  rotationChannelIndex     = static_cast<int>(iff.read_int32());

					const uint translationMask          = static_cast<uint>(iff.read_uint32());
					const int  xTranslationChannelIndex = static_cast<int>(iff.read_int32());
					const int  yTranslationChannelIndex = static_cast<int>(iff.read_int32());
					const int  zTranslationChannelIndex = static_cast<int>(iff.read_int32());

					m_transformInfoVector.push_back(new TransformInfo(
							nameBuffer,
							newHasAnimatedRotations,
							rotationChannelIndex,
							translationMask,
							xTranslationChannelIndex,
							yTranslationChannelIndex,
							zTranslationChannelIndex));
				}
				iff.exitChunk(TAG_XFIN);
			}
		}
		iff.exitForm(TAG_XFRM);
		

		//-- load animated rotation channels
		const bool hasArotTag = iff.enterForm(TAG_AROT, true);
		DEBUG_FATAL(!hasArotTag && rotationChannelCount, ("rotationChannelCount [%d] with no data", rotationChannelCount));
		if (hasArotTag)
		{
			m_rotationChannels.resize(static_cast<size_t>(rotationChannelCount));

			for (size_t i = 0; i < static_cast<size_t>(rotationChannelCount); ++i)
				m_rotationChannels[i].load_0003(iff);

			iff.exitForm(TAG_AROT);
		}

		//-- load static rotation data
		const bool hasSrotTag = iff.enterChunk(TAG_SROT, true);
		DEBUG_FATAL(!hasSrotTag && staticRotationCount, ("staticRotationCount [%d] with no data", staticRotationCount));
		if (hasSrotTag)
		{
			m_staticRotations.reserve(static_cast<size_t>(staticRotationCount));
			
			for (size_t i = 0; i < static_cast<size_t>(staticRotationCount); ++i)
				m_staticRotations.push_back(iff.read_floatQuaternion());

			iff.exitChunk(TAG_SROT);
		}

		//-- load animated translation channels
		const bool hasAtrnTag = iff.enterForm(TAG_ATRN, true);
		DEBUG_FATAL(!hasAtrnTag && translationChannelCount, ("translationChannelCount [%d] with no data", translationChannelCount));
		if (hasAtrnTag)
		{
			m_translationChannels.resize(static_cast<size_t>(translationChannelCount));

			for (size_t i = 0; i < static_cast<size_t>(translationChannelCount); ++i)
				m_translationChannels[i].load_0003(iff);

			iff.exitForm(TAG_ATRN);
		}

		//-- load static translation data
		const bool hasStrnTag = iff.enterChunk(TAG_STRN, true);
		DEBUG_FATAL(!hasStrnTag && staticTranslationCount, ("staticTranslationCount [%d] with no data", staticTranslationCount));
		if (hasStrnTag)
		{
			m_staticTranslations.reserve(static_cast<size_t>(staticTranslationCount));
			
			for (size_t i = 0; i < static_cast<size_t>(staticTranslationCount); ++i)
				m_staticTranslations.push_back(iff.read_float());

			iff.exitChunk(TAG_STRN);
		}

		//-- load animation messages
		if (iff.enterForm(TAG_MSGS, true))
		{
			// get # messages
			iff.enterChunk(TAG_INFO);

				const int messageCount = static_cast<int>(iff.read_int16());

			iff.exitChunk(TAG_INFO);

			// load the messages
			if (messageCount > 0)
			{
				if (!m_messages)
					m_messages = new MessageVector();
				m_messages->reserve(static_cast<size_t>(messageCount));

				for (int i = 0; i < messageCount; ++i)
					m_messages->push_back(Message::createFromIff_0003(iff));
			}

			iff.exitForm(TAG_MSGS);
		}

		//-- load locomotion translation data
		if (iff.enterChunk(TAG_LOCT, true))
		{
			//-- load average translational speed
			m_averageTranslationSpeed = iff.read_float();

			//-- load key count
			const int keyCount = static_cast<int>(iff.read_int16());
			
			//-- create key storage
			if (!m_locomotionTranslationKeys)
				m_locomotionTranslationKeys = new VectorKeyDataVector();
			else
				VectorKeyDataVector().swap(*m_locomotionTranslationKeys);

			m_locomotionTranslationKeys->reserve(static_cast<VectorKeyDataVector::size_type>(keyCount));

			//-- load key data
			for (int i = 0; i < keyCount; ++i)
			{
				const float  frameNumber = static_cast<float>(iff.read_int16());
				const Vector translation = iff.read_floatVector();

				m_locomotionTranslationKeys->push_back(VectorKeyData(frameNumber, translation));
				calculateVectorDistanceToNextFrame(*m_locomotionTranslationKeys);
			}

			iff.exitChunk(TAG_LOCT);
		}

		//-- load locomotion rotation data
		if (iff.enterChunk(TAG_LOCR, true))
		{
			//-- load key count
			const int keyCount = static_cast<int>(iff.read_int16());
			
			//-- create key storage
			if (!m_locomotionRotationKeys)
				m_locomotionRotationKeys = new QuaternionKeyDataVector();
			else
				QuaternionKeyDataVector().swap(*m_locomotionRotationKeys);

			m_locomotionRotationKeys->reserve(static_cast<VectorKeyDataVector::size_type>(keyCount));

			//-- load key data
			for (int i = 0; i < keyCount; ++i)
			{
				const float      frameNumber = static_cast<float>(iff.read_int16());
				const Quaternion rotation    = iff.read_floatQuaternion();

				m_locomotionRotationKeys->push_back(QuaternionKeyData(frameNumber, rotation));
				calculateQuaternionDistanceToNextFrame(*m_locomotionRotationKeys);
			}

			iff.exitChunk(TAG_LOCR);
		}

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::getLocomotionNone(float /* beginFrameTime */, float /* endFrameTime */, int & /* rotationStartKeyIndex */, int & /* translationStartKeyIndex */, Quaternion &rotation, Vector &translation) const
{
	//-- no rotation
	rotation = Quaternion::identity;

	//-- no translation
	translation = Vector::zero;
} //lint !e1763 // const function indirectly modifies class --- how?

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::getLocomotionRotation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int & /* translationStartKeyIndex */, Quaternion &rotation, Vector &translation) const
{
	NOT_NULL(m_locomotionRotationKeys);

	//-- get rotation (relative to Maya scene world) at begin time
	const Quaternion beginRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationKeys, beginFrameTime, rotationStartKeyIndex);

	//-- get rotation (relative to Maya scene world) at end time
	const Quaternion endRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationKeys, endFrameTime, rotationStartKeyIndex);

	//-- calculate delta rotation relative to begin time rotation
	rotation = endRotation_mayaWorld * beginRotation_mayaWorld.getComplexConjugate();

	//-- no translation
	translation = Vector::zero;	
} //lint !e1763 // const function indirectly modifies class --- how?

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::getLocomotionRotationTranslation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const
{
	NOT_NULL(m_locomotionRotationKeys);
	NOT_NULL(m_locomotionTranslationKeys);

	//-- get rotation (relative to Maya scene world) at begin time
	const Quaternion beginRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationKeys, beginFrameTime, rotationStartKeyIndex);

	//-- get rotation (relative to Maya scene world) at end time
	const Quaternion endRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationKeys, endFrameTime, rotationStartKeyIndex);

	//-- calculate delta rotation relative to begin time rotation
	rotation = endRotation_mayaWorld * beginRotation_mayaWorld.getComplexConjugate();

	//-- get translation (relative to Maya scene world) at begin time
	const Vector beginPosition_mayaWorld = computeVectorFromKeys(*m_locomotionTranslationKeys, beginFrameTime, translationStartKeyIndex);

	//-- get translation (relative to Maya scene world) at end time
	const Vector endPosition_mayaWorld = computeVectorFromKeys(*m_locomotionTranslationKeys, endFrameTime, translationStartKeyIndex);

	//-- calculate delta translation assuming delta rotation was applied
	//   prior to translation (i.e. find delta translation in post-rotation
	//   space relative to start world position.
	const Vector deltaPosition_mayaWorld = endPosition_mayaWorld - beginPosition_mayaWorld;

	// @todo -TRF- this could be handled FAR more efficiently via a vector transform operation on the Quaternion.
	// build objectToMayaWorld transform from end MayaWorld rotation
	Transform  objectToMayaWorld(Transform::IF_none);
	endRotation_mayaWorld.getTransformPreserveTranslation(&objectToMayaWorld);

	// convert delta position in MayaWorld space into object space
	translation = objectToMayaWorld.rotate_p2l(deltaPosition_mayaWorld);
} //lint !e1763 // const function indirectly modifies class --- how?

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplate::getLocomotionTranslation(float beginFrameTime, float endFrameTime, int & /* rotationStartKeyIndex */, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const
{
	NOT_NULL(m_locomotionTranslationKeys);

	//-- get translation (relative to Maya scene world) at begin time
	const Vector beginPosition_mayaWorld = computeVectorFromKeys(*m_locomotionTranslationKeys, beginFrameTime, translationStartKeyIndex);

	//-- get translation (relative to Maya scene world) at end time
	const Vector endPosition_mayaWorld = computeVectorFromKeys(*m_locomotionTranslationKeys, endFrameTime, translationStartKeyIndex);

	//-- calculate delta translation
	translation = endPosition_mayaWorld - beginPosition_mayaWorld;

	//-- no rotation
	rotation = Quaternion::identity;
} //lint !e1763 // const function indirectly modifies class --- how?

// ======================================================================
