// ======================================================================
//
// TransformAnimationResolver.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationNotification.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationController.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <limits>
#include <string>

// ======================================================================

const int TransformAnimationResolver::cms_maxSkeletonTemplateCount = 16;

// ======================================================================

struct TransformAnimationResolver::CallbackInfo
{
public:

	CallbackInfo(AnimationMessageCallback callback, void *context);

public:

	AnimationMessageCallback  m_callback;
	void                     *m_context;

private:

	// Disabled.
	CallbackInfo();

};

// ======================================================================
// class TransformAnimationResolver::SkeletonTemplateData
// ======================================================================

class TransformAnimationResolver::SkeletonTemplateData
{
public:

	SkeletonTemplateData(const SkeletalAppearanceTemplate &appearanceTemplate, const SkeletonTemplate &skeletonTemplate, AnimationEnvironment &animationEnvironment, Appearance *ownerAppearance, int channel, AnimationStatePath const &initialPath, CallbackInfoVector const &callbackInfoVector);
	~SkeletonTemplateData();

	bool  isSkeletonTemplate(const SkeletonTemplate &skeletonTemplate) const;

	int   setFirstGlobalTransformIndex(int globalIndex);
	bool  hasGlobalTransformIndex(int globalIndex) const;
	bool  findTransformIndex(CrcString const &transformName, int &globalTransformIndex) const;

	void  alter(float deltaTime);

	void  evaluateTransformComponents(int globalTransformIndex, Quaternion &rotation, Vector &translation);
	void  getObjectLocomotion(Quaternion &rotation, Vector &translation) const;

	void  setDestinationState(const AnimationStatePath &destinationStatePath, bool skipTraversal, bool skipWithDelay);
	void  playAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, int &animationId, bool &animationIsAdd, AnimationNotification *notification);
	void  stopAction(int animationId, bool animationIsAdd);
	bool  hasActionCompleted(int animationId, bool animationIsAdd) const;
	void  addAnimationCallback(AnimationMessageCallback callback, void *context);

	bool  isPlayOnceTrackActive() const;
	bool  doesPlayOnceTrackHaveLocomotionPriority() const;
	int   getLoopTrackLocomotionPriority() const;
	int   getPlayOnceTrackLocomotionPriority() const;

	TransformAnimationController       *getAnimationController();
	const TransformAnimationController *getAnimationController() const;
	const BasicSkeletonTemplate        &getSkeletonTemplate() const;

	void                                recreateAnimationController(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletalAppearance2 &appearance, bool destroyExistingControllerFirst, AnimationStatePath const &initialPath);
	bool                                hasAnimationController() const;

private:

	void  convertTransformIndexGlobalToLocal(int globalTransformIndex, int &localTransformIndex) const;
	void  convertTransformIndexLocalToGlobal(int localTransformIndex,  int &globalTransformIndex) const;
	int   getTransformCount() const;

	// disabled
	SkeletonTemplateData();
	SkeletonTemplateData(const SkeletonTemplateData &rhs);         //lint -esym(754, SkeletonTemplateData::SkeletonTemplateData) // not referenced // defensive hiding.
	SkeletonTemplateData &operator =(const SkeletonTemplateData&); //lint -esym(754, SkeletonTemplateData::operator=) // not referenced // defensive hiding.

private:

	const BasicSkeletonTemplate  *m_skeletonTemplate;
	const SkeletonTemplate       &m_masterSkeletonTemplate;
	TransformAnimationController *m_animationController;
	int                           m_firstGlobalTransformIndex;
	int                           m_channel;

};

// ======================================================================

TransformAnimationResolver::CallbackInfo::CallbackInfo(AnimationMessageCallback callback, void *context) :
	m_callback(callback),
	m_context(context)
{
}

// ======================================================================
// class TransformAnimationResolver::SkeletonTemplateData implementation
// ======================================================================

inline void TransformAnimationResolver::SkeletonTemplateData::convertTransformIndexGlobalToLocal(int globalTransformIndex, int &localTransformIndex) const
{
	localTransformIndex = globalTransformIndex - m_firstGlobalTransformIndex;
}

// ----------------------------------------------------------------------

inline void TransformAnimationResolver::SkeletonTemplateData::convertTransformIndexLocalToGlobal(int localTransformIndex,  int &globalTransformIndex) const
{
	globalTransformIndex = localTransformIndex + m_firstGlobalTransformIndex;
}

// ----------------------------------------------------------------------

inline int TransformAnimationResolver::SkeletonTemplateData::getTransformCount() const
{
	if (m_skeletonTemplate)
		return m_skeletonTemplate->getJointCount();
	else
		return 0;
}

// ----------------------------------------------------------------------

inline TransformAnimationController *TransformAnimationResolver::SkeletonTemplateData::getAnimationController()
{
	return m_animationController;
}

// ----------------------------------------------------------------------

inline const TransformAnimationController *TransformAnimationResolver::SkeletonTemplateData::getAnimationController() const
{
	return m_animationController;
}

// ----------------------------------------------------------------------

inline const BasicSkeletonTemplate &TransformAnimationResolver::SkeletonTemplateData::getSkeletonTemplate() const
{
	NOT_NULL(m_skeletonTemplate);
	return *m_skeletonTemplate;
}

// ======================================================================

TransformAnimationResolver::SkeletonTemplateData::SkeletonTemplateData(const SkeletalAppearanceTemplate &appearanceTemplate, const SkeletonTemplate &skeletonTemplate, AnimationEnvironment &animationEnvironment, Appearance *ownerAppearance, int channel, AnimationStatePath const &initialPath, CallbackInfoVector const &callbackInfoVector) :
	m_skeletonTemplate(skeletonTemplate.fetchBasicSkeletonTemplate(0)),
	m_masterSkeletonTemplate(skeletonTemplate),
	m_animationController(0),
	m_firstGlobalTransformIndex(-1),
	m_channel(channel)
{
	//-- Keep track of master skeleton template.  Name of master template is needed to recreate animation controllers.
	//   This feature is used by the animation editor and will be needed when rebuilding controllers after a player scale
	//   operation.  (Scaling invalidates cached locomotion animations).
	m_masterSkeletonTemplate.fetch();

	//-- Create the animation controller if one exists.
	if (appearanceTemplate.shouldCreateAnimationController())
	{
		m_animationController = appearanceTemplate.createAnimationController(skeletonTemplate.getName(), &m_skeletonTemplate->getTransformNameMap(), animationEnvironment, ownerAppearance, channel, initialPath);

		//-- Apply animation callbacks specified in the callback vector.
		CallbackInfoVector::const_iterator const endIt = callbackInfoVector.end();
		for (CallbackInfoVector::const_iterator it = callbackInfoVector.begin(); it != endIt; ++it)
			addAnimationCallback(it->m_callback, it->m_context);
	}
}

// ----------------------------------------------------------------------

TransformAnimationResolver::SkeletonTemplateData::~SkeletonTemplateData()
{
	delete m_animationController;

	if (m_skeletonTemplate)
	{
		m_skeletonTemplate->release();
		m_skeletonTemplate = 0;
	}

	m_masterSkeletonTemplate.release();
}

// ----------------------------------------------------------------------
/**
 * Check if the given SkeletonTemplate is the same skeleton template as that represented by this SkeletonTemlpateData
 * instance.
 *
 * @param skeletonTemplate  the SkeletonTemplate to compare against this SkeletonTemplateData instance.
 *
 * @return  true if the given SkeletonTemplate is the same skeleton template represented by this
 *          instance, otherwise returns false.
 */

bool TransformAnimationResolver::SkeletonTemplateData::isSkeletonTemplate(const SkeletonTemplate &skeletonTemplate) const
{
	//-- Check this skeleton template against the first detail level of the given skeletonTemplate.
	const BasicSkeletonTemplate *testSkeletonTemplate = skeletonTemplate.fetchBasicSkeletonTemplate(0);
	const bool                   result = (testSkeletonTemplate == m_skeletonTemplate);

	//-- Release local reference.
	testSkeletonTemplate->release();

	return result;
}

// ----------------------------------------------------------------------

int TransformAnimationResolver::SkeletonTemplateData::setFirstGlobalTransformIndex(int globalIndex)
{
	NOT_NULL(m_skeletonTemplate);

	//-- Keep track of first global transform index assigned to the skeleton template.
	m_firstGlobalTransformIndex = globalIndex;

	return m_firstGlobalTransformIndex + getTransformCount();
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::hasGlobalTransformIndex(int globalIndex) const
{
	int localIndex = -1;
	convertTransformIndexGlobalToLocal(globalIndex, localIndex);

	return ((localIndex >= 0) && (localIndex < getTransformCount()));
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::findTransformIndex(CrcString const &transformName, int &globalTransformIndex) const
{
	if (!m_skeletonTemplate)
		return false;

	//-- Check if the SkeletonTemplate defines this joint.
	bool found           = false;
	int  localJointIndex = -1;
	m_skeletonTemplate->findJointIndex(transformName, &localJointIndex, &found);

	if (found)
	{
		//-- Return the global transform index.
		convertTransformIndexLocalToGlobal(localJointIndex, globalTransformIndex);
	}

	//-- Return whether given transform name was found on this SkeletonTemplate.
	return found;
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::alter(float deltaTime)
{
	if (m_animationController)
		m_animationController->alter(deltaTime);
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::evaluateTransformComponents(int globalTransformIndex, Quaternion &rotation, Vector &translation)
{
	//-- Get animation controller to evaluate transform components.
	if (m_animationController)
	{
		//-- Convert global transform index to joint index.
		int localTransformIndex = -1;

		convertTransformIndexGlobalToLocal(globalTransformIndex, localTransformIndex);
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localTransformIndex, getTransformCount());

		m_animationController->evaluateTransformComponents(localTransformIndex, rotation, translation);
	}
	else
	{
		//-- No animation controller, return no delta from bind pose.
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::getObjectLocomotion(Quaternion &rotation, Vector &translation) const
{
	if (m_animationController)
		m_animationController->getObjectLocomotion(rotation, translation);
	else
	{
		//-- No animation controller.
		rotation    = Quaternion::identity;
		translation = Vector::zero;
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::setDestinationState(const AnimationStatePath &destinationStatePath, bool skipTraversal, bool skipWithDelay)
{
	if (m_animationController)
	{
		StateHierarchyAnimationController *const shController = m_animationController->asStateHierarchyAnimationController();

		if (shController)
			shController->setDestinationState(destinationStatePath, skipTraversal, skipWithDelay);
		else
		{
			AnimationEnvironment             &animationEnvironment = m_animationController->getAnimationEnvironment();
			const SkeletalAppearance2 *const &appearance           = animationEnvironment.getSkeletalAppearance();

			if (appearance)
			{
				const AppearanceTemplate * const at = appearance->getAppearanceTemplate();
				if (at)
				{
					WARNING (true, ("Invalid animation controller on '%s'", at->getName ()));
					return;
				}
			}
			WARNING (true, ("Invalid animation controller"));
		}
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::playAction(const CrcLowerString &actionName, const AnimationEnvironment &animationEnvironment, int &animationId, bool &animationIsAdd, AnimationNotification *notification)
{
	if (m_animationController)
	{
		StateHierarchyAnimationController *const shController = m_animationController->asStateHierarchyAnimationController();
		if (shController)
			shController->playAction(actionName, animationEnvironment, animationId, animationIsAdd, notification);
		else
		{
			DEBUG_REPORT_LOG(true, ("tried to play action [%s] on .skt with old style controller; unsupported, skipping.\n", actionName.getString()));
		}
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::stopAction(int animationId, bool animationIsAdd)
{
	if (m_animationController)
	{
		StateHierarchyAnimationController *const shController = m_animationController->asStateHierarchyAnimationController();
		if (shController)
			shController->stopAction(animationId, animationIsAdd);
		else
		{
			DEBUG_REPORT_LOG(true, ("tried to stop action [%s] on .skt with old style controller; unsupported, skipping.\n"));
		}
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::hasActionCompleted(int animationId, bool animationIsAdd) const
{
	if (!m_animationController)
		return true;
	else
	{
		StateHierarchyAnimationController *const shController = m_animationController->asStateHierarchyAnimationController();
		if (shController)
			return shController->hasActionCompleted(animationId, animationIsAdd);
		else
			return true;
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::addAnimationCallback(AnimationMessageCallback callback, void *context)
{
	if (m_animationController && callback)
	{
		//-- Add the animation callback to the animation controller.
		// @todo I can never remove the callback with the current API.  Fix this, possibly via an API that lets the caller walk the callbacks.
		IGNORE_RETURN(m_animationController->addAnimationMessageListener(callback, context));
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::isPlayOnceTrackActive() const
{
	const StateHierarchyAnimationController *const controller = (m_animationController == NULL) ? NULL : m_animationController->asStateHierarchyAnimationController();
	if (!controller)
		return false;

	return controller->isPlayOnceTrackActive();
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::doesPlayOnceTrackHaveLocomotionPriority() const
{
	const StateHierarchyAnimationController *const controller = (m_animationController == NULL) ? NULL : m_animationController->asStateHierarchyAnimationController();
	if (!controller)
		return false;

	return controller->doesPlayOnceTrackHaveLocomotionPriority();
}

// ----------------------------------------------------------------------

int TransformAnimationResolver::SkeletonTemplateData::getLoopTrackLocomotionPriority() const
{
	const StateHierarchyAnimationController *const controller = (m_animationController == NULL) ? NULL : m_animationController->asStateHierarchyAnimationController();
	if (!controller)
		return std::numeric_limits<int>::min();

	return controller->getLoopTrackLocomotionPriority();
}

// ----------------------------------------------------------------------

int TransformAnimationResolver::SkeletonTemplateData::getPlayOnceTrackLocomotionPriority() const
{
	const StateHierarchyAnimationController *const controller = (m_animationController != NULL) ? m_animationController->asStateHierarchyAnimationController() : NULL;
	if (!controller)
		return std::numeric_limits<int>::min();

	return controller->getPlayOnceTrackLocomotionPriority();
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::SkeletonTemplateData::recreateAnimationController(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletalAppearance2 &appearance, bool destroyExistingControllerFirst, AnimationStatePath const &initialPath)
{
	//-- Keep track of the old animation controller.
	TransformAnimationController *oldController = 0;

	if (destroyExistingControllerFirst)
	{
		// Destroy the old animation controller.
		delete m_animationController;
		m_animationController = 0;
	}
	else
	{
		oldController = m_animationController;
	}

	//-- Create the new animation controller.
	if (appearanceTemplate.shouldCreateAnimationController() && m_skeletonTemplate)
		m_animationController = appearanceTemplate.createAnimationController(m_masterSkeletonTemplate.getName(), &m_skeletonTemplate->getTransformNameMap(), appearance.getAnimationEnvironment(), &appearance, m_channel, initialPath);

	//-- Delete the old animation controller.
	if (!destroyExistingControllerFirst)
		delete oldController;
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::SkeletonTemplateData::hasAnimationController() const
{
	return (m_animationController != NULL);
}

// ======================================================================
// class TransformAnimationResolver: public member functions
// ======================================================================

TransformAnimationResolver::TransformAnimationResolver(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletalAppearance2 &appearance) :
	m_appearanceTemplate(appearanceTemplate),
	m_appearance(appearance),
	m_transformCount(0),
	m_skeletonTemplateDataVector(0),
	m_rotations(0),
	m_translations(0),
	m_transformEvaluatedFlags(0),
	m_mostRecentAlterElapsedTime(0.0f),
	m_mostRecentAnimationStatePath(),
	m_callbackInfoVector()
{
	//-- Initialize most recent animation state path to root (standing, non combat).
	m_mostRecentAnimationStatePath.clearPath();
	m_mostRecentAnimationStatePath.appendState(AnimationStateNameIdManager::getRootId());

	//-- Keep local reference.
	IGNORE_RETURN(AppearanceTemplateList::fetch(&m_appearanceTemplate));
}

// ----------------------------------------------------------------------

TransformAnimationResolver::~TransformAnimationResolver()
{
	delete [] m_transformEvaluatedFlags;
	delete m_translations;
	delete m_rotations;

	if (m_skeletonTemplateDataVector)
	{
		std::for_each(m_skeletonTemplateDataVector->begin(), m_skeletonTemplateDataVector->end(), PointerDeleter());
		delete m_skeletonTemplateDataVector;
	}

	//-- Release local reference.
	AppearanceTemplateList::release(&m_appearanceTemplate);
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::specifySkeletonTemplates(const SkeletonTemplateVector &skeletonTemplates)
{
	const SkeletonTemplateVector::size_type  newSkeletonTemplateCount = skeletonTemplates.size();

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(newSkeletonTemplateCount), cms_maxSkeletonTemplateCount);

	//-- Walk through existing SkeletonTemplateData entries, removing any that are not present in the newly specified set of skeleton templates.
	//   Also take note of which ones already exist so I don't need to recreate them.

	// Default all skeletonTemplate entries to require addition.
	int8  addSkeletonTemplateFlags[cms_maxSkeletonTemplateCount];
	memset(addSkeletonTemplateFlags, 1, newSkeletonTemplateCount);

	if (!m_skeletonTemplateDataVector)
	{
		//-- SkeletonTemplateDataVector doesn't exist, create it.
		m_skeletonTemplateDataVector = new SkeletonTemplateDataVector();
	}
	else
	{
		//-- SkeletonTemplateDataVector already existed, check desired SkeletonTemplate instances vs. what is already present.
		bool deleteDataEntry;

		SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
		for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt;)
		{
			//-- Check this SkeletonTemplateData vector against all new skeletonTemplates.
			deleteDataEntry = true;

			for (SkeletonTemplateVector::size_type i = 0; i < newSkeletonTemplateCount; ++i)
			{
				if ((*it)->isSkeletonTemplate(*NON_NULL(skeletonTemplates[i])))
				{
					//-- The existing SkeletonTemplateData entry refers to a SkeletonTemplate that should be present
					//   (and therefore preserved --- don't delete it --- animation controller will not require a reset).
					deleteDataEntry = false;

					//-- Don't need to create an entry for this skeleton template --- one already exists, I'll just use that one.
					addSkeletonTemplateFlags[i] = 0;
					break;
				}
			}

			//-- Delete data entry as necessary.  Increment it as appropriate.
			if (deleteDataEntry)
			{
				it    = m_skeletonTemplateDataVector->erase(it);
				endIt = m_skeletonTemplateDataVector->end();
			}
			else
				++it;
		}
	}

	//-- Create SkeletonTemplateData entries for all SkeletonTemplate instances for which a SkeletonTemplateData
	//   is not already in existence.
	m_skeletonTemplateDataVector->reserve(newSkeletonTemplateCount);

	for (SkeletonTemplateVector::size_type i = 0; i < newSkeletonTemplateCount; ++i)
	{
		if (addSkeletonTemplateFlags[i])
		{
			const SkeletonTemplate *skeletonTemplate = skeletonTemplates[i];
			NOT_NULL(skeletonTemplate);

			int const channel = static_cast<int>(m_skeletonTemplateDataVector->size());
			m_skeletonTemplateDataVector->push_back(new SkeletonTemplateData(m_appearanceTemplate, *skeletonTemplate, m_appearance.getAnimationEnvironment(), &m_appearance, channel, m_mostRecentAnimationStatePath, m_callbackInfoVector));
		}
	}

	//-- Assign the starting global transform index for each SkeletonTemplateData.
	int startingGlobalTransformIndex = 0;

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		startingGlobalTransformIndex = (*it)->setFirstGlobalTransformIndex(startingGlobalTransformIndex);
	}

	//-- Keep track of transform count.
	const bool sizeChanged = (startingGlobalTransformIndex != m_transformCount);
	m_transformCount       = startingGlobalTransformIndex;

	//-- Resize data arrays for transform count.
	if (sizeChanged)
	{
		//-- Create/resize rotation array.
		if (!m_rotations)
			m_rotations = new QuaternionVector(static_cast<QuaternionVector::size_type>(m_transformCount), Quaternion::identity);
		else
			m_rotations->resize(static_cast<QuaternionVector::size_type>(m_transformCount));

		//-- Create/resize translation array.
		if (!m_translations)
			m_translations = new VectorVector(static_cast<VectorVector::size_type>(m_transformCount), Vector::zero);
		else
			m_translations->resize(static_cast<VectorVector::size_type>(m_transformCount));

		//-- Recreate transform evaluation flags, set to not-evaluated.
		delete [] m_transformEvaluatedFlags;

		m_transformEvaluatedFlags = new int8[static_cast<size_t>(m_transformCount)];
		NOT_NULL(m_transformEvaluatedFlags);

		memset(m_transformEvaluatedFlags, 0, static_cast<size_t>(m_transformCount));
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::findTransformIndex(CrcString const &name, int &transformIndex) const
{
	if (!m_skeletonTemplateDataVector)
		return false;

	//-- Scan SkeletonTemplate instances for one that has the given transform name.
	const SkeletonTemplateDataVector::const_iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::const_iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		if ((*it)->findTransformIndex(name, transformIndex))
			return true;
	}

	//-- Not found.
	return false;
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::getTransformComponents(int transformIndex, Quaternion &rotation, Vector &translation) const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("TransformAnimationResolver::getTransformComponents");

	//-- Check for no animation controller situation (e.g. for a wearable).
	if (!m_rotations || !m_translations)
	{
		// Must be no animation controllers.  Return no delta from bind pose.
		rotation    = Quaternion::identity;
		translation = Vector::zero;

		return;
	}

	//-- Check if the transform's rotation needs to be evaluated.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, m_transformCount);

	//-- Evaluate transform data if necessary.
	NOT_NULL(m_transformEvaluatedFlags);
	if (!m_transformEvaluatedFlags[transformIndex])
	{
		// Get the associated skeleton template data.
		SkeletonTemplateData *stData = 0;

		getSkeletonTemplateData(transformIndex, stData);
		NOT_NULL(stData);

		// Evaluate the transform components.
		stData->evaluateTransformComponents(transformIndex, (*m_rotations)[static_cast<QuaternionVector::size_type>(transformIndex)], (*m_translations)[static_cast<VectorVector::size_type>(transformIndex)]);

		// Mark transform as evaluated since last alter.
		m_transformEvaluatedFlags[transformIndex] = 1;
	}

	//-- Return evaluated components.
	rotation    = (*m_rotations)[static_cast<QuaternionVector::size_type>(transformIndex)];
	translation = (*m_translations)[static_cast<VectorVector::size_type>(transformIndex)];
}

// ----------------------------------------------------------------------
/**
 * Override the rotation and translation value for a given transform
 * for this frame only.
 *
 * This function will override the the transform's rotation and translation
 * for this frame.
 *
 * @param transformIndex  the 0-based index of the transform to modify.
 * @param rotation        the rotational component for this transform this frame.
 * @param translation     the translational component for this transform this frame.
 */

void TransformAnimationResolver::overrideTransformUntilNextAlter(int transformIndex, const Quaternion &rotation, const Vector &translation)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformIndex, m_transformCount);

	if (!m_rotations || !m_translations)
	{
		DEBUG_WARNING(true, ("Tried to override a transform on an appearance that doesn't have any animation controllers and (thus) doesn't have any transforms to override."));
		return;
	}

	//-- Override the values.  The values will remain overridden until the next alter() call.
	(*m_rotations)[static_cast<QuaternionVector::size_type>(transformIndex)] = rotation;
	(*m_translations)[static_cast<VectorVector::size_type>(transformIndex)]  = translation;

	//-- Set the evaluated flag to true.
	NOT_NULL(m_transformEvaluatedFlags);
	m_transformEvaluatedFlags[transformIndex] = 1;
}

// ----------------------------------------------------------------------

/**
 * Copy transforms from another resolver into this instance.
 *
 * The source and destination resolver do not need to have an identical
 * set of transforms present.  Only the intersection of transforms
 * present in both resolvers (source and destination) are modified.
 *
 * @param sourceResolver  resolver instance from which transform data
 *                        will be copied.  Data is copied into this instance.
 */

void TransformAnimationResolver::copyTransformsFrom(const TransformAnimationResolver &sourceResolver)
{
	//-- Validate preconditions.
	if (!m_skeletonTemplateDataVector || !m_rotations || !m_translations || !m_transformEvaluatedFlags)
		return;

	int localTransformIndex  = 0;
	int sourceTransformIndex = -1;

	//-- Loop over each of the skeleton templates and lookup target index.
	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		// Get the skeleton template's transform name map.
		const TransformNameMap &transformNameMap = (*it)->getSkeletonTemplate().getTransformNameMap();

		// For each entry in the map, lookup the index in the source transform name map.  If it has an entry, set it.
		const int templateTransformCount = transformNameMap.getTransformCount();
		for (int i = 0; i < templateTransformCount; ++i, ++localTransformIndex)
		{
			if (sourceResolver.findTransformIndex(transformNameMap.getTransformName(i), sourceTransformIndex))
			{
				// Source has a transform entry for this local transform, get it.
				sourceResolver.getTransformComponents(sourceTransformIndex, (*m_rotations)[static_cast<QuaternionVector::size_type>(localTransformIndex)], (*m_translations)[static_cast<VectorVector::size_type>(localTransformIndex)]);

				// Mark local transform as evaluated so it doesn't get replaced until after next alter() call.
				m_transformEvaluatedFlags[localTransformIndex] = 0;
			}
		}
	}
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::alter(float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("TransformAnimationResolver::alter");

	//-- Set all transforms to be unevaluated.  All transforms need to be
	//   re-evaluated after the animation controllers are altered because
	//   new animation has (potentially) taken place.  The artifact: calls
	//   to TransformAnimationResolver::getRotation()/getTranslation()
	//   prior to alter will return different values than post-alter during
	//   any given frame.
	if (m_transformEvaluatedFlags)
	{
		// Reset all evaluation flags.
		memset(m_transformEvaluatedFlags, 0, static_cast<size_t>(m_transformCount));
	}

	//-- Alter all animation controllers.  Each SkeletonTemplate associated
	//   with this resolver may have its own animation controller.
	if (m_skeletonTemplateDataVector)
	{
		const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
		for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->alter(deltaTime);
		}
	}

	//-- Save the most recent alter elapsed time so other entities (e.g. skeleton) can learn about it.
	m_mostRecentAlterElapsedTime = deltaTime;
}

// ----------------------------------------------------------------------

void TransformAnimationResolver::getObjectLocomotion(Quaternion &rotation, Vector &translation, float elapsedTime) const
{
	// For now, retrieve the locomotion from the first skeleton template data.  Presumably this will be
	// the body skeleton which will have locomotion.  Later we will need a way to indicate which .skt gives
	// us locomotion.
	if (m_skeletonTemplateDataVector && !m_skeletonTemplateDataVector->empty())
		(*m_skeletonTemplateDataVector->front()).getObjectLocomotion(rotation, translation);
	else
	{
		//-- Apply translation at requested rate.  There is no animations to play, so failing to do this
		//   here would prevent movement for players in 1st person only mode where no avatar yet exists.
		rotation    = Quaternion::identity;
		translation = elapsedTime * m_appearance.getDesiredVelocity();
	}
}

// ----------------------------------------------------------------------
/**
 * Tells each animation controller (one per .skt) to transition to the given
 * destination state path (if not already there).
 *
 * @param destinationStatePath  the path of the state in which the controller
 *                              should be.
 */

void TransformAnimationResolver::setDestinationState(const AnimationStatePath &destinationStatePath, bool skipTraversal, bool skipWithDelay)
{
	//-- Keep track of the most recent animation state path that is set so we can use it as default state for initializing new controllers.  Needed
	//   for asyncronous loading.
	destinationStatePath.copyTo(m_mostRecentAnimationStatePath);

	if (!m_skeletonTemplateDataVector)
		return;

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		(*it)->setDestinationState(destinationStatePath, skipTraversal, skipWithDelay);
	}
}

// ----------------------------------------------------------------------
/**
 * Tells each animation controller (one per .skt) to play the specified
 * action.
 *
 * @param actionName  name of the action to play.
 */

void TransformAnimationResolver::playAction(const CrcLowerString &actionName, int &primaryControllerAnimationId, bool &primaryControllerAnimationIsAdd, AnimationNotification *notification)
{
	//-- Set notification's channel count.
	if (notification)
		notification->setChannelCount(m_skeletonTemplateDataVector ? static_cast<int>(m_skeletonTemplateDataVector->size()) : 0);

	if (!m_skeletonTemplateDataVector)
		return;

	const AnimationEnvironment &animationEnvironment = m_appearance.getAnimationEnvironment();

	int  index = 0;
	int  dummyAnimationId;
	bool dummyIsAdd;

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it, ++index)
	{
		NOT_NULL(*it);

		if (index == 0)
			(*it)->playAction(actionName, animationEnvironment, primaryControllerAnimationId, primaryControllerAnimationIsAdd, notification);
		else
			(*it)->playAction(actionName, animationEnvironment, dummyAnimationId, dummyIsAdd, notification);
	}
}

// ----------------------------------------------------------------------
/**
 * Stop the specified animation action from playing on the primary (body)
 * animation controller.
 *
 * @param animationId     the animationId value returned by playAction().
 * @param animationIsAdd  the animationIsAdd value returned by playAction().
 */

void TransformAnimationResolver::stopPrimaryAction(int animationId, bool animationIsAdd)
{
	if (m_skeletonTemplateDataVector && !m_skeletonTemplateDataVector->empty())
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		m_skeletonTemplateDataVector->front()->stopAction(animationId, animationIsAdd);
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::hasActionCompleted(int primaryControllerAnimationId, bool primaryControllerAnimationIsAdd) const
{
	if (!m_skeletonTemplateDataVector)
		return true;

	if (m_skeletonTemplateDataVector->empty())
		return true;
	else
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		return m_skeletonTemplateDataVector->front()->hasActionCompleted(primaryControllerAnimationId, primaryControllerAnimationIsAdd);
	}
}

// ----------------------------------------------------------------------
/**
 * Add the specified callback to each skeleton segment's animation controller.
 *
 * @param callback  the callback to add to the animation controller.
 * @param context   the context for the callback.
 */

void TransformAnimationResolver::addAnimationCallback(AnimationMessageCallback callback, void *context)
{

	//-- If this callback is already installed, do nothing.
	CallbackInfoVector::iterator i;
	for (i = m_callbackInfoVector.begin(); i != m_callbackInfoVector.end(); ++i)
	{
		if (i->m_callback == callback && i->m_context == context)
		{
			return;
		}
	}
	
	//-- Keep track of all added callbacks.  We'll need to apply these to any newly created skeleton templates.
	m_callbackInfoVector.push_back(CallbackInfo(callback, context));

	if (!m_skeletonTemplateDataVector)
		return;

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		(*it)->addAnimationCallback(callback, context);
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::isPrimaryPlayOnceTrackActive() const
{
	if (!m_skeletonTemplateDataVector)
		return false;

	if (m_skeletonTemplateDataVector->empty())
		return false;
	else
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		return m_skeletonTemplateDataVector->front()->isPlayOnceTrackActive();
	}
}

// ----------------------------------------------------------------------

bool TransformAnimationResolver::doesPrimaryPlayOnceTrackHaveLocomotionPriority() const
{
	if (!m_skeletonTemplateDataVector)
		return false;

	if (m_skeletonTemplateDataVector->empty())
		return false;
	else
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		return m_skeletonTemplateDataVector->front()->doesPlayOnceTrackHaveLocomotionPriority();
	}
}

// ----------------------------------------------------------------------

int TransformAnimationResolver::getPrimaryLoopTrackLocomotionPriority() const
{
	if (!m_skeletonTemplateDataVector || m_skeletonTemplateDataVector->empty())
		return std::numeric_limits<int>::min();
	else
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		return m_skeletonTemplateDataVector->front()->getLoopTrackLocomotionPriority();
	}
}

// ----------------------------------------------------------------------

int TransformAnimationResolver::getPrimaryPlayOnceTrackLocomotionPriority() const
{
	if (!m_skeletonTemplateDataVector || m_skeletonTemplateDataVector->empty())
		return std::numeric_limits<int>::min();
	else
	{
		NOT_NULL(m_skeletonTemplateDataVector->front());
		return m_skeletonTemplateDataVector->front()->getPlayOnceTrackLocomotionPriority();
	}
}

// ----------------------------------------------------------------------
/**
 * Return true if any of the skeletons have an animation controller.
 *
 * @return  true if any of the skeletons have an animation controller; false otherwise.
 */

bool TransformAnimationResolver::hasAnimationController() const
{
	if (!m_skeletonTemplateDataVector)
		return false;

	const SkeletonTemplateDataVector::const_iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::const_iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);

		const bool hasController = (*it)->hasAnimationController();
		if (hasController)
			return true;
	}

	//-- No animation controllers.
	return false;
}

// ----------------------------------------------------------------------
/**
 * Return the number of skeleton templates associated with this
 * TransformAnimationResolver.
 *
 * @return  the number of skeleton templates associated with this
 *          TransformAnimationResolver.
 */

int TransformAnimationResolver::getSkeletonTemplateCount() const
{
	if (!m_skeletonTemplateDataVector)
		return 0;
	else
		return static_cast<int>(m_skeletonTemplateDataVector->size());
}

// ----------------------------------------------------------------------
/**
 * Returns the animation controller for the specified skeleton template.
 *
 * Index 0 should be the primary (body) controller.
 *
 * @return  the animation controller for the specified skeleton template.
 *          Can be NULL if there is no animation controller for the specified
 *          skeleton template.
 */

TransformAnimationController *TransformAnimationResolver::getAnimationController(int skeletonTemplateIndex)
{
	if ((skeletonTemplateIndex < 0) || (skeletonTemplateIndex >= getSkeletonTemplateCount()))
		return 0;

	NOT_NULL(m_skeletonTemplateDataVector);
	return (*m_skeletonTemplateDataVector)[static_cast<SkeletonTemplateDataVector::size_type>(skeletonTemplateIndex)]->getAnimationController();
}

// ----------------------------------------------------------------------
/**
 * Returns the const animation controller for the specified skeleton template.
 *
 * Index 0 should be the primary (body) controller.
 *
 * @return  the const animation controller for the specified skeleton template.
 *          Can be NULL if there is no animation controller for the specified
 *          skeleton template.
 */

const TransformAnimationController *TransformAnimationResolver::getAnimationController(int skeletonTemplateIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, skeletonTemplateIndex, getSkeletonTemplateCount());
	NOT_NULL(m_skeletonTemplateDataVector);

	return (*m_skeletonTemplateDataVector)[static_cast<SkeletonTemplateDataVector::size_type>(skeletonTemplateIndex)]->getAnimationController();
}

// ----------------------------------------------------------------------
/**
 * Tries to return the transform animation map for the primary (i.e. body)
 * skeleton.
 *
 * @return  the animation controller for the primary (i.e. body) skeleton.
 *          Can be NULL if there is no animation controller.
 */

const TransformNameMap *TransformAnimationResolver::getPrimarySkeletonTransformNameMap() const
{
	if (m_skeletonTemplateDataVector && !m_skeletonTemplateDataVector->empty())
		return &((*m_skeletonTemplateDataVector->front()).getSkeletonTemplate().getTransformNameMap());
	else
		return 0;
}

// ----------------------------------------------------------------------
/**
 * Recreate the animation controllers for each attached skeleton.
 *
 * @param destroyExistingControllerFirst  if true, destroys the existing controller
 *                                        first.  In this case, it is possible that
 *                                        loaded animation templates will get unloaded.
 *                                        During game play, this should be set false so
 *                                        that animation templates referenced by the old
 *                                        animation controller are not unloaded.
 */

void TransformAnimationResolver::recreateAnimationControllers(bool destroyExistingControllerFirst)
{
	//-- Recreate animation controllers.
	if (!m_skeletonTemplateDataVector)
		return;

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		(*it)->recreateAnimationController(m_appearanceTemplate, m_appearance, destroyExistingControllerFirst, m_mostRecentAnimationStatePath);
	}

	//-- Reset evaluation flags.
	if (m_transformEvaluatedFlags)
	{
		memset(m_transformEvaluatedFlags, 0, static_cast<size_t>(m_transformCount));
	}

	//-- Alter the appearance so the bind pose is not used on the next frame.
	IGNORE_RETURN(m_appearance.alter(0.05f));
}

// ======================================================================

void TransformAnimationResolver::getSkeletonTemplateData(int globalTransformIndex, SkeletonTemplateData *& skeletonTemplateData) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, globalTransformIndex, m_transformCount);
	NOT_NULL(m_skeletonTemplateDataVector);

	const SkeletonTemplateDataVector::iterator endIt = m_skeletonTemplateDataVector->end();
	for (SkeletonTemplateDataVector::iterator it = m_skeletonTemplateDataVector->begin(); it != endIt; ++it)
	{
		if ((*it)->hasGlobalTransformIndex(globalTransformIndex))
		{
			skeletonTemplateData = *it;
			return;
		}
	}

	//-- Indicate the specified global index was not found.
	skeletonTemplateData = 0;
}

// ======================================================================
