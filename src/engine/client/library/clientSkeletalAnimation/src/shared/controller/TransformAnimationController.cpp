// ======================================================================
//
// TransformAnimationController.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TransformAnimationController.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <vector>

// ======================================================================

class TransformAnimationController::MessageCallbackInfo
{
public:

	class LessIdComparator
	{
	public:
		bool operator()(const MessageCallbackInfo *lhs, const MessageCallbackInfo *rhs) const;
		bool operator()(const MessageCallbackInfo *lhs, int rhs) const;
		bool operator()(int lhs, const MessageCallbackInfo *rhs) const;
	};

public:

	MessageCallbackInfo(int id, AnimationMessageCallback animationMessageCallback, void *context);
	~MessageCallbackInfo();

	int                       getId() const;
	AnimationMessageCallback  getCallback() const; //lint -esym(1763, MessageCallbackInfo::getCallback) // const member indirectly modifies class // acceptable here
	void                     *getContext() const;  //lint -esym(1763, MessageCallbackInfo::getContext)  // const member indirectly modifies class // acceptable here

private:

	// disabled
	MessageCallbackInfo();
	MessageCallbackInfo(const MessageCallbackInfo&);             //lint -esym(754, MessageCallbackInfo::MessageCallbackInfo) // not referenced // preventative
	MessageCallbackInfo &operator =(const MessageCallbackInfo&); //lint -esym(754, MessageCallbackInfo::operator=)           // not referenced // preventative

private:

	int                       m_id;
	AnimationMessageCallback  m_animationMessageCallback;
	void                     *m_context;

};

// ======================================================================

class TransformAnimationController::TransformBookmark::TransformData
{
public:

	TransformData(const CrcLowerString &transformName, const Quaternion &rotation, const Vector &translation);

	const CrcLowerString &getTransformName() const;
	const Quaternion     &getRotation() const;
	const Vector         &getTranslation() const;

private:

	// disabled
	TransformData();
	TransformData(const TransformData&);             //lint -esym(754, TransformData::TransformData) // not referenced // defensive hiding
	TransformData &operator =(const TransformData&); //lint -esym(754, TransformData::operator=)     // not referenced // cannot exist

private:

	const CrcLowerString  m_transformName;
	const Quaternion      m_rotation;
	const Vector          m_translation;

};

// ======================================================================
// class TransformAnimationController::MessageCallbackInfo
// ======================================================================

TransformAnimationController::MessageCallbackInfo::MessageCallbackInfo(int id, AnimationMessageCallback animationMessageCallback, void *context)
:	m_id(id),
	m_animationMessageCallback(animationMessageCallback),
	m_context(context)
{
}

// ----------------------------------------------------------------------

inline TransformAnimationController::MessageCallbackInfo::~MessageCallbackInfo()
{
	m_context = 0;
}

// ----------------------------------------------------------------------

inline int TransformAnimationController::MessageCallbackInfo::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

inline TransformAnimationController::AnimationMessageCallback TransformAnimationController::MessageCallbackInfo::getCallback() const
{
	return m_animationMessageCallback;
}

// ----------------------------------------------------------------------

inline void *TransformAnimationController::MessageCallbackInfo::getContext() const
{
	return m_context;
}

// ======================================================================
// class TransformAnimationController::MessageCallbackInfo::LessIdComparator
// ======================================================================

inline bool TransformAnimationController::MessageCallbackInfo::LessIdComparator::operator()(const MessageCallbackInfo *lhs, const MessageCallbackInfo *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getId() < rhs->getId();
}

// ----------------------------------------------------------------------

inline bool TransformAnimationController::MessageCallbackInfo::LessIdComparator::operator()(const MessageCallbackInfo *lhs, int rhs) const
{
	NOT_NULL(lhs);

	return lhs->getId() < rhs;
}

// ----------------------------------------------------------------------

inline bool TransformAnimationController::MessageCallbackInfo::LessIdComparator::operator()(int lhs, const MessageCallbackInfo *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getId();
}

// ======================================================================
// class TransformAnimationController::TransformBookmark::TransformData
// ======================================================================

TransformAnimationController::TransformBookmark::TransformData::TransformData(const CrcLowerString &transformName, const Quaternion &rotation, const Vector &translation)
:	m_transformName(transformName),
	m_rotation(rotation),
	m_translation(translation)
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &TransformAnimationController::TransformBookmark::TransformData::getTransformName() const
{
	return m_transformName;
}

// ----------------------------------------------------------------------

inline const Quaternion &TransformAnimationController::TransformBookmark::TransformData::getRotation() const
{
	return m_rotation;
}

// ----------------------------------------------------------------------

inline const Vector &TransformAnimationController::TransformBookmark::TransformData::getTranslation() const
{
	return m_translation;
}

// ======================================================================
// class TransformAnimationController::Bookmark
// ======================================================================

TransformAnimationController::Bookmark::~Bookmark()
{
}

// ======================================================================

TransformAnimationController::Bookmark::Bookmark()
{
}

// ======================================================================
// class TransformAnimationController::TransformBookmark
// ======================================================================

TransformAnimationController::TransformBookmark::TransformBookmark(int transformCountHint)
:	Bookmark(),
	m_transformDataVector(new TransformDataVector())
{
	m_transformDataVector->reserve(static_cast<size_t>(transformCountHint));
}

// ----------------------------------------------------------------------

TransformAnimationController::TransformBookmark::~TransformBookmark()
{
	std::for_each(m_transformDataVector->begin(), m_transformDataVector->end(), PointerDeleter());
	delete m_transformDataVector;
}

// ----------------------------------------------------------------------

void TransformAnimationController::TransformBookmark::addTransform(const CrcLowerString &transformName, const Quaternion &rotation, const Vector &translation)
{
	m_transformDataVector->push_back(new TransformData(transformName, rotation, translation));
}

// ----------------------------------------------------------------------

int TransformAnimationController::TransformBookmark::getTransformCount() const
{
	return static_cast<int>(m_transformDataVector->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &TransformAnimationController::TransformBookmark::getTransformName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());
	return (*m_transformDataVector)[static_cast<size_t>(index)]->getTransformName();
}

// ----------------------------------------------------------------------

const Quaternion &TransformAnimationController::TransformBookmark::getRotation(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());
	return (*m_transformDataVector)[static_cast<size_t>(index)]->getRotation();
}

// ----------------------------------------------------------------------

const Vector &TransformAnimationController::TransformBookmark::getTranslation(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTransformCount());
	return (*m_transformDataVector)[static_cast<size_t>(index)]->getTranslation();
}

// ======================================================================
// class TransformAnimationController
// ======================================================================

TransformAnimationController::~TransformAnimationController()
{
	if (m_messageCallbackInfo)
	{
		// NOTE: it is now common for message listeners to remain on the controller.
		//       For example, CreatureObject instances install a animationMessage -> ClientEvent
		//       callback and do not try to remove it prior to destruction.
		// DEBUG_WARNING(!m_messageCallbackInfo->empty(), ("animation controller getting deleted with %u animation message listeners still attached", m_messageCallbackInfo->size()));

		std::for_each(m_messageCallbackInfo->begin(), m_messageCallbackInfo->end(), PointerDeleter());
		delete m_messageCallbackInfo;
	}

	m_transformNameMap = 0;
}

// ----------------------------------------------------------------------
/**
 * Create a snapshot of the animation controller state that can be applied
 * to the controller at a later time.
 *
 * This function is useful when the TransformNameMap is about to be modified.
 * Modifying the transform name map causes controllers to lose animation
 * pose information.  Calling this function before the skeleton modification,
 * then calling applyBookmark() after the modification should prevent sharp
 * hiccups from occuring in the Skeleton.
 *
 * The caller should delete the Bookmark instance when it is no longer needed.
 *
 * There are few guarantees as to how much state is restored by any given
 * controller class.  The caller can expect that the state of the Skeleton
 * transforms will be restored.  No performance guarantees are made about
 * this operation: it likely will be considerably slower than typical
 * animation playback operations.
 *
 * This default implementation does not do anything and returns NULL.  If
 * a controller implementation wants to provide this functionality, it should
 * override this function and applyBookmark().
 *
 * @return  a Bookmark-derived instance that snapshots the controller's
 *          current state.
 *
 * @see TransformAnimationController::applyBookmark()
 */

TransformAnimationController::Bookmark *TransformAnimationController::createBookmark() const
{
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Restore this animation controller instance to the state previously
 * saved in the provided Bookmark.
 *
 * The caller should only pass in a Bookmark instance that was returned by
 * a call to createBookmark() on this instance.  
 *
 * This default implementation does nothing.  If a controller implementation 
 * wants to provide this functionality, it should override this function 
 * and createBookmark().
 *
 * @arg bookmark  contains the state to which the controller should be restored
 *                as closely as possible
 */

void TransformAnimationController::applyBookmark(const Bookmark *bookmark)
{
	UNREF(bookmark);
}

// ----------------------------------------------------------------------

int TransformAnimationController::addAnimationMessageListener(AnimationMessageCallback callback, void *context)
{
	//-- ensure the container exists
	if (!m_messageCallbackInfo)
		m_messageCallbackInfo = new MessageCallbackInfoVector;

#ifdef _DEBUG
	MessageCallbackInfoVector::iterator i;
	for (i = m_messageCallbackInfo->begin(); i != m_messageCallbackInfo->end(); ++i)
	{
		if ((*i)->getCallback() == callback && (*i)->getContext() == context)
		{
			DEBUG_FATAL(true, ("TransformAnimationController adding redundant callback, numcallbacks [%d]", m_messageCallbackInfo->size()));
			break;
		}
	}
#endif

	//-- create the new message callback info
	const int callbackId = m_nextMessageCallbackId++;
	m_messageCallbackInfo->push_back(new MessageCallbackInfo(callbackId, callback, context));

	return callbackId;
}

// ----------------------------------------------------------------------

void TransformAnimationController::removeAnimationMessageListener(int id)
{
	NOT_NULL(m_messageCallbackInfo);

	// note: this routine assumes message callback info instances are always
	//       added in increasing id order.

	//-- search for the specified message callback info
	MessageCallbackInfoVector::iterator findIt = std::lower_bound(m_messageCallbackInfo->begin(), m_messageCallbackInfo->end(), id, MessageCallbackInfo::LessIdComparator());
	if ((findIt == m_messageCallbackInfo->end()) || MessageCallbackInfo::LessIdComparator()(id, *findIt))
	{
		DEBUG_WARNING(true, ("tried to remove non-existent message listener (id = %d)\n", id));
		return;
	}

	//-- remove message callback
	delete *findIt;
	IGNORE_RETURN(m_messageCallbackInfo->erase(findIt));
}

// ----------------------------------------------------------------------

StateHierarchyAnimationController *TransformAnimationController::asStateHierarchyAnimationController()
{
	return NULL;
}

// ----------------------------------------------------------------------

StateHierarchyAnimationController const *TransformAnimationController::asStateHierarchyAnimationController() const
{
	return NULL;
}

// ----------------------------------------------------------------------

TrackAnimationController *TransformAnimationController::asTrackAnimationController()
{
	return NULL;
}

// ----------------------------------------------------------------------
/**
 * Determine if object-level debugging is enabled for this controller.
 *
 * @return true if the object associated with this controller is the
 * skeletal animation debugging focus object or if no focus object
 * is set; false otherwise.
 */

bool TransformAnimationController::isObjectLevelDebuggingEnabled() const
{
	//-- Handle no debugging focus object.
	Object const *focusObject = SkeletalAnimationDebugging::getFocusObject();
	if (!focusObject)
	{
		// There is no focus object, so enable for all objects.
		return true;
	}

	//-- Check if object associated with controller is the focus object.
	SkeletalAppearance2 const *appearance = m_animationEnvironment.getSkeletalAppearance();
	if (!appearance)
	{
		// Not the focus object.
		return false;
	}

	// Object level debugging is enabled if the associated appearance's owner is the animation system's focus object.
	return (appearance->getOwner() == focusObject);
}

// ======================================================================
// protected member functions
// ======================================================================

TransformAnimationController::TransformAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap *transformNameMap) :
	m_animationEnvironment(animationEnvironment),
	m_transformNameMap(NON_NULL(transformNameMap)),
	m_nextMessageCallbackId(1),
	m_messageCallbackInfo(0)
{
}

// ----------------------------------------------------------------------

int TransformAnimationController::getAnimationMessageCallbackCount() const
{
	if (m_messageCallbackInfo)
		return static_cast<int>(m_messageCallbackInfo->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

void TransformAnimationController::getAnimationMessageCallbackInfo(int index, AnimationMessageCallback &callback, void *&context) const
{
	NOT_NULL(m_messageCallbackInfo);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_messageCallbackInfo->size()));

	//-- get the info
	const MessageCallbackInfo *info = (*m_messageCallbackInfo)[static_cast<size_t>(index)];
	NOT_NULL(info);

	//-- return the info
	callback = info->getCallback();
	context  = info->getContext();
}

// ======================================================================
