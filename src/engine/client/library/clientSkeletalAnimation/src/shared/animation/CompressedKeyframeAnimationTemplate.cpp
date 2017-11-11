// ======================================================================
//
// CompressedKeyframeAnimationTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/CompressedKeyframeAnimationTemplate.h"

#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/CompressedKeyframeAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Quaternion.h"
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

namespace CompressedKeyframeAnimationTemplateNamespace
{
	const Tag TAG_AROT = TAG(A,R,O,T);
	const Tag TAG_ATRN = TAG(A,T,R,N);
	const Tag TAG_CHNL = TAG(C,H,N,L);
	const Tag TAG_CKAT = TAG(C,K,A,T);
	const Tag TAG_LOCT = TAG(L,O,C,T);
	const Tag TAG_MESG = TAG(M,E,S,G);
	const Tag TAG_MSGS = TAG(M,S,G,S);
	const Tag TAG_QCHN = TAG(Q,C,H,N);
	const Tag TAG_SROT = TAG(S,R,O,T);
	const Tag TAG_STRN = TAG(S,T,R,N);
	const Tag TAG_XFIN = TAG(X,F,I,N);
	const Tag TAG_XFRM = TAG(X,F,R,M);

	bool s_rotationCompressionFix = true;

	bool s_translationFix = true;
	float s_translationFixEpsilon = std::numeric_limits<float>::epsilon() * 2.0f;

	bool shouldCompressKeyFrame(float const previousValue, float const nextValue)
	{
		return !WithinEpsilonInclusive(previousValue, nextValue, s_translationFixEpsilon);
	}
}

using namespace CompressedKeyframeAnimationTemplateNamespace;

// ======================================================================
// enclosed member declarations
// ======================================================================

// @todo revisit access levels on TransformInfo (why is everything private?  legacy from previously being an exposed class?)

class CompressedKeyframeAnimationTemplate::TransformInfo
{
friend class CompressedKeyframeAnimationTemplate;

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

// ----------------------------------------------------------------------

class CompressedKeyframeAnimationTemplate::FullCompressedQuaternion
{
public:

	FullCompressedQuaternion(const CompressedQuaternion &rotation, uint8 xFormat, uint8 yFormat, uint8 zFormat);

	Quaternion  expand() const;

private:

	// Disabled.
	FullCompressedQuaternion();
	FullCompressedQuaternion &operator =(const FullCompressedQuaternion&);

private:

	const CompressedQuaternion m_rotation;
	const uint8                m_xFormat;
	const uint8                m_yFormat;
	const uint8                m_zFormat;

};

// ======================================================================

class CompressedKeyframeAnimationTemplate::TranslationChannel
{
public:

	TranslationChannel();

	const RealKeyDataVector &getKeyDataVector() const;

	void load_0001(Iff &iff);

private:

	RealKeyDataVector  m_keyDataVector;

};

// ======================================================================

class CompressedKeyframeAnimationTemplate::RotationChannel
{
public:

	RotationChannel();

	const QuaternionKeyDataVector &getKeyDataVector() const;
	QuaternionKeyDataVector       &getKeyDataVector();

	uint8                          getXCompressionFormat() const;
	uint8                          getYCompressionFormat() const;
	uint8                          getZCompressionFormat() const;

	void load_0001(Iff &iff);

private:

	uint8                    m_xCompressionFormat;
	uint8                    m_yCompressionFormat;
	uint8                    m_zCompressionFormat;
	QuaternionKeyDataVector  m_keyDataVector;
};

// ======================================================================

class CompressedKeyframeAnimationTemplate::Message
{
public:

	static Message *createFromIff_0001(Iff &iff);

public:

	const CrcLowerString &getName() const;
	const IntVector      &getSignaledFrameNumbers() const;

private:

	Message();

	void  load_0001(Iff &iff);

private:

	CrcLowerString  m_name;
	IntVector       m_signaledFrameNumbers;

};

// ----------------------------------------------------------------------

class CompressedKeyframeAnimationTemplate::SignaledMessageReporter
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

bool                CompressedKeyframeAnimationTemplate::ms_installed;
MemoryBlockManager *CompressedKeyframeAnimationTemplate::ms_memoryBlockManager;

// ======================================================================
// struct CompressedKeyframeAnimationTemplate::RealKeyData
// ======================================================================

CompressedKeyframeAnimationTemplate::RealKeyData::RealKeyData() :
	m_frameNumber(0.0f),
	m_keyValue(0.0f),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}


CompressedKeyframeAnimationTemplate::RealKeyData::RealKeyData(float frameNumber, float keyValue) :
	m_frameNumber(frameNumber),
	m_keyValue(keyValue),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// struct CompressedKeyframeAnimationTemplate::QuaternionKeyData
// ======================================================================

CompressedKeyframeAnimationTemplate::QuaternionKeyData::QuaternionKeyData() :
	m_frameNumber(0.0f),
	m_rotation(0),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

CompressedKeyframeAnimationTemplate::QuaternionKeyData::QuaternionKeyData(float frameNumber, const CompressedQuaternion &rotation) :
	m_frameNumber(frameNumber),
	m_rotation(rotation),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// struct CompressedKeyframeAnimationTemplate::VectorKeyData
// ======================================================================

CompressedKeyframeAnimationTemplate::VectorKeyData::VectorKeyData(float frameNumber, const Vector &vector) :
	m_frameNumber(frameNumber),
	m_vector(vector),
	m_oneOverDistanceToNextKeyframe(1.0f)
{
}

// ======================================================================
// class CompressedKeyframeAnimationTemplate::TransformInfo
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(CompressedKeyframeAnimationTemplate::TransformInfo, true, 0, 0, 0);

// ======================================================================

inline CrcString const &CompressedKeyframeAnimationTemplate::TransformInfo::getTransformName() const
{
	return m_name;
}

// ======================================================================

inline CompressedKeyframeAnimationTemplate::TransformInfo::TransformInfo(
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
// class CompressedKeyframeAnimationTemplate::FullCompressedQuaternion
// ======================================================================

CompressedKeyframeAnimationTemplate::FullCompressedQuaternion::FullCompressedQuaternion(const CompressedQuaternion &rotation, uint8 xFormat, uint8 yFormat, uint8 zFormat) :
	m_rotation(rotation),
	m_xFormat(xFormat),
	m_yFormat(yFormat),
	m_zFormat(zFormat)
{
}

// ----------------------------------------------------------------------

inline Quaternion CompressedKeyframeAnimationTemplate::FullCompressedQuaternion::expand() const
{
	return m_rotation.expand(m_xFormat, m_yFormat, m_zFormat);
}

// ======================================================================
// struct CompressedKeyframeAnimationTemplate::TransformInfo::LessNameComparator
// ======================================================================

bool CompressedKeyframeAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const TransformInfo *lhs, const TransformInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getTransformName() < rhs->getTransformName();
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const CrcString *lhs, const TransformInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return *lhs < rhs->getTransformName();
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::TransformInfo::LessNameComparator::operator ()(const TransformInfo *lhs, const CrcString *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getTransformName() < *rhs;
}

// ======================================================================
// class CompressedKeyframeAnimationTemplate::Message
// ======================================================================

CompressedKeyframeAnimationTemplate::Message *CompressedKeyframeAnimationTemplate::Message::createFromIff_0001(Iff &iff)
{
	Message *const message = new Message();
	message->load_0001(iff);

	return message;
}

// ======================================================================

const CrcLowerString &CompressedKeyframeAnimationTemplate::Message::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::IntVector &CompressedKeyframeAnimationTemplate::Message::getSignaledFrameNumbers() const
{
	return m_signaledFrameNumbers;
}

// ======================================================================

CompressedKeyframeAnimationTemplate::Message::Message()
:	m_name(""),
	m_signaledFrameNumbers()
{
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::Message::load_0001(Iff &iff)
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
// class CompressedKeyframeAnimationTemplate::SignaledMessageReporter
// ======================================================================

CompressedKeyframeAnimationTemplate::SignaledMessageReporter::SignaledMessageReporter(IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers, int beginFrameNumber, int endFrameNumber):	
	m_messageIndex(0),
	m_signaledMessageIndices(signaledMessageIndices),
	m_signaledMessageFrameNumbers(signaledMessageFrameNumbers),
	m_beginFrameNumber(beginFrameNumber),
	m_endFrameNumber(endFrameNumber)
{
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::SignaledMessageReporter::operator()(const Message *message)
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
// class CompressedKeyframeAnimationTemplate::RotationChannel
// ======================================================================

CompressedKeyframeAnimationTemplate::RotationChannel::RotationChannel() :
	m_xCompressionFormat(0),
	m_yCompressionFormat(0),
	m_zCompressionFormat(0),
	m_keyDataVector()
{
}

// ----------------------------------------------------------------------

inline const CompressedKeyframeAnimationTemplate::QuaternionKeyDataVector &CompressedKeyframeAnimationTemplate::RotationChannel::getKeyDataVector() const
{
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

inline CompressedKeyframeAnimationTemplate::QuaternionKeyDataVector &CompressedKeyframeAnimationTemplate::RotationChannel::getKeyDataVector()
{
	// @todo: remove when load support for 0002 animation is removed.
	//lint -esym(1536, RotationChannel::m_keyDataVector) // exposing low-access member // that's okay, that's the whole point of this class
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::RotationChannel::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_QCHN);
	{
		//-- Get key count.
		const int keyCount = static_cast<int>(iff.read_int16());

		//-- Get compression format.
		m_xCompressionFormat = iff.read_uint8();
		m_yCompressionFormat = iff.read_uint8();
		m_zCompressionFormat = iff.read_uint8();

		//-- reserve appropriate amount of space
		m_keyDataVector.reserve(static_cast<size_t>(keyCount));

		// load data
		{
			uint32 lastCompressedRotation = 0;
			
			for (int i = 0; i < keyCount; ++i)
			{
				const float      frameNumber        = static_cast<real>(iff.read_int16());
				const uint32     compressedRotation = iff.read_uint32();

				// Remove even frames.
				if (!s_rotationCompressionFix || (((i == 0) || (i == (keyCount - 1)) || !(i & 1)) && (lastCompressedRotation != compressedRotation)))
				{
					m_keyDataVector.push_back(QuaternionKeyData(frameNumber, CompressedQuaternion(compressedRotation)));
				}
				
				lastCompressedRotation = compressedRotation;
			}
		}

		// Shrink the array.
		m_keyDataVector.resize(m_keyDataVector.size());

		//-- calculate one over distance between each keyframe and next
		calculateQuaternionDistanceToNextFrame(m_keyDataVector);
	}
	iff.exitChunk(TAG_QCHN);
}

// ----------------------------------------------------------------------

inline uint8 CompressedKeyframeAnimationTemplate::RotationChannel::getXCompressionFormat() const
{
	return m_xCompressionFormat;
}

// ----------------------------------------------------------------------

inline uint8 CompressedKeyframeAnimationTemplate::RotationChannel::getYCompressionFormat() const
{
	return m_yCompressionFormat;
}

// ----------------------------------------------------------------------

inline uint8 CompressedKeyframeAnimationTemplate::RotationChannel::getZCompressionFormat() const
{
	return m_zCompressionFormat;
}

// ======================================================================
// class CompressedKeyframeAnimationTemplate::TranslationChannel
// ======================================================================

CompressedKeyframeAnimationTemplate::TranslationChannel::TranslationChannel() :
	m_keyDataVector()
{
}

// ----------------------------------------------------------------------

inline const CompressedKeyframeAnimationTemplate::RealKeyDataVector &CompressedKeyframeAnimationTemplate::TranslationChannel::getKeyDataVector() const
{
	return m_keyDataVector;
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::TranslationChannel::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_CHNL);
	{
		//-- get key count
		const int keyCount = static_cast<int>(iff.read_int16());
		const int keyCount2 = keyCount / 2;

		//-- reserve appropriate amount of space
		m_keyDataVector.reserve(static_cast<size_t>(keyCount));

		// load data
		{
			float lastValue = std::numeric_limits<float>::max();

			for (int i = 0; i < keyCount; ++i)
			{
				const float frameNumber = static_cast<real>(iff.read_int16());
				const float value       = iff.read_float();

				if (!s_translationFix || ((i == 0) || (i == (keyCount - 1)) || (i == keyCount2) || shouldCompressKeyFrame(lastValue, value)))
				{
					m_keyDataVector.push_back(RealKeyData(frameNumber, value));
					lastValue = value;
				}
			}
		}

		// Shrink the array.
		m_keyDataVector.resize(m_keyDataVector.size());

		//-- calculate one over distance between each keyframe and next
		calculateFloatDistanceToNextFrame(m_keyDataVector);
	}
	iff.exitChunk(TAG_CHNL);
}

// ======================================================================
// CompressedKeyframeAnimationTemplate public static member functions
// ======================================================================

void CompressedKeyframeAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("CompressedKeyframeAnimationTemplate already installed"));

	CompressedKeyframeAnimationTemplate::TransformInfo::install();

	const bool success = SkeletalAnimationTemplateList::registerCreateFunction(TAG_CKAT, create);
	DEBUG_FATAL(!success, ("failed to register CompressedKeyframeAnimationTemplate"));
	UNREF(success);

	ms_memoryBlockManager       = new MemoryBlockManager("CompressedKeyframeAnimationTemplate", true, sizeof(CompressedKeyframeAnimationTemplate), 0, 0, 0);

	ms_installed = true;
	ExitChain::add(remove, "CompressedKeyframeAnimationTemplate");
}

// ----------------------------------------------------------------------

void *CompressedKeyframeAnimationTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("CompressedKeyframeAnimationTemplate not installed"));
	DEBUG_FATAL(size != sizeof(CompressedKeyframeAnimationTemplate), ("derived classes not supported by this operator new"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::operator delete(void *data)
{
	ms_memoryBlockManager->free(data);
}

// ----------------------------------------------------------------------

Quaternion CompressedKeyframeAnimationTemplate::computeQuaternionFromKeys(const RotationChannel &rotationChannel, float frameTime, int &startKeyIndex)
{
	const QuaternionKeyDataVector &keyDataVector      = rotationChannel.getKeyDataVector();
	DEBUG_FATAL(keyDataVector.empty(), ("No keys in this animation channel's data."));

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

	//-- Retrieve compression format.
	const uint8 xCompressionFormat = rotationChannel.getXCompressionFormat();
	const uint8 yCompressionFormat = rotationChannel.getYCompressionFormat();
	const uint8 zCompressionFormat = rotationChannel.getZCompressionFormat();

	//-- get frame distance to next key
	const float oneOverFrameDistance = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_oneOverDistanceToNextKeyframe;
	if (oneOverFrameDistance == 0.0f)
	{
		// the animation frame number must be exactly on a keyframe
		return keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation.expand(xCompressionFormat, yCompressionFormat, zCompressionFormat);
	}
	else
	{
		//-- find closest keyframe occurring on or after animation frame number
		int upperKeyframeIndex = lowerKeyframeIndex + 1;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, upperKeyframeIndex, keyCount);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber, frameTime, keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_frameNumber);

		const float       upperKeyframeWeight   = (frameTime - keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_frameNumber) * oneOverFrameDistance;
		const Quaternion  upperKeyframeRotation = keyDataVector[static_cast<size_t>(upperKeyframeIndex)].m_rotation.expand(xCompressionFormat, yCompressionFormat, zCompressionFormat);
		const Quaternion  lowerKeyframeRotation = keyDataVector[static_cast<size_t>(lowerKeyframeIndex)].m_rotation.expand(xCompressionFormat, yCompressionFormat, zCompressionFormat);

		//-- return the interpolation between the rotations based on weighting
		return lowerKeyframeRotation.slerp(upperKeyframeRotation, upperKeyframeWeight);
	}
}

// ----------------------------------------------------------------------

Vector CompressedKeyframeAnimationTemplate::computeVectorFromKeys(const VectorKeyDataVector &keyDataVector, float frameTime, int &startKeyIndex)
{
	DEBUG_FATAL(keyDataVector.empty(), ("No keys in this animation channel's data."));

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

void CompressedKeyframeAnimationTemplate::calculateVectorDistanceToNextFrame(VectorKeyDataVector &keyDataVector)
{
	DEBUG_FATAL(keyDataVector.empty(), ("No keys in this animation channel's data."));

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

void CompressedKeyframeAnimationTemplate::calculateQuaternionDistanceToNextFrame(QuaternionKeyDataVector &keyDataVector)
{
	DEBUG_FATAL(keyDataVector.empty(), ("No keys in this animation channel's data."));

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

void CompressedKeyframeAnimationTemplate::calculateFloatDistanceToNextFrame(RealKeyDataVector &keyDataVector)
{
	DEBUG_FATAL(keyDataVector.empty(), ("No keys in this animation channel's data."));

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
// CompressedKeyframeAnimationTemplate public member functions
// ======================================================================

SkeletalAnimation *CompressedKeyframeAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	// create it
	CompressedKeyframeAnimation *skeletalAnimation = new CompressedKeyframeAnimation(this, animationEnvironment, transformNameMap);
	NOT_NULL(skeletalAnimation);

	// get initial reference for caller
	skeletalAnimation->fetch();

	return skeletalAnimation;
}

// ----------------------------------------------------------------------

int CompressedKeyframeAnimationTemplate::getTransformCount() const
{
	return static_cast<int>(m_transformInfoVector.size());
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::TransformInfo &CompressedKeyframeAnimationTemplate::getTransformInfo(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());
	return *m_transformInfoVector[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int CompressedKeyframeAnimationTemplate::getTransformIndex(CrcString const &transformName) const
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

CrcString const &CompressedKeyframeAnimationTemplate::getName(const TransformInfo &transformInfo) const
{
	return transformInfo.m_name;
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::hasAnimatedRotation(const TransformInfo &transformInfo) const
{
	return transformInfo.m_hasAnimatedRotation;
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::RotationChannel &CompressedKeyframeAnimationTemplate::getAnimatedRotationChannel(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!transformInfo.m_hasAnimatedRotation, ("Transform [%s] doesn't animate rotation", transformInfo.m_name.getString()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_rotationChannelIndex, static_cast<int>(m_rotationChannels.size()));

	return m_rotationChannels[static_cast<size_t>(transformInfo.m_rotationChannelIndex)];
}

// ----------------------------------------------------------------------

const Quaternion CompressedKeyframeAnimationTemplate::getStaticRotation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(transformInfo.m_hasAnimatedRotation, ("Transform [%s] doesn't have static rotation.", transformInfo.m_name.getString()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_rotationChannelIndex, static_cast<int>(m_staticRotations.size()));

	return m_staticRotations[static_cast<size_t>(transformInfo.m_rotationChannelIndex)].expand();
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::animatesTranslation(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_translationMask) != 0;
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::animatesTranslationX(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_xTranslation) != 0;
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::animatesTranslationY(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_yTranslation) != 0;
}

// ----------------------------------------------------------------------

bool CompressedKeyframeAnimationTemplate::animatesTranslationZ(const TransformInfo &transformInfo) const
{
	return (transformInfo.m_translationMask & SATCCF_zTranslation) != 0;
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::RealKeyDataVector &CompressedKeyframeAnimationTemplate::getXTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationX(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_xTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_xTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::RealKeyDataVector &CompressedKeyframeAnimationTemplate::getYTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationY(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_yTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_yTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

const CompressedKeyframeAnimationTemplate::RealKeyDataVector &CompressedKeyframeAnimationTemplate::getZTranslationChannelData(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(!animatesTranslationZ(transformInfo), ("doesn't animate this channel"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_zTranslationChannelIndex, static_cast<int>(m_translationChannels.size()));

	return m_translationChannels[static_cast<size_t>(transformInfo.m_zTranslationChannelIndex)].getKeyDataVector();
}

// ----------------------------------------------------------------------

float CompressedKeyframeAnimationTemplate::getStaticXTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationX(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_xTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_xTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

float CompressedKeyframeAnimationTemplate::getStaticYTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationY(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_yTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_yTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

float CompressedKeyframeAnimationTemplate::getStaticZTranslation(const TransformInfo &transformInfo) const
{
	DEBUG_FATAL(animatesTranslationZ(transformInfo), ("can't have a static translation"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformInfo.m_zTranslationChannelIndex, static_cast<int>(m_staticTranslations.size()));

	return m_staticTranslations[static_cast<size_t>(transformInfo.m_zTranslationChannelIndex)];
}

// ----------------------------------------------------------------------

int CompressedKeyframeAnimationTemplate::getMessageCount() const
{
	if (m_messages)
		return static_cast<int>(m_messages->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

const CrcLowerString &CompressedKeyframeAnimationTemplate::getMessageName(int index) const
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

void CompressedKeyframeAnimationTemplate::getSignaledMessages(int beginFrameNumber, int endFrameNumber, IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers) const
{
	if (!m_messages)
		return;

	std::for_each(m_messages->begin(), m_messages->end(), SignaledMessageReporter(signaledMessageIndices, signaledMessageFrameNumbers, beginFrameNumber, endFrameNumber));
}

// ======================================================================
// CompressedKeyframeAnimationTemplate private static member functions
// ======================================================================

void CompressedKeyframeAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("CompressedKeyframeAnimationTemplate not installed"));

	const bool success = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_CKAT);
	DEBUG_FATAL(!success, ("failed to deregister CompressedKeyframeAnimationTemplate"));
	UNREF(success);

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *CompressedKeyframeAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	return new CompressedKeyframeAnimationTemplate(name, iff);
}

// ======================================================================
// CompressedKeyframeAnimationTemplate private member functions
// ======================================================================

CompressedKeyframeAnimationTemplate::CompressedKeyframeAnimationTemplate(const CrcLowerString &name, Iff &iff)
:	SkeletalAnimationTemplate(name),
	m_framesPerSecond(0),
	m_frameCount(0),
	m_transformInfoVector(),
	m_rotationChannels(),
	m_translationChannels(),
	m_staticRotations(),
	m_staticTranslations(),
	m_messages(0),
	m_locomotionRotationChannel(0),
	m_locomotionTranslationKeys(0),
	m_averageTranslationSpeed(0.0f),
   m_getLocomotionFunction(&CompressedKeyframeAnimationTemplate::getLocomotionNone)
{
	iff.enterForm(TAG_CKAT);
	
		switch (iff.getCurrentName())
		{
			case TAG_0000:
				DEBUG_FATAL(true, ("-TRF- I canned this format before code was ever submitted.  This must be stale data."));
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
			{
				char tagName[5];
				ConvertTagToString(iff.getCurrentName(), tagName);
				FATAL(true, ("unknown or unsupported CompressedKeyframeAnimationTemplate version [%s]", tagName));
			}
		}

	iff.exitForm(TAG_CKAT);

	//-- determine which locomotion function to use based on locomotion data provided in data.
	if (m_locomotionRotationChannel && m_locomotionTranslationKeys)
		m_getLocomotionFunction = &CompressedKeyframeAnimationTemplate::getLocomotionRotationTranslation;
	else if (m_locomotionRotationChannel)
		m_getLocomotionFunction = &CompressedKeyframeAnimationTemplate::getLocomotionRotation;
	else if (m_locomotionTranslationKeys)
		m_getLocomotionFunction = &CompressedKeyframeAnimationTemplate::getLocomotionTranslation;

	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogSktCreateDestroy(), ("CKAT: CREATE [%s].\n", SkeletalAnimationTemplate::getName().getString()));
}

// ----------------------------------------------------------------------

CompressedKeyframeAnimationTemplate::~CompressedKeyframeAnimationTemplate()
{
	if (m_messages)
	{
		std::for_each(m_messages->begin(), m_messages->end(), PointerDeleter());
		delete m_messages;
	}

	delete m_locomotionRotationChannel;
	delete m_locomotionTranslationKeys;

	std::for_each(m_transformInfoVector.begin(), m_transformInfoVector.end(), PointerDeleter());


	DEBUG_REPORT_LOG(ConfigClientSkeletalAnimation::getLogSktCreateDestroy(), ("KSAT: DESTROY [%s].\n", SkeletalAnimationTemplate::getName().getString()));
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::load_0001(Iff &iff)
{
	int transformInfoCount      = 0;
	int rotationChannelCount    = 0;
	int staticRotationCount     = 0;
	int translationChannelCount = 0;
	int staticTranslationCount  = 0;

	iff.enterForm(TAG_0001);

		//-- load general animation information
		iff.enterChunk(TAG_INFO);
		{
			m_framesPerSecond       = iff.read_float();
			m_frameCount            = static_cast<int>(iff.read_int16());
			transformInfoCount      = static_cast<int>(iff.read_int16());

			rotationChannelCount    = static_cast<int>(iff.read_int16());
			staticRotationCount     = static_cast<int>(iff.read_int16());
			translationChannelCount = static_cast<int>(iff.read_int16());
			staticTranslationCount  = static_cast<int>(iff.read_int16());
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
					const int  rotationChannelIndex     = static_cast<int>(iff.read_int16());

					const uint translationMask          = static_cast<uint>(iff.read_uint8());
					const int  xTranslationChannelIndex = static_cast<int>(iff.read_int16());
					const int  yTranslationChannelIndex = static_cast<int>(iff.read_int16());
					const int  zTranslationChannelIndex = static_cast<int>(iff.read_int16());

					m_transformInfoVector.push_back(new TransformInfo(
							nameBuffer,
							newHasAnimatedRotations,
							rotationChannelIndex,
							translationMask,
							xTranslationChannelIndex,
							yTranslationChannelIndex,
							zTranslationChannelIndex));

 					// DEBUG_REPORT_LOG(true, ("%s - %i (%i) %i (%i) %i (%i)\n", nameBuffer, xTranslationChannelIndex, ((translationMask & SATCCF_xTranslation) != 0) ? 1 : 0, yTranslationChannelIndex, ((translationMask & SATCCF_yTranslation) != 0) ? 1 : 0, zTranslationChannelIndex, ((translationMask & SATCCF_zTranslation) != 0) ? 1 : 0));
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
				m_rotationChannels[i].load_0001(iff);

			iff.exitForm(TAG_AROT);
		}

		//-- load static rotation data
		const bool hasSrotTag = iff.enterChunk(TAG_SROT, true);
		DEBUG_FATAL(!hasSrotTag && staticRotationCount, ("staticRotationCount [%d] with no data", staticRotationCount));
		if (hasSrotTag)
		{
			m_staticRotations.reserve(static_cast<size_t>(staticRotationCount));
			
			for (size_t i = 0; i < static_cast<size_t>(staticRotationCount); ++i)
			{
				const uint8  xFormat                 = iff.read_uint8();
				const uint8  yFormat                 = iff.read_uint8();
				const uint8  zFormat                 = iff.read_uint8();
				const uint32 compressedRotationValue = iff.read_uint32();
				m_staticRotations.push_back(FullCompressedQuaternion(CompressedQuaternion(compressedRotationValue), xFormat, yFormat, zFormat));
			}

			iff.exitChunk(TAG_SROT);
		}

		//-- load animated translation channels
		const bool hasAtrnTag = iff.enterForm(TAG_ATRN, true);
		DEBUG_FATAL(!hasAtrnTag && translationChannelCount, ("translationChannelCount [%d] with no data", translationChannelCount));
		if (hasAtrnTag)
		{
			m_translationChannels.resize(static_cast<size_t>(translationChannelCount));

			for (size_t i = 0; i < static_cast<size_t>(translationChannelCount); ++i)
			{
				// DEBUG_REPORT_LOG(true, ("animated translation channel %i\n", i));
				m_translationChannels[i].load_0001(iff);
			}

			iff.exitForm(TAG_ATRN);
		}

		//-- load static translation data
		const bool hasStrnTag = iff.enterChunk(TAG_STRN, true);
		DEBUG_FATAL(!hasStrnTag && staticTranslationCount, ("staticTranslationCount [%d] with no data", staticTranslationCount));
		if (hasStrnTag)
		{
			m_staticTranslations.reserve(static_cast<size_t>(staticTranslationCount));
			
			for (size_t i = 0; i < static_cast<size_t>(staticTranslationCount); ++i)
			{
				m_staticTranslations.push_back(iff.read_float());
 				// DEBUG_REPORT_LOG(true, ("STRN %1.2f\n", m_staticTranslations.back()));
			}

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
					m_messages->push_back(Message::createFromIff_0001(iff));
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
		if (!iff.atEndOfForm() && (iff.getCurrentName() == TAG_QCHN))
		{
			m_locomotionRotationChannel = new RotationChannel();
			m_locomotionRotationChannel->load_0001(iff);
		}

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::getLocomotionNone(float /* beginFrameTime */, float /* endFrameTime */, int & /* rotationStartKeyIndex */, int & /* translationStartKeyIndex */, Quaternion &rotation, Vector &translation) const
{
	//-- no rotation
	rotation = Quaternion::identity;

	//-- no translation
	translation = Vector::zero;
} //lint !e1763 // const function indirectly modifies class --- how? (because some parameters are passed by reference and modify the class)

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::getLocomotionRotation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int & /* translationStartKeyIndex */, Quaternion &rotation, Vector &translation) const
{
	NOT_NULL(m_locomotionRotationChannel);

	//-- get rotation (relative to Maya scene world) at begin time
	const Quaternion beginRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationChannel, beginFrameTime, rotationStartKeyIndex);

	//-- get rotation (relative to Maya scene world) at end time
	const Quaternion endRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationChannel, endFrameTime, rotationStartKeyIndex);

	//-- calculate delta rotation relative to begin time rotation
	rotation = endRotation_mayaWorld * beginRotation_mayaWorld.getComplexConjugate();

	//-- no translation
	translation = Vector::zero;	
} //lint !e1763 // const function indirectly modifies class --- how? (because some parameters are passed by reference and modify the class)

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::getLocomotionRotationTranslation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const
{
	NOT_NULL(m_locomotionRotationChannel);
	NOT_NULL(m_locomotionTranslationKeys);

	//-- get rotation (relative to Maya scene world) at begin time
	const Quaternion beginRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationChannel, beginFrameTime, rotationStartKeyIndex);

	//-- get rotation (relative to Maya scene world) at end time
	const Quaternion endRotation_mayaWorld = computeQuaternionFromKeys(*m_locomotionRotationChannel, endFrameTime, rotationStartKeyIndex);

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
} //lint !e1763 // const function indirectly modifies class --- how? (because some parameters are passed by reference and modify the class)

// ----------------------------------------------------------------------

void CompressedKeyframeAnimationTemplate::getLocomotionTranslation(float beginFrameTime, float endFrameTime, int & /* rotationStartKeyIndex */, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const
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
} //lint !e1763 // const function indirectly modifies class --- how? (because some parameters are passed by reference and modify the class)

// ======================================================================
