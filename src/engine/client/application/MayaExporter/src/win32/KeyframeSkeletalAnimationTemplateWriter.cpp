// ======================================================================
//
// KeyframeSkeletalAnimationTemplateWriter.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "KeyframeSkeletalAnimationTemplateWriter.h"

#include "clientSkeletalAnimation/KeyframeSkeletalAnimationTemplateDef.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/CompressedQuaternion.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <map>
#include <string>
#include <vector>
#include <algorithm>

// ======================================================================

namespace KeyframeSkeletalAnimationTemplateWriterNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<Quaternion>            QuaternionVector;
	typedef std::vector<CompressedQuaternion>  CompressedQuaternionVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_AROT = TAG(A,R,O,T);
	const Tag TAG_ATRN = TAG(A,T,R,N);
	const Tag TAG_CHNL = TAG(C,H,N,L);
	const Tag TAG_CKAT = TAG(C,K,A,T);
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

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                        s_compressData = true;

	QuaternionVector            s_rotations;
	CompressedQuaternionVector  s_compressedRotations;

}

using namespace KeyframeSkeletalAnimationTemplateWriterNamespace;

// ======================================================================
// embedded classes
// ======================================================================

class KeyframeSkeletalAnimationTemplateWriter::AnimationMessage
{
public:

	explicit AnimationMessage(const CrcLowerString &name);

	void                  addMessageSignal(int keyFrameNumber);
	void                  write(Iff &iff) const;

	const CrcLowerString &getName() const;

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	// disabled
	AnimationMessage();
	AnimationMessage(const AnimationMessage&);             //lint -esym(754, AnimationMessage::AnimationMessage) // not referenced // that's okay, this is preventative
	AnimationMessage &operator =(const AnimationMessage&); //lint -esym(754, AnimationMessage::operator=) // not referenced // that's okay, this is preventative

private:

	CrcLowerString  m_name;
	IntVector       m_signalFrameNumbers;
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::QuaternionKeyData
{
public:

	QuaternionKeyData(int frameNumber, const Quaternion &rotation);

public:

	int         m_frameNumber;
	Quaternion  m_rotation;

private:

	// disabled
	QuaternionKeyData();
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::RealKeyData
{
public:

	RealKeyData(int frameNumber, float keyData);

public:

	int    m_frameNumber;
	float  m_keyData; 

private:

	// disabled
	RealKeyData();
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::VectorKeyData
{
public:

	VectorKeyData(int frameNumber, const Vector &keyData);

public:

	int     m_frameNumber;
	Vector  m_vector; 

private:

	// disabled
	VectorKeyData();
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::RotationChannelData
{
	QuaternionKeyDataVector  m_keyData;
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData
{
	RealKeyDataVector  m_keyData;
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::VectorChannelData
{
	VectorKeyDataVector  m_keyData;
};

// ======================================================================

struct KeyframeSkeletalAnimationTemplateWriter::TransformData
{
public:

	explicit TransformData(const char *name);

public:

	std::string  m_name;

	bool         m_hasAnimatedRotation;
	int          m_rotationChannelIndex;

	unsigned     m_translationMask;
	int          m_xTranslationChannelIndex;
	int          m_yTranslationChannelIndex;
	int          m_zTranslationChannelIndex;

private:

	// disabled
	TransformData();
	TransformData(const TransformData&);             //lint -esym(754, TransformData::TransformData) // unreferenced // right, this is for prevention purposes
	TransformData &operator =(const TransformData&); //lint -esym(754, TransformData::operator=)     // unreferenced // right, this is for prevention purposes
};

// ======================================================================
// struct KeyframeSkeletalAnimationTemplateWriter::QuaternionKeyData
// ======================================================================

inline KeyframeSkeletalAnimationTemplateWriter::QuaternionKeyData::QuaternionKeyData(int frameNumber, const Quaternion &rotation) :
	m_frameNumber(frameNumber),
	m_rotation(rotation)
{
}

// ======================================================================
// struct KeyframeSkeletalAnimationTemplateWriter::RealKeyData
// ======================================================================

inline KeyframeSkeletalAnimationTemplateWriter::RealKeyData::RealKeyData(int frameNumber, float keyData) :
	m_frameNumber(frameNumber),
	m_keyData(keyData)
{
}

// ======================================================================
// struct KeyframeSkeletalAnimationTemplateWriter::VectorKeyData
// ======================================================================

inline KeyframeSkeletalAnimationTemplateWriter::VectorKeyData::VectorKeyData(int frameNumber, const Vector &keyData) :
	m_frameNumber(frameNumber),
	m_vector(keyData)
{
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplateWriter::AnimationMessage
// ======================================================================

KeyframeSkeletalAnimationTemplateWriter::AnimationMessage::AnimationMessage(const CrcLowerString &name) :
	m_name(name),
	m_signalFrameNumbers()
{
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::AnimationMessage::addMessageSignal(int keyFrameNumber)
{
	m_signalFrameNumbers.push_back(keyFrameNumber);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::AnimationMessage::write(Iff &iff) const
{
	iff.insertChunk(TAG_MESG);

		//-- write # signal frames
		iff.insertChunkData(static_cast<int16>(m_signalFrameNumbers.size()));

		//-- write message name
		iff.insertChunkString(m_name.getString());

		//-- write frame signal numbers

		// ensure they're sorted in ascending order
		IntVector sortedFrameNumbers(m_signalFrameNumbers);
		std::sort(sortedFrameNumbers.begin(), sortedFrameNumbers.end());

		// write them out
		const IntVector::iterator endIt = sortedFrameNumbers.end();
		for (IntVector::iterator it = sortedFrameNumbers.begin(); it != endIt; ++it)
			iff.insertChunkData(static_cast<int16>(*it));

	iff.exitChunk(TAG_MESG);
}

// ----------------------------------------------------------------------

inline const CrcLowerString &KeyframeSkeletalAnimationTemplateWriter::AnimationMessage::getName() const
{
	return m_name;
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplateWriter::TransformData
// ======================================================================

KeyframeSkeletalAnimationTemplateWriter::TransformData::TransformData(const char *name)
:	m_name(std::string(name)),
	m_hasAnimatedRotation(false),
	m_rotationChannelIndex(-1),
	m_translationMask(0),
	m_xTranslationChannelIndex(-1),
	m_yTranslationChannelIndex(-1),
	m_zTranslationChannelIndex(-1)
{
}

// ======================================================================
// class KeyframeSkeletalAnimationTemplateWriter
// ======================================================================

void KeyframeSkeletalAnimationTemplateWriter::setCompressionMode(bool enabled)
{
	s_compressData = enabled;
}

// ======================================================================

KeyframeSkeletalAnimationTemplateWriter::KeyframeSkeletalAnimationTemplateWriter() :
	m_framesPerSecond(0.0f),
	m_animationFrameCount(0),
	m_averageTranslationSpeed(0.0f),
	m_transformData(new TransformDataVector()),
	m_rotationChannelData(new RotationChannelDataVector()),
	m_translationChannelData(new TranslationChannelDataVector()),
	m_staticRotations(new QuaternionVector()),
	m_staticTranslations(new FloatVector()),
	m_animationMessages(0),
	m_currentAnimationMessage(0),
	m_locomotionRotationChannelData(0),
	m_locomotionTranslationChannelData(0)
{
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::~KeyframeSkeletalAnimationTemplateWriter()
{
	delete m_locomotionTranslationChannelData;
	delete m_locomotionRotationChannelData;

	m_currentAnimationMessage = 0;

	if (m_animationMessages)
	{
		std::for_each(m_animationMessages->begin(), m_animationMessages->end(), PointerDeleterPairSecond());
		delete m_animationMessages;
	}

	delete m_staticTranslations;
	delete m_staticRotations;

	std::for_each(m_transformData->begin(), m_transformData->end(), PointerDeleter());
	delete m_transformData;

	std::for_each(m_translationChannelData->begin(), m_translationChannelData->end(), PointerDeleter());
	delete m_translationChannelData;

	std::for_each(m_rotationChannelData->begin(), m_rotationChannelData->end(), PointerDeleter());
	delete m_rotationChannelData;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::write(Iff &iff) const
{
	if (s_compressData)
		writeCompressed(iff);
	else
		writeUncompressed(iff);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setFramesPerSecond(real framesPerSecond)
{
	m_framesPerSecond = framesPerSecond;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setAnimationFrameCount(int animationFrameCount)
{
	m_animationFrameCount = animationFrameCount;
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::TransformData &KeyframeSkeletalAnimationTemplateWriter::addTransform(const char *name)
{
	DEBUG_FATAL(!name || !*name, ("tried to add invalid transform name (null or empty)"));

	// -TRF- note we're not checking if calling code is trying to add same transform name twice.
	
	//-- add the transform
	m_transformData->push_back(new TransformData(name));

	//-- return reference to it
	return *(m_transformData->back());
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setStaticRotation(TransformData &transformData, const Quaternion &rotation)
{
	DEBUG_FATAL(transformData.m_rotationChannelIndex != -1, ("rotation attribute already set"));

	transformData.m_rotationChannelIndex = static_cast<int>(m_staticRotations->size());
	m_staticRotations->push_back(rotation);

	transformData.m_hasAnimatedRotation = false;
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::RotationChannelData &KeyframeSkeletalAnimationTemplateWriter::addAnimatedRotation(TransformData &transformData)
{
	DEBUG_FATAL(transformData.m_rotationChannelIndex != -1, ("rotation attribute already set"));

	transformData.m_rotationChannelIndex = static_cast<int>(m_rotationChannelData->size());
	m_rotationChannelData->push_back(new RotationChannelData());

	transformData.m_hasAnimatedRotation = true;

	return *m_rotationChannelData->back();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the RotationChannelData instance associated with the locomotion's
 * rotation channel.
 *
 * @return  a RotationChannelData instance that can be used with addRotationKey()
 *          to place rotation keys on the locomotion rotation channel.
 */

KeyframeSkeletalAnimationTemplateWriter::RotationChannelData &KeyframeSkeletalAnimationTemplateWriter::getLocomotionRotationChannelData()
{
	if (!m_locomotionRotationChannelData)
		m_locomotionRotationChannelData = new RotationChannelData;

	NOT_NULL(m_locomotionRotationChannelData);
	return *m_locomotionRotationChannelData;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::addRotationKey(RotationChannelData &channelData, int keyFrame, const Quaternion &rotation) const
{
	channelData.m_keyData.push_back(QuaternionKeyData(keyFrame, rotation));
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setStaticTranslationX(TransformData &transformData, real translation)
{
	DEBUG_FATAL(transformData.m_xTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_xTranslationChannelIndex = static_cast<int>(m_staticTranslations->size());
	m_staticTranslations->push_back(translation);

	transformData.m_translationMask &= ~SATCCF_xTranslation;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setStaticTranslationY(TransformData &transformData, real translation)
{
	DEBUG_FATAL(transformData.m_yTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_yTranslationChannelIndex = static_cast<int>(m_staticTranslations->size());
	m_staticTranslations->push_back(translation);

	transformData.m_translationMask &= ~SATCCF_yTranslation;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::setStaticTranslationZ(TransformData &transformData, real translation)
{
	DEBUG_FATAL(transformData.m_zTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_zTranslationChannelIndex = static_cast<int>(m_staticTranslations->size());
	m_staticTranslations->push_back(translation);

	transformData.m_translationMask &= ~SATCCF_zTranslation;
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &KeyframeSkeletalAnimationTemplateWriter::addChannelTranslationX(TransformData &transformData)
{
	DEBUG_FATAL(transformData.m_xTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_xTranslationChannelIndex = static_cast<int>(m_translationChannelData->size());
	m_translationChannelData->push_back(new TranslationChannelData());

	transformData.m_translationMask |= SATCCF_xTranslation;

	return *m_translationChannelData->back();
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &KeyframeSkeletalAnimationTemplateWriter::addChannelTranslationY(TransformData &transformData)
{
	DEBUG_FATAL(transformData.m_yTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_yTranslationChannelIndex = static_cast<int>(m_translationChannelData->size());
	m_translationChannelData->push_back(new TranslationChannelData());

	transformData.m_translationMask |= SATCCF_yTranslation;

	return *m_translationChannelData->back();
}

// ----------------------------------------------------------------------

KeyframeSkeletalAnimationTemplateWriter::TranslationChannelData &KeyframeSkeletalAnimationTemplateWriter::addChannelTranslationZ(TransformData &transformData)
{
	DEBUG_FATAL(transformData.m_zTranslationChannelIndex != -1, ("translation attribute already set"));

	transformData.m_zTranslationChannelIndex = static_cast<int>(m_translationChannelData->size());
	m_translationChannelData->push_back(new TranslationChannelData());

	transformData.m_translationMask |= SATCCF_zTranslation;

	return *m_translationChannelData->back();
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::addTranslationKey(TranslationChannelData &channelData, int keyFrame, real keyData) const
{
	channelData.m_keyData.push_back(RealKeyData(keyFrame, keyData));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the VectorChannelData instance associated with the locomotion's
 * translation channel.
 *
 * @return  a VectorChannelData instance that can be used with addVectorKey()
 *          to place translation keys on the locomotion's translation channel.
 */

KeyframeSkeletalAnimationTemplateWriter::VectorChannelData &KeyframeSkeletalAnimationTemplateWriter::getLocomotionTranslationChannelData()
{
	if (!m_locomotionTranslationChannelData)
		m_locomotionTranslationChannelData = new VectorChannelData;

	NOT_NULL(m_locomotionTranslationChannelData);
	return *m_locomotionTranslationChannelData;
}

// ----------------------------------------------------------------------
/**
 * Add a key at the given frame and with the given vector to the 
 * specified vector channel.
 *
 * Keys must be added in chronological order.
 *
 * @param channelData  the vector channel to which the key will be appended.
 * @param keyFrame     the 0-based frame number for which the key is specified.
 * @param keyData      the vector value for the key at the given time.
 */

void KeyframeSkeletalAnimationTemplateWriter::addVectorKey(VectorChannelData &channelData, int keyFrame, const Vector &keyData) const
{
	channelData.m_keyData.push_back(VectorKeyData(keyFrame, keyData));
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::beginAnimationMessage(const CrcLowerString &messageName)
{
	// if this triggers, it is likely the endAnimationMessage() was omitted from the last beginAnimationMessage()
	DEBUG_FATAL(m_currentAnimationMessage, ("called beginAnimationMessage() during another beginAnimationMessage()/endAnimationMessage() pair."));

	//-- ensure animation message container exists
	if (!m_animationMessages)
		m_animationMessages = new AnimationMessageMap;

	//-- find or create animation message
	AnimationMessage *animationMessage = 0;

	// look in the map
	AnimationMessageMap::iterator lowerBoundResult = m_animationMessages->lower_bound(&messageName);
	if ((lowerBoundResult == m_animationMessages->end()) || m_animationMessages->key_comp()(&messageName, lowerBoundResult->first))
	{
		//-- create AnimationMessage: it doesn't exist.
		animationMessage = new AnimationMessage(messageName);
	
		//-- add to map
		IGNORE_RETURN(m_animationMessages->insert(lowerBoundResult, AnimationMessageMap::value_type(&animationMessage->getName(), animationMessage)));
	}
	else
	{
		// animation message already exists
		animationMessage = lowerBoundResult->second;
	}

	// this should be set now
	NOT_NULL(animationMessage);

	//-- set as new current message
	m_currentAnimationMessage = animationMessage;
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::addAnimationMessageSignal(int keyFrameNumber)
{
	DEBUG_FATAL(!m_currentAnimationMessage, ("must be called within a beginAnimationMessage()/endAnimationMessage() pair"));
	NOT_NULL(m_currentAnimationMessage);
	
	m_currentAnimationMessage->addMessageSignal(keyFrameNumber);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::endAnimationMessage()
{
	DEBUG_FATAL(!m_currentAnimationMessage, ("must be called after an open call to beginAnimationMessage()"));
	NOT_NULL(m_currentAnimationMessage);

	m_currentAnimationMessage = 0;
}

// ======================================================================

void KeyframeSkeletalAnimationTemplateWriter::writeUncompressed(Iff &iff) const
{
	iff.insertForm(TAG_KFAT);
	{
		iff.insertForm(TAG_0003);
		{
			//-- write general animation info
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<float>(m_framesPerSecond));
				iff.insertChunkData(static_cast<int>(m_animationFrameCount));

				iff.insertChunkData(static_cast<int32>(m_transformData->size()));

				// channel data
				iff.insertChunkData(static_cast<int32>(m_rotationChannelData->size()));
				iff.insertChunkData(static_cast<int32>(m_staticRotations->size()));
				iff.insertChunkData(static_cast<int32>(m_translationChannelData->size()));
				iff.insertChunkData(static_cast<int32>(m_staticTranslations->size()));
			}
			iff.exitChunk(TAG_INFO);

			//-- write transform info container
			iff.insertForm(TAG_XFRM);
			{
				const size_t transformCount = m_transformData->size();
				for (size_t i = 0; i < transformCount; ++i)
				{
					NOT_NULL((*m_transformData)[i]);
					const TransformData &transformData = *(*m_transformData)[i];

					iff.insertChunk(TAG_XFIN);
					{
						iff.insertChunkString(transformData.m_name.c_str());

						iff.insertChunkData(static_cast<int8>(transformData.m_hasAnimatedRotation ? 1 : 0));
						iff.insertChunkData(static_cast<uint32>(transformData.m_rotationChannelIndex));

						iff.insertChunkData(static_cast<uint32>(transformData.m_translationMask));
						iff.insertChunkData(static_cast<uint32>(transformData.m_xTranslationChannelIndex));
						iff.insertChunkData(static_cast<uint32>(transformData.m_yTranslationChannelIndex));
						iff.insertChunkData(static_cast<uint32>(transformData.m_zTranslationChannelIndex));
					}
					iff.exitChunk(TAG_XFIN);
				}
			}
			iff.exitForm(TAG_XFRM);

			//-- write animated rotation channel data
			iff.insertForm(TAG_AROT);
			{
				const size_t channelCount = m_rotationChannelData->size();
				for (size_t i = 0; i < channelCount; ++i)
				{
					NOT_NULL((*m_rotationChannelData)[i]);
					const RotationChannelData *const channelData = (*m_rotationChannelData)[i];

					iff.insertChunk(TAG_QCHN);
					{
						const size_t keyframeCount = channelData->m_keyData.size();
						iff.insertChunkData(static_cast<int32>(keyframeCount));

						for (size_t keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
						{
							const QuaternionKeyData &keyData = channelData->m_keyData[keyframeIndex];

							iff.insertChunkData(static_cast<int32>(keyData.m_frameNumber));
							iff.insertChunkFloatQuaternion(keyData.m_rotation);
						}
					}
					iff.exitChunk(TAG_QCHN);
				}
			}
			iff.exitForm(TAG_AROT);

			//-- write static rotation channel data
			iff.insertChunk(TAG_SROT);
			{
				const size_t valueCount = m_staticRotations->size();
				for (size_t i = 0; i < valueCount; ++i)
					iff.insertChunkFloatQuaternion((*m_staticRotations)[i]);
			}
			iff.exitChunk(TAG_SROT);

			//-- write animated translation channel data
			iff.insertForm(TAG_ATRN);
			{
				const size_t channelCount = m_translationChannelData->size();
				for (size_t i = 0; i < channelCount; ++i)
				{
					NOT_NULL((*m_translationChannelData)[i]);
					const TranslationChannelData *const channelData = (*m_translationChannelData)[i];

					iff.insertChunk(TAG_CHNL);
					{
						const size_t keyframeCount = channelData->m_keyData.size();
						iff.insertChunkData(static_cast<int32>(keyframeCount));

						for (size_t keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
						{
							const RealKeyData &keyData = channelData->m_keyData[keyframeIndex];
							iff.insertChunkData(static_cast<int32>(keyData.m_frameNumber));
							iff.insertChunkData(static_cast<float>(keyData.m_keyData));
						}
					}
					iff.exitChunk(TAG_CHNL);
				}
			}
			iff.exitForm(TAG_ATRN);

			//-- write static translation channel data
			iff.insertChunk(TAG_STRN);
			{
				const size_t valueCount = m_staticTranslations->size();
				for (size_t i = 0; i < valueCount; ++i)
				{
					iff.insertChunkData(static_cast<float>((*m_staticTranslations)[i]));
				}
			}
			iff.exitChunk(TAG_STRN);

			//-- write animation messages
			if ((m_animationMessages) && !m_animationMessages->empty())
			{
				iff.insertForm(TAG_MSGS);
				{
					//-- write # animation messages
					iff.insertChunk(TAG_INFO);
						iff.insertChunkData(static_cast<int16>(m_animationMessages->size()));
					iff.exitChunk(TAG_INFO);

					//-- write each animation message
					const AnimationMessageMap::iterator endIt = m_animationMessages->end();
					for (AnimationMessageMap::iterator it = m_animationMessages->begin(); it != endIt; ++it)
						it->second->write(iff);
				}
				iff.exitForm(TAG_MSGS);
			}

			//-- write locomotion translation data
			if (m_locomotionTranslationChannelData && !m_locomotionTranslationChannelData->m_keyData.empty())
			{
				iff.insertChunk(TAG_LOCT);

					// write average translation speed
					iff.insertChunkData(static_cast<float>(m_averageTranslationSpeed));

					// write # keys
					iff.insertChunkData(static_cast<int16>(m_locomotionTranslationChannelData->m_keyData.size()));

					// write key data
					const VectorKeyDataVector::iterator endIt = m_locomotionTranslationChannelData->m_keyData.end();
					for (VectorKeyDataVector::iterator it = m_locomotionTranslationChannelData->m_keyData.begin(); it != endIt; ++it)
					{
						iff.insertChunkData(static_cast<int16>(it->m_frameNumber));
						iff.insertChunkFloatVector(it->m_vector);
					}

				iff.exitChunk(TAG_LOCT);
			}

			//-- write locomotion rotation data
			if (m_locomotionRotationChannelData && !m_locomotionRotationChannelData->m_keyData.empty())
			{
				iff.insertChunk(TAG_LOCR);

					// insert # keys
					iff.insertChunkData(static_cast<int16>(m_locomotionRotationChannelData->m_keyData.size()));

					const QuaternionKeyDataVector::iterator endIt = m_locomotionRotationChannelData->m_keyData.end();
					for (QuaternionKeyDataVector::iterator it = m_locomotionRotationChannelData->m_keyData.begin(); it != endIt; ++it)
					{
						iff.insertChunkData(static_cast<int16>(it->m_frameNumber));
						iff.insertChunkFloatQuaternion(it->m_rotation);
					}

				iff.exitChunk(TAG_LOCR);
			}
		}
		iff.exitForm(TAG_0003);
	}
	iff.exitForm(TAG_KFAT);
}

// ----------------------------------------------------------------------

void KeyframeSkeletalAnimationTemplateWriter::writeCompressed(Iff &iff) const
{
	iff.insertForm(TAG_CKAT);
	{
		iff.insertForm(TAG_0001);
		{
			//-- write general animation info
			iff.insertChunk(TAG_INFO);
			{
				iff.insertChunkData(static_cast<float>(m_framesPerSecond));
				iff.insertChunkData(static_cast<int16>(m_animationFrameCount));

				iff.insertChunkData(static_cast<int16>(m_transformData->size()));

				// channel data
				iff.insertChunkData(static_cast<int16>(m_rotationChannelData->size()));
				iff.insertChunkData(static_cast<int16>(m_staticRotations->size()));
				iff.insertChunkData(static_cast<int16>(m_translationChannelData->size()));
				iff.insertChunkData(static_cast<int16>(m_staticTranslations->size()));
			}
			iff.exitChunk(TAG_INFO);

			//-- write transform info container
			iff.insertForm(TAG_XFRM);
			{
				const size_t transformCount = m_transformData->size();
				for (size_t i = 0; i < transformCount; ++i)
				{
					NOT_NULL((*m_transformData)[i]);
					const TransformData &transformData = *(*m_transformData)[i];

					iff.insertChunk(TAG_XFIN);
					{
						iff.insertChunkString(transformData.m_name.c_str());

						iff.insertChunkData(static_cast<int8>(transformData.m_hasAnimatedRotation ? 1 : 0));
						iff.insertChunkData(static_cast<int16>(transformData.m_rotationChannelIndex));

						iff.insertChunkData(static_cast<uint8>(transformData.m_translationMask));
						iff.insertChunkData(static_cast<uint16>(transformData.m_xTranslationChannelIndex));
						iff.insertChunkData(static_cast<uint16>(transformData.m_yTranslationChannelIndex));
						iff.insertChunkData(static_cast<uint16>(transformData.m_zTranslationChannelIndex));
					}
					iff.exitChunk(TAG_XFIN);
				}
			}
			iff.exitForm(TAG_XFRM);

			//-- write animated rotation channel data
			iff.insertForm(TAG_AROT);
			{
				const size_t channelCount = m_rotationChannelData->size();
				for (size_t i = 0; i < channelCount; ++i)
				{
					NOT_NULL((*m_rotationChannelData)[i]);
					const RotationChannelData *const channelData = (*m_rotationChannelData)[i];

					s_compressedRotations.clear();
					s_rotations.clear();

					iff.insertChunk(TAG_QCHN);
					{
						const size_t keyframeCount = channelData->m_keyData.size();

						// Collect rotations.
						{
							for (size_t keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
							{
								const QuaternionKeyData &keyData = channelData->m_keyData[keyframeIndex];
								s_rotations.push_back(keyData.m_rotation);
							}
						}

						// Determine optimal compression format for this rotation channel.
						uint8  xFormat;
						uint8  yFormat;
						uint8  zFormat;

						CompressedQuaternion::getOptimalCompressionFormat(s_rotations, xFormat, yFormat, zFormat);

						// Compress the values.
						CompressedQuaternion::compressRotations(s_rotations, xFormat, yFormat, zFormat, s_compressedRotations);

						// Write the compressed rotation channel data.
						iff.insertChunkData(static_cast<int16>(keyframeCount));
						iff.insertChunkData(xFormat);
						iff.insertChunkData(yFormat);
						iff.insertChunkData(zFormat);

						{
							for (size_t keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
							{
								const QuaternionKeyData &keyData = channelData->m_keyData[keyframeIndex];

								iff.insertChunkData(static_cast<int16>(keyData.m_frameNumber));
								iff.insertChunkData(s_compressedRotations[keyframeIndex].getCompressedValue());
							}
						}
					}
					iff.exitChunk(TAG_QCHN);
				}
			}
			iff.exitForm(TAG_AROT);

			//-- write static rotation channel data
			iff.insertChunk(TAG_SROT);
			{
				uint8 xFormat = 0;
				uint8 yFormat = 0;
				uint8 zFormat = 0;

				CompressedQuaternion::getOptimalCompressionFormat(*m_staticRotations, xFormat, yFormat, zFormat);
				//DEBUG_WARNING(true,("(%d,%d,%d) - optimalcompression",xFormat,yFormat,zFormat));

				const size_t valueCount = m_staticRotations->size();
				for (size_t i = 0; i < valueCount; ++i)
				{
					const Quaternion &rotation = (*m_staticRotations)[i];

					// Write format info.
					iff.insertChunkData(xFormat);
					iff.insertChunkData(yFormat);
					iff.insertChunkData(zFormat);

					// Write the compressed rotation.
					iff.insertChunkData(CompressedQuaternion(rotation, xFormat, yFormat, zFormat).getCompressedValue());
				}
			}
			iff.exitChunk(TAG_SROT);

			//-- write animated translation channel data
			iff.insertForm(TAG_ATRN);
			{
				const size_t channelCount = m_translationChannelData->size();
				for (size_t i = 0; i < channelCount; ++i)
				{
					NOT_NULL((*m_translationChannelData)[i]);
					const TranslationChannelData *const channelData = (*m_translationChannelData)[i];

					iff.insertChunk(TAG_CHNL);
					{
						const size_t keyframeCount = channelData->m_keyData.size();
						iff.insertChunkData(static_cast<int16>(keyframeCount));

						for (size_t keyframeIndex = 0; keyframeIndex < keyframeCount; ++keyframeIndex)
						{
							const RealKeyData &keyData = channelData->m_keyData[keyframeIndex];
							iff.insertChunkData(static_cast<int16>(keyData.m_frameNumber));
							iff.insertChunkData(static_cast<float>(keyData.m_keyData));
						}
					}
					iff.exitChunk(TAG_CHNL);
				}
			}
			iff.exitForm(TAG_ATRN);

			//-- write static translation channel data
			iff.insertChunk(TAG_STRN);
			{
				const size_t valueCount = m_staticTranslations->size();
				for (size_t i = 0; i < valueCount; ++i)
				{
					iff.insertChunkData(static_cast<float>((*m_staticTranslations)[i]));
				}
			}
			iff.exitChunk(TAG_STRN);

			//-- write animation messages
			if ((m_animationMessages) && !m_animationMessages->empty())
			{
				iff.insertForm(TAG_MSGS);
				{
					//-- write # animation messages
					iff.insertChunk(TAG_INFO);
						iff.insertChunkData(static_cast<int16>(m_animationMessages->size()));
					iff.exitChunk(TAG_INFO);

					//-- write each animation message
					const AnimationMessageMap::iterator endIt = m_animationMessages->end();
					for (AnimationMessageMap::iterator it = m_animationMessages->begin(); it != endIt; ++it)
						it->second->write(iff);
				}
				iff.exitForm(TAG_MSGS);
			}

			//-- write locomotion translation data
			if (m_locomotionTranslationChannelData && !m_locomotionTranslationChannelData->m_keyData.empty())
			{
				iff.insertChunk(TAG_LOCT);

					// write average translation speed
					iff.insertChunkData(static_cast<float>(m_averageTranslationSpeed));

					// write # keys
					iff.insertChunkData(static_cast<int16>(m_locomotionTranslationChannelData->m_keyData.size()));

					// write key data
					const VectorKeyDataVector::iterator endIt = m_locomotionTranslationChannelData->m_keyData.end();
					for (VectorKeyDataVector::iterator it = m_locomotionTranslationChannelData->m_keyData.begin(); it != endIt; ++it)
					{
						iff.insertChunkData(static_cast<int16>(it->m_frameNumber));
						iff.insertChunkFloatVector(it->m_vector);
					}

				iff.exitChunk(TAG_LOCT);
			}

			//-- write locomotion rotation data
			if (m_locomotionRotationChannelData && !m_locomotionRotationChannelData->m_keyData.empty())
			{
				iff.insertChunk(TAG_QCHN);

					// Collect the uncompressed rotations.
					s_compressedRotations.clear();
					s_rotations.clear();

					{
						const QuaternionKeyDataVector::iterator endIt = m_locomotionRotationChannelData->m_keyData.end();
						for (QuaternionKeyDataVector::iterator it = m_locomotionRotationChannelData->m_keyData.begin(); it != endIt; ++it)
							s_rotations.push_back(it->m_rotation);
					}

					// Determine optimal compression format for this rotation channel.
					uint8  xFormat;
					uint8  yFormat;
					uint8  zFormat;

					CompressedQuaternion::getOptimalCompressionFormat(s_rotations, xFormat, yFormat, zFormat);

					// Compress the values.
					CompressedQuaternion::compressRotations(s_rotations, xFormat, yFormat, zFormat, s_compressedRotations);

					// Write the compressed rotations.
					iff.insertChunkData(static_cast<int16>(m_locomotionRotationChannelData->m_keyData.size()));
					iff.insertChunkData(xFormat);
					iff.insertChunkData(yFormat);
					iff.insertChunkData(zFormat);

					{
						CompressedQuaternionVector::size_type compressedIndex = 0;

						const QuaternionKeyDataVector::iterator endIt = m_locomotionRotationChannelData->m_keyData.end();
						for (QuaternionKeyDataVector::iterator it = m_locomotionRotationChannelData->m_keyData.begin(); it != endIt; ++it, ++compressedIndex)
						{
							iff.insertChunkData(static_cast<int16>(it->m_frameNumber));
							iff.insertChunkData(s_compressedRotations[compressedIndex].getCompressedValue());
						}
					}

				iff.exitChunk(TAG_QCHN);
			}
		}
		iff.exitForm(TAG_0001);
	}
	iff.exitForm(TAG_CKAT);
}

// ======================================================================
