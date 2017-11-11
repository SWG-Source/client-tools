// ======================================================================
//
// TransformAnimationResolver.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_TransformAnimationResolver_H
#define INCLUDED_TransformAnimationResolver_H

// ======================================================================

class AnimationNotification;
class AnimationStatePath;
class CrcLowerString;
class CrcString;
class Quaternion;
class SkeletalAppearance2;
class SkeletalAppearanceTemplate;
class SkeletonTemplate;
class TransformAnimationController;
class TransformNameMap;
class Vector;

#include "clientSkeletalAnimation/AnimationStatePath.h"
#include <vector>

// ======================================================================

class TransformAnimationResolver
{
public:

	typedef stdvector<const SkeletonTemplate*>::fwd  SkeletonTemplateVector;

	typedef void (*AnimationMessageCallback)(void *context, const CrcLowerString &animationMessageName, const TransformAnimationController *controller);

public:

	TransformAnimationResolver(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletalAppearance2 &appearance);
	~TransformAnimationResolver();

	void              specifySkeletonTemplates(const SkeletonTemplateVector &skeletonTemplates);

	int               getTransformCount() const;
	bool              findTransformIndex(CrcString const &name, int &transformIndex) const;

	void              getTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation) const;
	void              overrideTransformUntilNextAlter(int transformIndex, const Quaternion &rotation, const Vector &translation);

	void              copyTransformsFrom(const TransformAnimationResolver &sourceResolver);

	void              alter(float deltaTime);
	float             getMostRecentAlterElapsedTime() const;
	void              getObjectLocomotion(Quaternion &rotation, Vector &translation, float elapsedTime) const;

	void              setDestinationState(const AnimationStatePath &destinationStatePath, bool skipTraversal = false, bool skipWithDelay = true);
	void              playAction(const CrcLowerString &actionName, int &primaryControllerAnimationId, bool &primaryControllerAnimationIsAdd, AnimationNotification *notification);
	void              stopPrimaryAction(int animationId, bool animationIsAdd);

	bool              hasActionCompleted(int primaryControllerAnimationId, bool primaryControllerAnimationIsAdd) const;
	void              addAnimationCallback(AnimationMessageCallback callback, void *context);

	bool              isPrimaryPlayOnceTrackActive() const;
	bool              doesPrimaryPlayOnceTrackHaveLocomotionPriority() const;
	int               getPrimaryLoopTrackLocomotionPriority() const;
	int               getPrimaryPlayOnceTrackLocomotionPriority() const;

	bool              hasAnimationController() const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool functions (expensive or not in line with intended general usage of class).
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	int                                 getSkeletonTemplateCount() const;

	TransformAnimationController       *getAnimationController(int skeletonTemplateIndex);
	const TransformAnimationController *getAnimationController(int skeletonTemplateIndex) const;

	const TransformNameMap             *getPrimarySkeletonTransformNameMap() const;
	void                                recreateAnimationControllers(bool destroyExistingControllerFirst = true);

	const SkeletalAppearanceTemplate   &getSkeletalAppearanceTemplate() const;
	SkeletalAppearance2                &getSkeletalAppearance();
	SkeletalAppearance2 const          &getSkeletalAppearance() const;

public:

	static const int  cms_maxSkeletonTemplateCount;

private:

	struct CallbackInfo;

	typedef stdvector<Quaternion>::fwd  QuaternionVector;
	typedef stdvector<Vector>::fwd      VectorVector;

	class SkeletonTemplateData;
	friend class SkeletonTemplateData;

	typedef stdvector<SkeletonTemplateData*>::fwd  SkeletonTemplateDataVector;
	typedef stdvector<CallbackInfo>::fwd           CallbackInfoVector;

private:

	void getSkeletonTemplateData(int globalTransformIndex, SkeletonTemplateData *& skeletonTemplateData) const;

	// disabled
	TransformAnimationResolver();
	TransformAnimationResolver(const TransformAnimationResolver&);
	TransformAnimationResolver &operator =(const TransformAnimationResolver&);

private:

	/// The skeletal appearance template, used to create an animation controller for a particular SkeletonTemplate.
	const SkeletalAppearanceTemplate   &m_appearanceTemplate;

	/// The appearance associated with the resolver.
	SkeletalAppearance2                &m_appearance;

	/// Number of transforms managed by the resolver.
	int                                 m_transformCount;

	/// Per-SkeletonTemplate Data, including: animation controller, index of first transform.
	mutable SkeletonTemplateDataVector *m_skeletonTemplateDataVector;

	/// The delta bind pose rotations for all the SkeletonTemplate instances associated with the TransformAnimationResolver.
	mutable QuaternionVector           *m_rotations;

	/// The delta bind pose translations for all the SkeletonTemplate instances associated with the TransformAnimationResolver.
	mutable VectorVector               *m_translations;

	/// Flag per transform indicating whether transform has been evaluated since last call to alter().
	mutable int8                       *m_transformEvaluatedFlags;

	float                               m_mostRecentAlterElapsedTime;

	AnimationStatePath                  m_mostRecentAnimationStatePath;
	CallbackInfoVector                  m_callbackInfoVector;
};

// ======================================================================
/**
 * Retrieve the number of transforms controlled by this TransformAnimationResolver instance.
 *
 * Note: an Appearance can have more transforms than are controlled by the TransformAnimationResolver.
 * SkeletalAppearance2 mesh generators can provide hardpoints that are associated with the mesh.
 * Those hardpoint transforms do not animate and are not controlled by an animation controller.
 * Those hardpoints are fixed relative to another transform.  The Skeleton instance will contain
 * the hardpoints while the TransformAnimationResolver will not.
 *
 * @return  the number of transforms controlled by this TransformAnimationResolver instance.
 */

inline int TransformAnimationResolver::getTransformCount() const
{
	return m_transformCount;
}

// ======================================================================

inline const SkeletalAppearanceTemplate &TransformAnimationResolver::getSkeletalAppearanceTemplate() const
{
	return m_appearanceTemplate;
}

// ----------------------------------------------------------------------

inline SkeletalAppearance2 &TransformAnimationResolver::getSkeletalAppearance()
{
	return m_appearance;
}

// ----------------------------------------------------------------------

inline SkeletalAppearance2 const &TransformAnimationResolver::getSkeletalAppearance() const
{
	return m_appearance;
}

// ----------------------------------------------------------------------

inline float TransformAnimationResolver::getMostRecentAlterElapsedTime() const
{
	return m_mostRecentAlterElapsedTime;
}

// ======================================================================

#endif
