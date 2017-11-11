// ======================================================================
//
// TrackAnimationController.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationNotification.h"
#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/TrackAnimationController_AddAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_AnimationFactory.h"
#include "clientSkeletalAnimation/TrackAnimationController_PhysicalTrackTemplate.h"
#include "clientSkeletalAnimation/TrackAnimationController_PriorityBlendAnimation.h"
#include "clientSkeletalAnimation/TrackAnimationController_Track.h"
#include "clientSkeletalAnimation/TrackAnimationController_TrackTemplate.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/Appearance.h"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

// ======================================================================

#define DO_VALIDATE_CHANNEL_INDEX 1

#if DO_VALIDATE_CHANNEL_INDEX != 0
#  define VALIDATE_CHANNEL_INDEX() validateChannelIndex()
#else
#  define VALIDATE_CHANNEL_INDEX() NOP
#endif

// ======================================================================

namespace TrackAnimationControllerNamespace
{
	typedef std::vector<float>  FloatVector;

	FloatVector  s_elapsedTimeSinceMessageTriggers;
	int          s_maxAnimationChannelCount;
}

using namespace TrackAnimationControllerNamespace;

// ======================================================================
// class TrackAnimationController: static member variables
// ======================================================================

const ConstCharCrcLowerString                        TrackAnimationController::cms_loopTrackName("loop");
const ConstCharCrcLowerString                        TrackAnimationController::cms_addTrackName("add");
const ConstCharCrcLowerString                        TrackAnimationController::cms_actionTrackName("action");
const ConstCharCrcLowerString                        TrackAnimationController::cms_addCombinerTrackName("add_combiner");
const ConstCharCrcLowerString                        TrackAnimationController::cms_actionCombinerTrackName("action_combiner");
const ConstCharCrcLowerString                        TrackAnimationController::cms_logicalRootTrackName("logical_root");
const ConstCharCrcLowerString                        TrackAnimationController::cms_logicalPrimaryTrackName("logical_primary");

bool                                                 TrackAnimationController::ms_installed;
TrackAnimationController::TrackTemplateVector        TrackAnimationController::ms_trackTemplates;
TrackAnimationController::TrackTemplateMap          *TrackAnimationController::ms_trackTemplateMap;

AnimationTrackId                                     TrackAnimationController::ms_loopTrackId;
AnimationTrackId                                     TrackAnimationController::ms_addTrackId;
AnimationTrackId                                     TrackAnimationController::ms_actionTrackId;
AnimationTrackId                                     TrackAnimationController::ms_addCombinerTrackId;
AnimationTrackId                                     TrackAnimationController::ms_actionCombinerTrackId;
AnimationTrackId                                     TrackAnimationController::ms_logicalRootTrackId;    // points to logicalPrimaryTrack (when no add track playing) or addCombiner track (when add track is playing).
AnimationTrackId                                     TrackAnimationController::ms_logicalPrimaryTrackId; // points to looping (when no action playing) or action combiner track (when action playing).

int                                                  TrackAnimationController::ms_physicalTrackCount;
int                                                  TrackAnimationController::ms_logicalTrackCount;

TrackAnimationController::TrackIdVector              TrackAnimationController::ms_logicalTrackIdByResolverIndex;

TrackAnimationController::ConstCrcLowerStringVector  TrackAnimationController::ms_emittedMessageNames;
TrackAnimationController::IntVector                  TrackAnimationController::ms_emittedMessageIndices;

TrackAnimationController::IntVector                  TrackAnimationController::ms_physicalTrackTemplateByTrackIndex;

bool                                                 TrackAnimationController::ms_logAnimationMessages;
bool                                                 TrackAnimationController::ms_logUnappliedAnimationTime;

#ifdef _DEBUG
int                                                  TrackAnimationController::ms_controllerCreationCount;
#endif

// ======================================================================

class TrackAnimationController::LogicalTrackTemplate: public TrackAnimationController::TrackTemplate
{
public:

	LogicalTrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId, int resolverIndex);

	virtual bool  hasTrackIndexResolver() const;
	virtual int   getTrackIndexResolverIndex() const;
	virtual int   getPhysicalTrackIndex() const;

private:

	// disabled
	LogicalTrackTemplate();
	LogicalTrackTemplate(const LogicalTrackTemplate&); 	           //lint -esym(754, LogicalTrackTemplate::LogicalTrackTemplate)  // unreferenced // defensive hiding
 	LogicalTrackTemplate &operator =(const LogicalTrackTemplate&); //lint -esym(754, LogicalTrackTemplate::operator=)  // unreferenced // defensive hiding

private:
	
	int  m_resolverIndex;

};

// ======================================================================

class TrackAnimationController::TrackIndexResolver
{
public:

	TrackIndexResolver();

	void                    setTargetTrackId(const AnimationTrackId &targetTrackId);
	const AnimationTrackId &getTargetTrackId() const;

	int                     getPhysicalTrackIndex(const TrackIndexResolverVector &trackIndexResolvers) const;

private:

	AnimationTrackId  m_targetTrackId;

};

// ======================================================================
/**
 * Function object to convert skeletal animation message indices into
 * message names.
 */

class TrackAnimationController::MessageNameLookup
{
public:

	explicit MessageNameLookup(const SkeletalAnimation &skeletalAnimation);
	MessageNameLookup(const MessageNameLookup&);

	const CrcLowerString *operator()(int index) const;

private:

	// disabled
	MessageNameLookup();
	//lint -esym(754, MessageNameLookup::operator=)  // not referenced // hiding, cannot exist
	//lint -esym(1714, MessageNameLookup::operator=) // not referenced // hiding, cannot exist
	MessageNameLookup &operator =(const MessageNameLookup&);

private:

	const SkeletalAnimation &m_skeletalAnimation;

};

// ======================================================================
/**
 * A StateChangeAction that sets one of the controller's logical track
 * mappings.
 */

class TrackAnimationController::TrackMapperStateChangeAction: public TrackAnimationController::StateChangeAction
{
public:

	TrackMapperStateChangeAction(const AnimationTrackId &logicalTrackId, const AnimationTrackId &mapToTrackId);

	virtual void doAction(TrackAnimationController &controller, Track &track) const;

private:

	// disabled
	TrackMapperStateChangeAction();

	//lint -esym(754, TrackMapperStateChangeAction::TrackMapperStateChangeAction) // unreferenced // defensive hiding
	//lint -esym(1714, TrackMapperStateChangeAction::TrackMapperStateChangeAction) // unreferenced // defensive hiding
	TrackMapperStateChangeAction(const TrackMapperStateChangeAction&);             

	//lint -esym(754, TrackMapperStateChangeAction::operator=) // unreferenced // defensive hiding
	//lint -esym(1714, TrackMapperStateChangeAction::operator=) // unreferenced // defensive hiding
	TrackMapperStateChangeAction &operator =(const TrackMapperStateChangeAction&); 

private:

	const AnimationTrackId  m_logicalTrackId;
	const AnimationTrackId  m_mapToTrackId;

};

// ======================================================================

class TrackAnimationController::AddAnimationFactory: public TrackAnimationController::AnimationFactory
{
public:

	AddAnimationFactory(const AnimationTrackId &trackId1, const AnimationTrackId &trackId2);

	virtual SkeletalAnimation *fetchSkeletalAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const;

private:

	AddAnimationFactory();

private:

	AnimationTrackId  m_trackId1;
	AnimationTrackId  m_trackId2;

};

// ======================================================================

class TrackAnimationController::PriorityBlendAnimationFactory: public TrackAnimationController::AnimationFactory
{
public:

	PriorityBlendAnimationFactory(const AnimationTrackId &trackId1, const AnimationTrackId &trackId2);

	virtual SkeletalAnimation *fetchSkeletalAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const;

private:

	PriorityBlendAnimationFactory();

private:

	AnimationTrackId  m_trackId1;
	AnimationTrackId  m_trackId2;

};

// ======================================================================
// class TrackAnimationController::LogicalTrackTemplate
// ======================================================================

TrackAnimationController::LogicalTrackTemplate::LogicalTrackTemplate(const CrcLowerString &trackName, const AnimationTrackId &trackId, int resolverIndex)
:	TrackTemplate(trackName, trackId),
	m_resolverIndex(resolverIndex)
{
}

// ----------------------------------------------------------------------

bool TrackAnimationController::LogicalTrackTemplate::hasTrackIndexResolver() const
{
	// logical tracks require index resolvers
	return true;
}

// ----------------------------------------------------------------------

int TrackAnimationController::LogicalTrackTemplate::getTrackIndexResolverIndex() const
{
	return m_resolverIndex;
}

// ----------------------------------------------------------------------

int TrackAnimationController::LogicalTrackTemplate::getPhysicalTrackIndex() const
{
	// this is invalid to call when hasTrackIndexResolver() returns true.
	DEBUG_FATAL(true, ("caller called getPhysicalTrackIndex() when hasTrackIndexResolver() returned true"));
	return 0; //lint !e527 // unreachable // reachable in release
}

// ======================================================================
// class TrackAnimationController::TrackIndexResolver
// ======================================================================

TrackAnimationController::TrackIndexResolver::TrackIndexResolver()
:	m_targetTrackId(AnimationTrackId::cms_invalid)
{
}

// ----------------------------------------------------------------------

inline void TrackAnimationController::TrackIndexResolver::setTargetTrackId(const AnimationTrackId &targetTrackId)
{
	m_targetTrackId = targetTrackId;
}

// ----------------------------------------------------------------------

inline const AnimationTrackId &TrackAnimationController::TrackIndexResolver::getTargetTrackId() const
{
	return m_targetTrackId;
}

// ----------------------------------------------------------------------

int TrackAnimationController::TrackIndexResolver::getPhysicalTrackIndex(const TrackIndexResolverVector &trackIndexResolvers) const
{
	AnimationTrackId  lookupTrackId = m_targetTrackId;
	int      loopCount = 0;

	do 
	{
		const TrackTemplate &trackTemplate = getTrackTemplate(lookupTrackId.getTrackIdValue());

		if (!trackTemplate.hasTrackIndexResolver())
		{
			// the target references a physical track
			return trackTemplate.getPhysicalTrackIndex();
		}
		else
		{
			// the target references a logical track.
			// change the lookupTrackId to the target's target, then try again.
			const int resolverIndex = trackTemplate.getTrackIndexResolverIndex();

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, resolverIndex, static_cast<int>(trackIndexResolvers.size()));
			lookupTrackId = trackIndexResolvers[static_cast<size_t>(resolverIndex)].m_targetTrackId;
		}

		++loopCount;
	} while (loopCount < getLogicalTrackTemplateCount());

	DEBUG_FATAL(true, ("failed to resolve physical track index, looks like a loop in the logical track mapping"));
	return 0; //lint !e527 // unreachable // reachable in release
}

// ======================================================================
// class TrackAnimationController::MessageNameLookup
// ======================================================================

inline TrackAnimationController::MessageNameLookup::MessageNameLookup(const SkeletalAnimation &skeletalAnimation)
:	m_skeletalAnimation(skeletalAnimation)
{
}

// ----------------------------------------------------------------------

//lint -esym(1714, MessageNameLookup::MessageNameLookup) // not referenced // false, used by stl algorithm
inline TrackAnimationController::MessageNameLookup::MessageNameLookup(const MessageNameLookup &rhs)
:	m_skeletalAnimation(rhs.m_skeletalAnimation)
{
}

// ----------------------------------------------------------------------

//lint -esym(1714, MessageNameLookup::operator*) // not referenced // false, used by stl algorithm
inline const CrcLowerString *TrackAnimationController::MessageNameLookup::operator()(int index) const
{
	return &(m_skeletalAnimation.getMessageName(index));
}

// ======================================================================
// class TrackAnimationController::StateChangeAction
// ======================================================================

TrackAnimationController::StateChangeAction::StateChangeAction()
{
}

// ----------------------------------------------------------------------

TrackAnimationController::StateChangeAction::~StateChangeAction()
{
}

// ======================================================================
// class TrackAnimationController::TrackMapperStateChangeAction
// ======================================================================

TrackAnimationController::TrackMapperStateChangeAction::TrackMapperStateChangeAction(const AnimationTrackId &logicalTrackId, const AnimationTrackId &mapToTrackId)
:	StateChangeAction(),
	m_logicalTrackId(logicalTrackId),
	m_mapToTrackId(mapToTrackId)
{
}

// ----------------------------------------------------------------------

void TrackAnimationController::TrackMapperStateChangeAction::doAction(TrackAnimationController &controller, Track & /* track */) const
{
	controller.setLogicalTrackIdMapping(m_logicalTrackId, m_mapToTrackId);

#if 0
	//-- Make sure the new track's animation is ready to handle animations.  In particular, I hit a bug where
	//   it appeared the PriorityAnimation for the action track was switched "on" after the alter but before the
	//   evaluate, causing the evaluate to use old data that was never flushed because an alter did not come in.
	//   Here I'll apply a zero alter on the new track.
	Track &track                 = controller.getTrack(m_mapToTrackId);
	SkeletalAnimation *animation = track.getCurrentAnimation();
	if (animation)
	{
		float              deltaTimeRemaining;
		SkeletalAnimation *replacement;

		IGNORE_RETURN(animation->alterSingleCycle(0.0f, replacement, deltaTimeRemaining));
	}
#endif

	// clear out the translation and rotation on the track or it could be picked up and used erroneously
	Track &track                 = controller.getTrack(m_mapToTrackId);
	track.clearLocomotionTranslation();
	track.clearLocomotionRotation();
}

// ======================================================================
// class TrackAnimationController::AddAnimationFactory
// ======================================================================

TrackAnimationController::AddAnimationFactory::AddAnimationFactory(const AnimationTrackId &trackId1, const AnimationTrackId &trackId2) :
	m_trackId1(trackId1),
	m_trackId2(trackId2)
{
}

// ----------------------------------------------------------------------

SkeletalAnimation *TrackAnimationController::AddAnimationFactory::fetchSkeletalAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const
{
	SkeletalAnimation *const animation = new AddAnimation(transformNameMap, animationController, m_trackId1, m_trackId2);
	animation->fetch();

	return animation;
}

// ======================================================================
// class TrackAnimationController::PriorityBlendAnimationFactory
// ======================================================================

TrackAnimationController::PriorityBlendAnimationFactory::PriorityBlendAnimationFactory(const AnimationTrackId &trackId1, const AnimationTrackId &trackId2) :
	m_trackId1(trackId1),
	m_trackId2(trackId2)
{
}

// ----------------------------------------------------------------------

SkeletalAnimation *TrackAnimationController::PriorityBlendAnimationFactory::fetchSkeletalAnimation(const TransformNameMap &transformNameMap, TrackAnimationController &animationController) const
{
	SkeletalAnimation *const animation = new PriorityBlendAnimation(transformNameMap, animationController, m_trackId1, m_trackId2);
	animation->fetch();

	return animation;
}

// ======================================================================
// class TrackAnimationController: public static member functions
// ======================================================================

void TrackAnimationController::install()
{
	DEBUG_FATAL(ms_installed, ("TrackAnimationController already installed"));

	ms_physicalTrackCount = 0;
	ms_logicalTrackCount  = 0;

#ifdef _DEBUG
	ms_controllerCreationCount = 0;
#endif

	ms_trackTemplateMap = new TrackTemplateMap();

	// @todo: initialize via data file
	//-- Add track templates.
	ms_logicalRootTrackId    = registerLogicalTrack(cms_logicalRootTrackName);
	ms_logicalPrimaryTrackId = registerLogicalTrack(cms_logicalPrimaryTrackName);

	ms_loopTrackId           = registerPhysicalTrack(cms_loopTrackName, NULL, false);
	ms_addTrackId            = registerPhysicalTrack(cms_addTrackName, NULL, true);
	ms_actionTrackId         = registerPhysicalTrack(cms_actionTrackName, NULL, true);
	ms_actionCombinerTrackId = registerPhysicalTrack(cms_actionCombinerTrackName, new PriorityBlendAnimationFactory(ms_actionTrackId, ms_loopTrackId), false);
	ms_addCombinerTrackId    = registerPhysicalTrack(cms_addCombinerTrackName, new AddAnimationFactory(ms_logicalPrimaryTrackId, ms_addTrackId), false); // add track must be second --- AddAnimation expects first animation to be base, second to be add-in.

	//-- Add enter/exit idle actions.
	// Change logical_root logical track to map either to the addCombiner or the logicalPrimary track
	// depending on whether any add animations are playing.
	registerGlobalPreExitIdleAction(ms_addTrackId, new TrackMapperStateChangeAction(ms_logicalRootTrackId, ms_addCombinerTrackId));
	registerGlobalPostEnterIdleAction(ms_addTrackId, new TrackMapperStateChangeAction(ms_logicalRootTrackId, ms_logicalPrimaryTrackId));

	// Change logical_primary logical track to map either to the actionCombiner or the loop track
	// depending on whether any action animations are playing.
	registerGlobalPreExitIdleAction(ms_actionTrackId, new TrackMapperStateChangeAction(ms_logicalPrimaryTrackId, ms_actionCombinerTrackId));
#if 0
	registerGlobalPostEnterIdleAction(ms_actionTrackId, new TrackMapperStateChangeAction(ms_logicalPrimaryTrackId, ms_loopTrackId));
#endif

	DebugFlags::registerFlag(ms_logAnimationMessages,      "ClientSkeletalAnimation/Character", "logAnimationMessages");
	DebugFlags::registerFlag(ms_logUnappliedAnimationTime, "ClientSkeletalAnimation/Character", "logUnappliedAnimationTime");

	s_maxAnimationChannelCount = ConfigClientSkeletalAnimation::getControllerMaxChannelCount();

	ms_installed = true;
	ExitChain::add(remove, "TrackAnimationController");
}

// ----------------------------------------------------------------------

bool TrackAnimationController::getTrackIdByName(const CrcLowerString &trackName, AnimationTrackId &trackId)
{
	const TrackTemplateMap::iterator findIt = ms_trackTemplateMap->find(&trackName);
	if (findIt != ms_trackTemplateMap->end())
	{
		//-- track exists
		NOT_NULL(findIt->second);
		trackId = findIt->second->getTrackId();

		return true;
	}
	else
	{
		//-- track doesn't exist
		trackId = AnimationTrackId::cms_invalid;
		return false;
	}
}

// ----------------------------------------------------------------------

int TrackAnimationController::getTrackTemplateCount()
{
	return static_cast<int>(ms_trackTemplates.size());
}

// ----------------------------------------------------------------------

int TrackAnimationController::getLogicalTrackTemplateCount()
{
	return ms_logicalTrackCount;
}

// ----------------------------------------------------------------------

int TrackAnimationController::getPhysicalTrackTemplateCount()
{
	return ms_physicalTrackCount;
}

// ----------------------------------------------------------------------

const TrackAnimationController::TrackTemplate &TrackAnimationController::getTrackTemplate(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTrackTemplateCount());
	return *NON_NULL(ms_trackTemplates[static_cast<size_t>(index)]);
}

// ----------------------------------------------------------------------

void TrackAnimationController::registerGlobalPostEnterIdleAction(const AnimationTrackId &physicalTrackId, StateChangeAction *stateChangeAction)
{
	NOT_NULL(stateChangeAction);

	//-- get the track template for the given track id
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, physicalTrackId.getTrackIdValue(), getTrackTemplateCount());
	PhysicalTrackTemplate *const trackTemplate = safe_cast<PhysicalTrackTemplate*>(ms_trackTemplates[static_cast<size_t>(physicalTrackId.getTrackIdValue())]);
	NOT_NULL(trackTemplate);

	//-- add the state change action
	trackTemplate->addPostEnterIdleAction(stateChangeAction);
}

// ----------------------------------------------------------------------

void TrackAnimationController::registerGlobalPreExitIdleAction(const AnimationTrackId &physicalTrackId, StateChangeAction *stateChangeAction)
{
	NOT_NULL(stateChangeAction);

	//-- get the track template for the given track id
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, physicalTrackId.getTrackIdValue(), getTrackTemplateCount());
	PhysicalTrackTemplate *const trackTemplate = safe_cast<PhysicalTrackTemplate*>(ms_trackTemplates[static_cast<size_t>(physicalTrackId.getTrackIdValue())]);
	NOT_NULL(trackTemplate);

	//-- add the state change action
	trackTemplate->addPreExitIdleAction(stateChangeAction);
}

// ======================================================================
// class TrackAnimationController: public member functions
// ======================================================================

TrackAnimationController::TrackAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap *transformNameMap, int channel) :
	TransformAnimationController(animationEnvironment, transformNameMap),
	m_tracks(new TrackVector(static_cast<size_t>(ms_physicalTrackCount), static_cast<Track*>(0))),
	m_trackIndexResolvers(new TrackIndexResolverVector(static_cast<size_t>(ms_logicalTrackCount))),
	m_physicalTrackMap(new IntVector(ms_trackTemplates.size())),
	m_channel(channel)
{
	DEBUG_FATAL(!ms_installed, ("TrackAnimationController not installed"));
	NOT_NULL(transformNameMap);

	//-- keep track of whether any controllers have been created
#ifdef _DEBUG
	++ms_controllerCreationCount;
#endif

	//-- create tracks
	for (int i = 0; i < ms_physicalTrackCount; ++i)
	{
		//-- get the physical track template for this track
		const size_t trackTemplateIndex = static_cast<size_t>(ms_physicalTrackTemplateByTrackIndex[static_cast<size_t>(i)]);
		const PhysicalTrackTemplate *const trackTemplate = dynamic_cast<PhysicalTrackTemplate*>(ms_trackTemplates[trackTemplateIndex]);
		NOT_NULL(trackTemplate);

		//-- create the track
		(*m_tracks)[static_cast<TrackVector::size_type>(i)] = new Track(*this, *trackTemplate);
	}

	//-- initialize the physical track mappings
	initializePhysicalTrackMap();

	//-- initialize the logical track mappings
	// -TRF- this should be data-file driven
	setLogicalTrackIdMapping(ms_logicalPrimaryTrackId, ms_loopTrackId);
	setLogicalTrackIdMapping(ms_logicalRootTrackId, ms_logicalPrimaryTrackId);
}

// ----------------------------------------------------------------------

TrackAnimationController::~TrackAnimationController()
{
	//-- delete tracks first.  Track references the controller and
	//   may need some controller state during cleanup.
	std::for_each(m_tracks->begin(), m_tracks->end(), PointerDeleter());
	delete m_tracks;

	delete m_physicalTrackMap;
	delete m_trackIndexResolvers;
}

// ----------------------------------------------------------------------

void TrackAnimationController::setLogicalTrackIdMapping(const AnimationTrackId &logicalTrackId, const AnimationTrackId &mapsToTrackId)
{
	//-- ensure specified logical track is, in fact, logical
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, logicalTrackId.getTrackIdValue(), static_cast<int>(ms_trackTemplates.size()));
	const TrackTemplate &trackTemplate = *NON_NULL(ms_trackTemplates[static_cast<size_t>(logicalTrackId.getTrackIdValue())]);

	DEBUG_FATAL(!trackTemplate.hasTrackIndexResolver(), ("specified AnimationTrackId is not a logical track"));

	//-- set the track index resolver's target
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, trackTemplate.getTrackIndexResolverIndex(), static_cast<int>(m_trackIndexResolvers->size()));
	TrackIndexResolver &trackIndexResolver = (*m_trackIndexResolvers)[static_cast<size_t>(trackTemplate.getTrackIndexResolverIndex())];

	trackIndexResolver.setTargetTrackId(mapsToTrackId);

	//-- update our physical track map
	updatePhysicalTrackMap();
}

// ----------------------------------------------------------------------

int TrackAnimationController::playAnimation(const AnimationTrackId &trackId, SkeletalAnimation *skeletalAnimation, PlayMode playMode, bool loop, BlendMode transitionBlendMode, float blendInTime, AnimationNotification *notification)
{
	//-- get the track
	Track &track = getTrack(trackId);

	//-- play the animation on the track
	int const returnCode = track.playAnimation(skeletalAnimation, playMode, loop, transitionBlendMode, blendInTime, notification);

	//-- Setup controller information for notification.
	if (notification)
		notification->setControllerInfo(m_channel, this, trackId);

	return returnCode;
}

// ----------------------------------------------------------------------

void TrackAnimationController::stopAnimation(const AnimationTrackId &trackId)
{
	//-- get the track
	Track &track = getTrack(trackId);

	//-- play the animation on the track
	track.stopAnimation();
}

// ----------------------------------------------------------------------

int TrackAnimationController::getMostRecentlyCompletedAnimationId(const AnimationTrackId &trackId) const
{
	//-- Get the track.
	const Track &track = getTrack(trackId);

	//-- Get the animation id.
	return track.getMostRecentlyCompletedAnimationId();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the id of the currently playing animation on the specified track.
 *
 * @param trackId      the track on which to query.
 * @param animationId  if an animation is currently playing (see return value),
 *                     this value returns the id of the currently playing animation.
 *
 * @return  true if an animation is currently playing; false otherwise.
 */

bool TrackAnimationController::getCurrentlyPlayingAnimationId(const AnimationTrackId &trackId, int &animationId) const
{
	//-- Get the track.
	const Track &track = getTrack(trackId);

	//-- Get the animation id.
	return track.getCurrentlyPlayingAnimationId(animationId);
}

// ----------------------------------------------------------------------

bool TrackAnimationController::isTrackIdle(const AnimationTrackId &trackId)
{
	//-- get the track
	Track &track = getTrack(trackId);

	//-- query if idle
	return track.isIdle();	
}

// ----------------------------------------------------------------------

bool TrackAnimationController::doesTrackHaveLocomotionPriority(const AnimationTrackId &trackId) const
{
	//-- Find the track that has locomotion priority.
	const Track &logicalRootTrack        = getTrack(ms_logicalRootTrackId);
	const Track &locomotionPriorityTrack = logicalRootTrack.getLocomotionPriorityTrack();

	//-- Test locomotion priority track against specified test track.
	const Track &targetTrack             = getTrack(trackId);
	const bool hasLocomotionPriority     = (&targetTrack == &locomotionPriorityTrack);

	//-- Return result.
	return hasLocomotionPriority;
}

// ----------------------------------------------------------------------

int TrackAnimationController::getTrackLocomotionPriority(const AnimationTrackId &trackId) const
{
	Track const &track = getTrack(trackId);
	
	SkeletalAnimation const *const animation = track.fetchCurrentAnimation();
	if (animation)
	{
		int const priority = animation->getLocomotionPriority();
		animation->release();
		return priority;
	}
	else
		return std::numeric_limits<int>::min();
}

// ----------------------------------------------------------------------

void TrackAnimationController::disableTrack(const AnimationTrackId &trackId)
{
	//-- For now, only handle disabling the action track.
	// @todo generalize this.
	if (trackId == ms_actionTrackId)
	{
		// When the action track requests to be disabled, point the logical primary track id to the loop track.
		setLogicalTrackIdMapping(ms_logicalPrimaryTrackId, ms_loopTrackId);
	}
}

// ----------------------------------------------------------------------

int TrackAnimationController::getChannel() const
{
	VALIDATE_CHANNEL_INDEX();
	return m_channel;
}

// ----------------------------------------------------------------------

void TrackAnimationController::alter(float deltaTime)
{
	//-- Alter the logical root track.  This will cause all other tracks to
	//   get evaluated but will not evaluate any non-playing tracks.
	Track &track = getTrack(ms_logicalRootTrackId);
	track.alter(deltaTime);
}

// ----------------------------------------------------------------------

void TrackAnimationController::evaluateTransformComponents(int localTransformIndex, Quaternion &rotation, Vector &translation)
{
	PROFILER_AUTO_BLOCK_DEFINE("Animation::evaluateComponents");

	//-- get the root track
	Track &track = getTrack(ms_logicalRootTrackId);

	//-- return the operations
	track.evaluateTransformComponents(localTransformIndex, rotation, translation);
}

// ----------------------------------------------------------------------

void TrackAnimationController::getObjectLocomotion(Quaternion &rotation, Vector &translation) const
{
	//-- get the root track
	const Track &track = getTrack(ms_logicalRootTrackId);

	//-- return the operations
	track.getObjectLocomotion(rotation, translation);
}

// ----------------------------------------------------------------------

TrackAnimationController *TrackAnimationController::asTrackAnimationController()
{
	return this;
}

// ----------------------------------------------------------------------
/**
 * Save the state of this animation controller instance for later
 * recall.
 *
 * The initial implementation of this function saves the
 * transform data of the logical root track.  It will apply that
 * data to the primary track during applyBookmark().
 * 
 * @return  a Bookmark-derived instance containing the state of the
 *          controller.
 *
 * @see TransformAnimationController::createBookmark()
 */

#if 0

TransformAnimationController::Bookmark *TrackAnimationController::createBookmark() const
{
	//-- get the root track
	const Track &track = getTrack(ms_logicalRootTrackId);

	//-- return the Track's bookmark
	return track.createBookmark();	
}

#endif

// ----------------------------------------------------------------------
/**
 * Recall the state of this animation controller instance previously saved 
 * in the bookmark.
 *
 * The initial implementation of this function applies the saved transform
 * state to the primary track.
 * 
 * @return  a Bookmark-derived instance containing the state of the
 *          controller.
 *
 * @see TransformAnimationController::createBookmark()
 */

#if 0

void TrackAnimationController::applyBookmark(const Bookmark *bookmark)
{
	//-- get the primary track
	Track &track = getTrack(ms_loopTrackId);

	//-- apply the bookmark to it
	track.applyBookmark(bookmark);
}

#endif

// ======================================================================
// class TransformAnimationController: private static member functions
// ======================================================================

void TrackAnimationController::remove()
{
	DEBUG_FATAL(!ms_installed, ("TrackAnimationController not installed"));

	ms_trackTemplateMap->clear();
	delete ms_trackTemplateMap;
	ms_trackTemplateMap = 0;
	
	std::for_each(ms_trackTemplates.begin(), ms_trackTemplates.end(), PointerDeleter());
	TrackTemplateVector().swap(ms_trackTemplates);

	TrackIdVector().swap(ms_logicalTrackIdByResolverIndex);
	ConstCrcLowerStringVector().swap(ms_emittedMessageNames);
	IntVector().swap(ms_emittedMessageIndices);

	IntVector().swap(ms_physicalTrackTemplateByTrackIndex);

	ms_installed = false;
}

// ----------------------------------------------------------------------

const AnimationTrackId &TrackAnimationController::registerPhysicalTrack(const CrcLowerString &trackName, AnimationFactory *animationFactory, bool allowSameTrackTrumping)
{
	DEBUG_FATAL(ms_controllerCreationCount != 0, ("invalid: trying to register track after controllers have been created"));

	//-- Handle if track name has been registered already.
	TrackTemplateMap::iterator lowerBoundResult = ms_trackTemplateMap->lower_bound(&trackName);
	if ((lowerBoundResult != ms_trackTemplateMap->end()) && !ms_trackTemplateMap->key_comp()(&trackName, lowerBoundResult->first))
	{
		// Track name already registered.

		//-- ensure previously-registered track is a physical track
		DEBUG_FATAL(!dynamic_cast<PhysicalTrackTemplate*>(lowerBoundResult->second), ("tried to re-register physical track [%s] but already-registered track is not physical", trackName.getString()));

		//-- Return existing trackId.
		delete animationFactory;
		return lowerBoundResult->second->getTrackId();
	}

	//-- Handle new track.

	// Create the track template.
	const int newTrackIdValue = static_cast<int>(ms_trackTemplates.size());
	TrackTemplate *const trackTemplate = new PhysicalTrackTemplate(trackName, AnimationTrackId(newTrackIdValue), ms_physicalTrackCount, animationFactory, allowSameTrackTrumping);

	// Bump up physical track count.
	++ms_physicalTrackCount;

	// Add it to the collections.
	ms_trackTemplates.push_back(trackTemplate);
	IGNORE_RETURN(ms_trackTemplateMap->insert(lowerBoundResult, TrackTemplateMap::value_type(&trackTemplate->getTrackName(), trackTemplate)));

	// Keep track of mapping from physical track index to all track index.
	// allows us to go from track index (only physical tracks) to the track template (all tracks combined).
	ms_physicalTrackTemplateByTrackIndex.push_back(newTrackIdValue);

	return trackTemplate->getTrackId();
}

// ----------------------------------------------------------------------

const AnimationTrackId &TrackAnimationController::registerLogicalTrack(const CrcLowerString &trackName)
{
	DEBUG_FATAL(ms_controllerCreationCount != 0, ("invalid: trying to register track after controllers have been created"));

	//-- handle if track name has been registered already
	TrackTemplateMap::iterator lowerBoundResult = ms_trackTemplateMap->lower_bound(&trackName);
	if ((lowerBoundResult != ms_trackTemplateMap->end()) && !ms_trackTemplateMap->key_comp()(&trackName, lowerBoundResult->first))
	{
		// Track name already registered.

		//-- ensure previously-registered track is a physical track
		DEBUG_FATAL(!dynamic_cast<LogicalTrackTemplate*>(lowerBoundResult->second), ("tried to re-register logical track [%s] but already-registered track is not logical", trackName.getString()));

		//-- return it
		return lowerBoundResult->second->getTrackId();
	}

	//-- handle new track

	// create the track template
	const int newTrackIdValue = static_cast<int>(ms_trackTemplates.size());
	TrackTemplate *const trackTemplate = new LogicalTrackTemplate(trackName, AnimationTrackId(newTrackIdValue), ms_logicalTrackCount);

	// bump up logical track count.
	++ms_logicalTrackCount;

	// add it to the collections
	ms_trackTemplates.push_back(trackTemplate);
	IGNORE_RETURN(ms_trackTemplateMap->insert(lowerBoundResult, TrackTemplateMap::value_type(&trackTemplate->getTrackName(), trackTemplate)));

	// setup mapping of a controller's TrackIndexResolver position to AnimationTrackId (static data), without requiring each
	// controller to store this data directly in the TrackIndexResolver.
	ms_logicalTrackIdByResolverIndex.push_back(trackTemplate->getTrackId());
	DEBUG_FATAL(static_cast<int>(ms_logicalTrackIdByResolverIndex.size()) != ms_logicalTrackCount, ("ms_logicalTrackIdByResolverIndex out of sync with ms_logicalTrackCount"));

	return trackTemplate->getTrackId();
}

// ======================================================================
// class TransformAnimationController: private member functions
// ======================================================================
/**
 * Initialize the quick-lookup map that maps a AnimationTrackId value to a physical
 * track.
 *
 * This function should be called by the constructor.  It only needs to be
 * called once per controller instance.  It sets up the quick-lookup
 * AnimationTrackId => physical track map for the static physical TrackIds.  These
 * mappings never change since physical TrackIds will always map to the
 * same physical track index.
 */

void TrackAnimationController::initializePhysicalTrackMap()
{
	const TrackTemplateVector::const_iterator endIt = ms_trackTemplates.end();
	for (TrackTemplateVector::const_iterator it = ms_trackTemplates.begin(); it != endIt; ++it)
	{
		const TrackTemplate &trackTemplate = *NON_NULL(*it);

		if (!trackTemplate.hasTrackIndexResolver())
		{
			//-- setup the map for this physical AnimationTrackId to the physical track index
			const size_t trackIdValue       = static_cast<size_t>(trackTemplate.getTrackId().getTrackIdValue());
			const int    physicalTrackIndex = trackTemplate.getPhysicalTrackIndex();

			(*m_physicalTrackMap)[trackIdValue] = physicalTrackIndex;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Update the logical AnimationTrackId mappings in the quick-lookup map that 
 * maps a AnimationTrackId value to a physical track.
 *
 * This function should be called whenever a logical track mapping
 * changes.  It will resolve logical TrackIds to a physical track id.
 */

void TrackAnimationController::updatePhysicalTrackMap()
{
	int    physicalTrackIndex;
	int    index = 0;

	const TrackIndexResolverVector::iterator endIt = m_trackIndexResolvers->end();
	for (TrackIndexResolverVector::iterator it = m_trackIndexResolvers->begin(); it != endIt; ++it, ++index)
	{
		//-- get the AnimationTrackId value for this logical entry
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(ms_logicalTrackIdByResolverIndex.size()));
		const size_t trackIdValue = static_cast<size_t>(ms_logicalTrackIdByResolverIndex[static_cast<size_t>(index)].getTrackIdValue());

		//-- resolve the logical AnimationTrackId to a physical track id
		if (it->getTargetTrackId() != AnimationTrackId::cms_invalid)
		{
			// track resolver's target has been set, resolve it
			physicalTrackIndex = it->getPhysicalTrackIndex(*m_trackIndexResolvers);
		}
		else
		{
			// track resolver's target has not yet been set.  This occurs on creation of the track.
			// if this logical track is referenced, it will not map to a valid track at this point.
			physicalTrackIndex = -1;
		}

		//-- set the mapping
		(*m_physicalTrackMap)[static_cast<size_t>(trackIdValue)] = physicalTrackIndex;
	}
}

// ----------------------------------------------------------------------

const TrackAnimationController::Track &TrackAnimationController::getTrack(const AnimationTrackId &trackId) const
{
	//-- lookup physical track index from trackId
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, trackId.getTrackIdValue(), static_cast<int>(m_physicalTrackMap->size()));
	const int physicalTrackIndex = (*m_physicalTrackMap)[static_cast<size_t>(trackId.getTrackIdValue())];

	//-- return the track
	// if this fires, the specified track probably is a logical track that is not currently mapped
	// to something that resolves to a physical track.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, physicalTrackIndex, static_cast<int>(m_tracks->size()));
	NOT_NULL((*m_tracks)[static_cast<size_t>(physicalTrackIndex)]);

	return *((*m_tracks)[static_cast<size_t>(physicalTrackIndex)]);
}

// ----------------------------------------------------------------------

const TrackAnimationController::Track *TrackAnimationController::getTrackConstPtr(const AnimationTrackId &trackId) const
{
	return &(getTrack(trackId));
}

// ----------------------------------------------------------------------

TrackAnimationController::Track &TrackAnimationController::getTrack(const AnimationTrackId &trackId)
{
	//-- lookup physical track index from trackId
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, trackId.getTrackIdValue(), static_cast<int>(m_physicalTrackMap->size()));
	const int physicalTrackIndex = (*m_physicalTrackMap)[static_cast<size_t>(trackId.getTrackIdValue())];

	//-- return the track
	// if this fires, the specified track probably is a logical track that is not currently mapped
	// to something that resolves to a physical track.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, physicalTrackIndex, static_cast<int>(m_tracks->size()));
	NOT_NULL((*m_tracks)[static_cast<size_t>(physicalTrackIndex)]);

	return *((*m_tracks)[static_cast<size_t>(physicalTrackIndex)]);
}

// ----------------------------------------------------------------------
/**
 * Emit animation message callbacks for all messages that have occurred
 * during the specified frame number range.
 *
 * @param  beginFrameNumber  specifies the 0-based frame number on which
 *                           to start scanning for messages to emit.
 *                           Messages start scanning on the first integral
 *                           frame number >= this value (i.e. inclusive).
 * @param  endFrameNumber    specifies the 0-based frame number on which
 *                           to end scanning for messages to emit.
 *                           Messages finish scanning on the first integral
 *                           frame number less than this value (i.e. exclusive).
 */

void TrackAnimationController::emitAnimationMessages(SkeletalAnimation &skeletalAnimation) const
{
	//-- reject if no animation messages on current animation
	if (skeletalAnimation.getMessageCount() < 1)
		return;

	//-- retrieve message indices emitted during this interval
	ms_emittedMessageIndices.clear();
	s_elapsedTimeSinceMessageTriggers.clear();

	skeletalAnimation.getSignaledMessages(ms_emittedMessageIndices, s_elapsedTimeSinceMessageTriggers);
	DEBUG_FATAL(ms_emittedMessageIndices.size() != s_elapsedTimeSinceMessageTriggers.size(), ("getSignaledMessages(): Unbalanced data returned, programmer error [%d]/[%d].", static_cast<int>(ms_emittedMessageIndices.size()), static_cast<int>(s_elapsedTimeSinceMessageTriggers.size())));

	// reject here if possible
	if (ms_emittedMessageIndices.empty())
		return;

	//-- convert message indices into message names
	ms_emittedMessageNames.resize(ms_emittedMessageIndices.size());
	std::transform(ms_emittedMessageIndices.begin(), ms_emittedMessageIndices.end(), ms_emittedMessageNames.begin(), MessageNameLookup(skeletalAnimation));


	//-- handle notifications and callbacks, filtering out duplicate messages.
	size_t const messageCount = ms_emittedMessageNames.size();
	const int callbackCount = getAnimationMessageCallbackCount();

	if (messageCount > 0 && (skeletalAnimation.hasNotification() || callbackCount > 0))
	{
		ConstCrcLowerStringVector processedMessageList;
		ConstCrcLowerStringVector::iterator processedMessageListItr;


		for (size_t messageIndex = 0; messageIndex < messageCount; ++messageIndex)
		{
			bool messageAlreadyProcessed = false;
			processedMessageListItr = processedMessageList.begin();
			while (processedMessageListItr != processedMessageList.end())
			{
				if (*(*processedMessageListItr) == *ms_emittedMessageNames[messageIndex])
				{
					// this message has already been sent for this update so ignore this one
					DEBUG_REPORT_LOG(ms_logAnimationMessages, ("ignoring duplicate message [%s].\n", ms_emittedMessageNames[messageIndex]->getString()));
					messageAlreadyProcessed = true;
					break;
				}
				++processedMessageListItr;
			}

			if (!messageAlreadyProcessed)
			{
				//-- Send messages to notification.
				skeletalAnimation.handleAnimationMessage(m_channel, *ms_emittedMessageNames[messageIndex], s_elapsedTimeSinceMessageTriggers[messageIndex]);

				//-- loop through all callbacks and tell them about it
				AnimationMessageCallback  callback = 0;
				void                     *context  = 0;


				for (int i = 0; i < callbackCount; ++i)
				{
					//-- get the callback
					getAnimationMessageCallbackInfo(i, callback, context);
					NOT_NULL(callback);

					(*callback)(context, *ms_emittedMessageNames[messageIndex], this);
					DEBUG_REPORT_LOG(ms_logAnimationMessages, ("callback [0x%08x]: signaled message [%s].\n", callback, ms_emittedMessageNames[messageIndex]->getString()));
				}

				processedMessageList.push_back(ms_emittedMessageNames[messageIndex]);
			}
		}


	}
}

// ----------------------------------------------------------------------

void TrackAnimationController::validateChannelIndex() const
{
	if ((m_channel < 0) || (m_channel >= s_maxAnimationChannelCount))
	{
		Object     const *const ownerObject     = getAnimationEnvironment().getOwnerObject();
		Appearance const *const ownerAppearance = (ownerObject ? ownerObject->getAppearance() : NULL);

		FATAL(true,
			("TrackAnimationController at [%p] for object id [%s], appearance template [%s] has invalid animation channel index [%d], valid range is [0..%d].",
			this,
			ownerObject ? ownerObject->getNetworkId().getValueString().c_str() : "<null owner>",
			ownerAppearance ? ownerAppearance->getAppearanceTemplateName() : "<null name>",
			m_channel,
			s_maxAnimationChannelCount - 1
			));
	}
}

// ======================================================================
