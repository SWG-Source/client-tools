// ======================================================================
//
// TransformAnimationController.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TransformAnimationController_H
#define INCLUDED_TransformAnimationController_H

// ======================================================================

class AnimationEnvironment;
class AnimationRotationOperation;
class AnimationVectorOperation;
class CrcLowerString;
class Quaternion;
class StateHierarchyAnimationController;
class TrackAnimationController;
class TransformNameMap;
class Vector;

// ======================================================================

class TransformAnimationController
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Contains a snapshot of animation controller state that can be 
	 * applied to the controller at a later time.
	 *
	 * This is an abstract base class from which workhorse implementations
	 * will be derived.
	 *
	 * @see TransformAnimationController::createBookmark()
	 * @see TransformAnimationController::applyBookmark()
	 */

	class Bookmark
	{
	public:

		virtual ~Bookmark() = 0;

	protected:

		Bookmark();

	private:

		// disabled
		Bookmark(const Bookmark&);
		Bookmark &operator =(const Bookmark&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef void (*AnimationMessageCallback)(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController *controller);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	virtual ~TransformAnimationController() = 0;

	virtual void                           alter(real time) = 0;

	/**
	 * Evaluate the rotation and translation, relative to bind pose, for the
	 * specified transform under control of this animation controller.
	 *
	 * @param localTransformIndex  the index of the transform (i.e. joint) to evaluate.
	 * @param rotation             the rotation (relative to bind pose) is returned in this parameter.
	 * @param translation          the translation (relative to bind pose) is returned in this parameter.
	 */
	virtual void  evaluateTransformComponents(int localTransformIndex, Quaternion &rotation, Vector &translation) = 0;

	/**
	 * Retrieve the Object rotation and translation that has occurred, due to animation, 
	 * since the last frame.
	 *
	 * The rotation and translation is relative to the Object's frame of reference.  It is the
	 * rotation that should be applied to the Object due to the animation playing
	 * during the time interval of [current time - last alter time] .. [current time].
	 *
	 * It is assumed that the rotation is applied prior to the translation.
	 *
	 * @return  the rotation to apply to the Object caused by the animation playing
	 *          during the time interval between last frame and this frame.
	 */
	virtual void  getObjectLocomotion(Quaternion &rotation, Vector &translation) const = 0;

	virtual Bookmark                      *createBookmark() const;
	virtual void                           applyBookmark(const Bookmark *bookmark);

	// these generally shouldn't need to be overridden
	virtual int                            addAnimationMessageListener(AnimationMessageCallback callback, void *context);
	virtual void                           removeAnimationMessageListener(int id);

	virtual StateHierarchyAnimationController       *asStateHierarchyAnimationController();
	virtual StateHierarchyAnimationController const *asStateHierarchyAnimationController() const;
	virtual TrackAnimationController                *asTrackAnimationController();

	AnimationEnvironment                  &getAnimationEnvironment();
	const AnimationEnvironment            &getAnimationEnvironment() const;
	const TransformNameMap                &getTransformNameMap() const;

	bool                                   isObjectLevelDebuggingEnabled() const;

protected:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/**
	 * An implementation of Bookmark that stores the name, quaternion
	 * and vector for each transform.
	 *
	 * This is a basic Bookmark implementation that any controller
	 * can use to store transform data.  If a controller wants to
	 * handle the ability to restore more information, it can use
	 * this class as a building block.
	 */

	class TransformBookmark: public Bookmark
	{
	public:

		TransformBookmark(int transformCountHint);
		virtual ~TransformBookmark();

		void                  addTransform(const CrcLowerString &transformName, const Quaternion &rotation, const Vector &translation);

		int                   getTransformCount() const;
		const CrcLowerString &getTransformName(int index) const;
		const Quaternion     &getRotation(int index) const;
		const Vector         &getTranslation(int index) const;

	private:

		class TransformData;

		typedef stdvector<TransformData*>::fwd  TransformDataVector;

	private:

		// disabled
		TransformBookmark(const TransformBookmark&);
		TransformBookmark &operator =(const TransformBookmark&);

	private:

		TransformDataVector *m_transformDataVector;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

protected:

	TransformAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap *transformNameMap);

	int   getAnimationMessageCallbackCount() const;
	void  getAnimationMessageCallbackInfo(int index, AnimationMessageCallback &callback, void *&context) const;

private:

	class MessageCallbackInfo;

	typedef stdvector<MessageCallbackInfo*>::fwd  MessageCallbackInfoVector;

private:

	// disabled
	TransformAnimationController();
	TransformAnimationController(const TransformAnimationController&);
	TransformAnimationController &operator =(const TransformAnimationController&);

private:

	AnimationEnvironment       &m_animationEnvironment;
	const TransformNameMap     *m_transformNameMap;
	int                         m_nextMessageCallbackId;
	MessageCallbackInfoVector  *m_messageCallbackInfo;

};

// ======================================================================

inline AnimationEnvironment &TransformAnimationController::getAnimationEnvironment()
{
	return m_animationEnvironment;
}

// ----------------------------------------------------------------------

inline const AnimationEnvironment &TransformAnimationController::getAnimationEnvironment() const
{
	return m_animationEnvironment;
}

// ----------------------------------------------------------------------

inline const TransformNameMap &TransformAnimationController::getTransformNameMap() const
{
	NOT_NULL(m_transformNameMap);
	return *m_transformNameMap;
}

// ======================================================================

#endif
