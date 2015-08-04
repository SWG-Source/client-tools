// ======================================================================
//
// KeyframeSkeletalAnimationTemplateWriter.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_KeyframeSkeletalAnimationTemplateWriter_H
#define INCLUDED_KeyframeSkeletalAnimationTemplateWriter_H

// ======================================================================

class CrcLowerString;
class Iff;
class LessPointerComparator;
class Quaternion;
class Vector;

// ======================================================================

class KeyframeSkeletalAnimationTemplateWriter
{
public:

	struct RotationChannelData;
	struct TransformData;
	struct TranslationChannelData;
	struct VectorChannelData;

public:

	static void setCompressionMode(bool enabled);

public:

	KeyframeSkeletalAnimationTemplateWriter();
	~KeyframeSkeletalAnimationTemplateWriter();

	void write(Iff &iff) const;

	void setFramesPerSecond(real framesPerSecond);
	void setAnimationFrameCount(int animationFrameCount);

	void setAverageTranslationSpeed(float averageTranslationSpeed);

	TransformData &addTransform(const char *name);

	void                 setStaticRotation(TransformData &transformData, const Quaternion &rotation);
	RotationChannelData &addAnimatedRotation(TransformData &transformData);
	RotationChannelData &getLocomotionRotationChannelData();
	void                 addRotationKey(RotationChannelData &channelData, int keyFrame, const Quaternion &rotation) const;

	// -TRF- I think all the x,y,z channel data should be converted into a single
	//       VectorChannel.  It is infrequent that the artists are modifying the translation,
	//       and the code simplification would warrant the merge.  I could then combine the
	//       translation channel handling with the locomotion translation handling, similar
	//       to the animation / locomotion rotation handling above.
	void setStaticTranslationX(TransformData &transformData, real translation);
	void setStaticTranslationY(TransformData &transformData, real translation);
	void setStaticTranslationZ(TransformData &transformData, real translation);

	TranslationChannelData &addChannelTranslationX(TransformData &transformData);
	TranslationChannelData &addChannelTranslationY(TransformData &transformData);
	TranslationChannelData &addChannelTranslationZ(TransformData &transformData);

	void addTranslationKey(TranslationChannelData &channelData, int keyFrame, real keyData) const;

	VectorChannelData &getLocomotionTranslationChannelData();
	void               addVectorKey(VectorChannelData &channelData, int keyFrame, const Vector &keyData) const;

	void beginAnimationMessage(const CrcLowerString &messageName);
	void addAnimationMessageSignal(int keyFrameNumber);
	void endAnimationMessage();

private:

	class  AnimationMessage;
	struct QuaternionKeyData;
	struct RealKeyData;
	struct VectorKeyData;

	typedef stdmap<const CrcLowerString *, AnimationMessage*, LessPointerComparator>::fwd  AnimationMessageMap;

	typedef stdvector<float>::fwd                                                          FloatVector;
	typedef stdvector<Quaternion>::fwd                                                     QuaternionVector;
	typedef stdvector<QuaternionKeyData>::fwd                                              QuaternionKeyDataVector;
	typedef stdvector<RealKeyData>::fwd                                                    RealKeyDataVector;
	typedef stdvector<RotationChannelData*>::fwd                                           RotationChannelDataVector;
	typedef stdvector<TransformData*>::fwd                                                 TransformDataVector;
	typedef stdvector<TranslationChannelData*>::fwd                                        TranslationChannelDataVector;
	typedef stdvector<VectorKeyData>::fwd                                                  VectorKeyDataVector;

private:

	void writeUncompressed(Iff &iff) const;
	void writeCompressed(Iff &iff) const;

	// disabled
	KeyframeSkeletalAnimationTemplateWriter(const KeyframeSkeletalAnimationTemplateWriter&);
	KeyframeSkeletalAnimationTemplateWriter &operator =(const KeyframeSkeletalAnimationTemplateWriter&);

private:

	float                         m_framesPerSecond;
	int                           m_animationFrameCount;

	float                         m_averageTranslationSpeed;

	TransformDataVector          *m_transformData;

	RotationChannelDataVector    *m_rotationChannelData;
	TranslationChannelDataVector *m_translationChannelData;
	QuaternionVector             *m_staticRotations;
	FloatVector                  *m_staticTranslations;

	AnimationMessageMap          *m_animationMessages;
	AnimationMessage             *m_currentAnimationMessage;

	RotationChannelData          *m_locomotionRotationChannelData;
	VectorChannelData            *m_locomotionTranslationChannelData;

};

// ======================================================================
/**
 * Set the average translation speed in meters per second.
 *
 * @param averageTranslationSpeed  average translation speed of the animation
 *                                 in meters per second.
 */

inline void KeyframeSkeletalAnimationTemplateWriter::setAverageTranslationSpeed(float averageTranslationSpeed)
{
	m_averageTranslationSpeed = averageTranslationSpeed;
}

// ======================================================================

#endif
