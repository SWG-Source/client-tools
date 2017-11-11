// ======================================================================
//
// KeyframeSkeletalAnimationTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_KeyframeSkeletalAnimationTemplate_H
#define INCLUDED_KeyframeSkeletalAnimationTemplate_H

// ======================================================================

// NOTE: I'm loading up include dependencies so I can allocate less internally.
//       I think this is acceptable because most code deals with the
//       virtual base class.

#include "clientSkeletalAnimation/KeyframeSkeletalAnimationTemplateDef.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <vector>

class Iff;
class MemoryBlockManager;

// ======================================================================
// lint supression

//1509: (Warning -- base class destructor for class 'xxx' is not virtual)
//lint -esym(1509, RealChannel) // that's right.  it's okay here.

// ======================================================================

class KeyframeSkeletalAnimationTemplate: public SkeletalAnimationTemplate
{
public:

	class TransformInfo;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct RealKeyData
	{
	public:

		RealKeyData();
		RealKeyData(float frameNumber, float keyValue);

	public:

		float  m_frameNumber;
		float  m_keyValue;
		float  m_oneOverDistanceToNextKeyframe;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct QuaternionKeyData
	{
	public:

		QuaternionKeyData();
		QuaternionKeyData(float frameNumber, const Quaternion &rotation);

	public:

		float       m_frameNumber;
		Quaternion  m_rotation;
		float       m_oneOverDistanceToNextKeyframe;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct VectorKeyData
	{
	public:

		VectorKeyData(float frameNumber, const Vector &vector);

	public:

		float   m_frameNumber;
		Vector  m_vector;
		float   m_oneOverDistanceToNextKeyframe;

	private:

		// disabled
		VectorKeyData();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<QuaternionKeyData>::fwd  QuaternionKeyDataVector;
	typedef stdvector<RealKeyData>::fwd        RealKeyDataVector;
	typedef stdvector<VectorKeyData>::fwd      VectorKeyDataVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Message;

	typedef stdvector<int>::fwd    IntVector;
	typedef stdvector<float>::fwd  FloatVector;


public:

	static void  install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

	static Quaternion  computeQuaternionFromKeys(const QuaternionKeyDataVector &keyDataVector, float frameTime, int &startKeyIndex);
	static Vector      computeVectorFromKeys(const VectorKeyDataVector &keyDataVector, float frameTime, int &startKeyIndex);

	// ignore these: they are intended for implementation only
	static void  calculateVectorDistanceToNextFrame(VectorKeyDataVector &keyDataVector);
	static void  calculateQuaternionDistanceToNextFrame(QuaternionKeyDataVector &keyDataVector);
	static void  calculateFloatDistanceToNextFrame(RealKeyDataVector &keyDataVector);

public:

	virtual SkeletalAnimation     *fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const;

	float                          getFramesPerSecond() const;
	int                            getFrameCount() const;

	//-- transform information
	int                            getTransformCount() const;
	const TransformInfo           &getTransformInfo(int index) const;
	int                            getTransformIndex(CrcString const &transformName) const;

	CrcString const               &getName(const TransformInfo &transformInfo) const;

	// rotation information
	bool                           hasAnimatedRotation(const TransformInfo &transformInfo) const;
	const QuaternionKeyDataVector &getAnimatedRotationData(const TransformInfo &transformInfo) const;
	const Quaternion              &getStaticRotation(const TransformInfo &transformInfo) const;

	// translation information
	bool                           animatesTranslation(const TransformInfo &transformInfo) const;
	bool                           animatesTranslationX(const TransformInfo &transformInfo) const;
	bool                           animatesTranslationY(const TransformInfo &transformInfo) const;
	bool                           animatesTranslationZ(const TransformInfo &transformInfo) const;

	const RealKeyDataVector       &getXTranslationChannelData(const TransformInfo &transformInfo) const;
	const RealKeyDataVector       &getYTranslationChannelData(const TransformInfo &transformInfo) const;
	const RealKeyDataVector       &getZTranslationChannelData(const TransformInfo &transformInfo) const;

	float                          getStaticXTranslation(const TransformInfo &transformInfo) const;
	float                          getStaticYTranslation(const TransformInfo &transformInfo) const;
	float                          getStaticZTranslation(const TransformInfo &transformInfo) const;

	// animation message info
	int                            getMessageCount() const;
	const CrcLowerString          &getMessageName(int index) const;
	void                           getSignaledMessages(int beginFrameNumber, int endFrameNumber, IntVector &signaledMessageIndices, FloatVector &signaledMessageFrameNumbers) const;

	const QuaternionKeyDataVector *getLocomotionRotationKeys() const;
	const VectorKeyDataVector     *getLocomotionTranslationKeys() const;

	void                           getLocomotion(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;

	float                          getAverageTranslationSpeed() const;

private:

	struct EvaluateRotationRequest;

	class RotationChannel;
	class SignaledMessageReporter;
	class TranslationChannel;

	typedef stdvector<EvaluateRotationRequest>::fwd  EvaluateRotationRequestVector;
	typedef stdvector<Message*>::fwd                 MessageVector;
	typedef stdvector<Quaternion>::fwd               QuaternionVector;
	typedef stdvector<RotationChannel>::fwd          RotationChannelVector;
	typedef stdvector<TransformInfo*>::fwd           TransformInfoVector;
	typedef stdvector<TranslationChannel>::fwd       TranslationChannelVector;

	typedef stdset<CrcLowerString>::fwd              CrcLowerStringSet;

	typedef void (KeyframeSkeletalAnimationTemplate:: *GetLocomotionFunction)(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;

private:

	static void                       remove();

	static SkeletalAnimationTemplate *create(const CrcLowerString &name, Iff &iff);

	static void                       buildQuaternionKeyframesFromEulers(const EvaluateRotationRequestVector &requestVector, int frameCount, const TranslationChannelVector &animatedEulerRotationChannels, const FloatVector &staticEulerRotationChannels, RotationChannelVector &rotationChannels);
	static void                       buildStaticQuaternionsFromEulers(const EvaluateRotationRequestVector &staticRequestVector, const FloatVector &eulerStaticRotationChannels, QuaternionVector &staticRotations);

private:

	KeyframeSkeletalAnimationTemplate(const CrcLowerString &name, Iff &iff);
	virtual  ~KeyframeSkeletalAnimationTemplate();

	void      load_0002(Iff &iff);
	void      load_0003(Iff &iff);

	void      getLocomotionNone(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;
	void      getLocomotionRotation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;
	void      getLocomotionRotationTranslation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;
	void      getLocomotionTranslation(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const;

	// disabled
	KeyframeSkeletalAnimationTemplate();
	KeyframeSkeletalAnimationTemplate(const KeyframeSkeletalAnimationTemplate&);
	KeyframeSkeletalAnimationTemplate &operator =(const KeyframeSkeletalAnimationTemplate&); 

private:

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;

	static CrcLowerStringSet                *ms_inefficientFormatNameSet;

private:

	float                     m_framesPerSecond;
	int                       m_frameCount;
	TransformInfoVector       m_transformInfoVector;

	RotationChannelVector     m_rotationChannels;
	TranslationChannelVector  m_translationChannels;

	QuaternionVector          m_staticRotations;
	FloatVector               m_staticTranslations;

	MessageVector            *m_messages;

	QuaternionKeyDataVector  *m_locomotionRotationKeys;
	VectorKeyDataVector      *m_locomotionTranslationKeys;
	float                     m_averageTranslationSpeed;

	GetLocomotionFunction     m_getLocomotionFunction;

};

// ======================================================================

inline float KeyframeSkeletalAnimationTemplate::getFramesPerSecond() const
{
	return m_framesPerSecond;
}

// ----------------------------------------------------------------------

inline int KeyframeSkeletalAnimationTemplate::getFrameCount() const
{
	return m_frameCount;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the vector of keys describing the delta rotation occurring
 * during animation playback.
 *
 * @return  the vector of keys describing the delta rotation occurring
 *          during animation playback.  Will be NULL if there is no
 *          rotation changes ocurring during playback of this animation.
 */

inline const KeyframeSkeletalAnimationTemplate::QuaternionKeyDataVector *KeyframeSkeletalAnimationTemplate::getLocomotionRotationKeys() const
{
	return m_locomotionRotationKeys;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the vector of keys describing the delta translation occurring
 * during animation playback.
 *
 * @return  the vector of keys describing the delta translation occurring
 *          during animation playback.  Will be NULL if there is no
 *          translation changes ocurring during playback of this animation.
 */

inline const KeyframeSkeletalAnimationTemplate::VectorKeyDataVector *KeyframeSkeletalAnimationTemplate::getLocomotionTranslationKeys() const
{
	return m_locomotionTranslationKeys;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the change in Object-space rotation and translation that should 
 * be applied to the Object.
 *
 * The change in translation and rotation applied between the begin frame
 * time (fractional frame number) and end frame time is returned relative
 * to object space.  It is assumed the entire rotation is applied first,
 * then the translation.  Application in a reverse order will cause invalid
 * translation to occur.
 *
 * endFrameTime must be >= beginFrameTime.
 *
 * @param beginFrameTime        the fractional frame number of the first frame
 *                              at which to start collecting change in orientation.
 * @param endFrameTime          the fractional frame number of the last frame
 *                              at which to start collecting change in orientation.
 * @param rotationStartKeyIndex     an int which should always be stored and passed
 *                              by the caller.  It is used to optimize the lookup
 *                              of keyframe data when called over sequential blocks
 *                              of time.
 * @param translationStartKeyIndex  an int which should always be stored and passed
 *                              by the caller.  It is used to optimize the lookup
 *                              of keyframe data when called over sequential blocks
 *                              of time.
 * @param rotation              returns the Object-space rotation occurring between
 *                              the begin frame and end frame.
 * @param translation           returns the Object-space translation occurring between
 *                              the begin frame and end frame, assuming the associated
 *                              rotation is first applied.
 */

inline void KeyframeSkeletalAnimationTemplate::getLocomotion(float beginFrameTime, float endFrameTime, int &rotationStartKeyIndex, int &translationStartKeyIndex, Quaternion &rotation, Vector &translation) const
{
	(this->*m_getLocomotionFunction)(beginFrameTime, endFrameTime, rotationStartKeyIndex, translationStartKeyIndex, rotation, translation);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the average translation speed indicated by the animation's
 * translation.
 *
 * Units are considered to be meters per second.
 *
 * @return  the average translation speed indicated by the animation's
 *          translation.
 */

inline float KeyframeSkeletalAnimationTemplate::getAverageTranslationSpeed() const
{
	return m_averageTranslationSpeed;
}

// ======================================================================

#endif
