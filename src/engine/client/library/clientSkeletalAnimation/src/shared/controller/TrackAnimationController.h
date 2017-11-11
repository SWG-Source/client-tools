// ======================================================================
//
// TrackAnimationController.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TrackAnimationController_H
#define INCLUDED_TrackAnimationController_H

// ======================================================================

#include "clientSkeletalAnimation/TransformAnimationController.h"

class AnimationEnvironment;
class AnimationNotification;
class AnimationTrackId;
class ConstCharCrcLowerString;
class CrcLowerString;
class LessPointerComparator;
class SkeletalAnimation;
class SkeletalAnimationTemplate;
class TransformNameMap;

// ======================================================================
/**
 * An animation controller based on a set of tracks that can be combined
 * to create composite effects.
 */

class TrackAnimationController: public TransformAnimationController
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Track;
	friend class Track;

	// classes I wish were private.
	class TrackIndexResolver;
	class TrackTemplate;

	class PriorityBlendAnimation;
	friend class PriorityBlendAnimation;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Abstract base class for an action that gets performed when a track
	 * changes from one state to another.
	 */

	class StateChangeAction
	{
	public:

		virtual ~StateChangeAction();
		virtual void doAction(TrackAnimationController &controller, Track &track) const = 0;

	protected:

		StateChangeAction();

	private:

		// disabled
		StateChangeAction(const StateChangeAction&);
		StateChangeAction &operator =(const StateChangeAction&);
		
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum PlayMode
	{
		/**
		 * Add to end of track's queue. Any currently playing 
		 * animation will stop at the end of its current loop
		 * and then start the next animation in the queue.
		 */
		PM_queue,

		/**
		 * Delete any entries in the queue.  Stop any currently
		 * playing animation and transition to the new animation
		 * immediately.
		 */
		PM_immediate
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum BlendMode
	{
		BM_noBlend,
		BM_linearBlend
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void                 install();

	static bool                 getTrackIdByName(const CrcLowerString &trackName, AnimationTrackId &trackId);
	static CrcLowerString      &getTrackNameById(const AnimationTrackId &trackId);

	static int                  getTrackTemplateCount();
	static int                  getLogicalTrackTemplateCount();
	static int                  getPhysicalTrackTemplateCount();

	static const TrackTemplate &getTrackTemplate(int index);

	static void                 registerGlobalPostEnterIdleAction(const AnimationTrackId &physicalTrackId, StateChangeAction *stateChangeAction);
	static void                 registerGlobalPreExitIdleAction(const AnimationTrackId &physicalTrackId, StateChangeAction *stateChangeAction);

public:

	explicit TrackAnimationController(AnimationEnvironment &animationEnvironment, const TransformNameMap *transformNameMap, int channel);
	virtual ~TrackAnimationController();

	void                                   setLogicalTrackIdMapping(const AnimationTrackId &logicalTrackId, const AnimationTrackId &mapsToTrackId);

	int                                    playAnimation(const AnimationTrackId &trackId, SkeletalAnimation *skeletalAnimation, PlayMode playMode, bool loop, BlendMode transitionBlendMode, float blendInTime, AnimationNotification *notification);
	void                                   stopAnimation(const AnimationTrackId &trackId);

	int                                    getMostRecentlyCompletedAnimationId(const AnimationTrackId &trackId) const;
	bool                                   getCurrentlyPlayingAnimationId(const AnimationTrackId &trackId, int &animationId) const;

	bool                                   isTrackIdle(const AnimationTrackId &trackId);
	bool                                   doesTrackHaveLocomotionPriority(const AnimationTrackId &trackId) const;
	int                                    getTrackLocomotionPriority(const AnimationTrackId &trackId) const;

	void                                   disableTrack(const AnimationTrackId &trackId);

	int                                    getChannel() const;

	virtual void                           alter(float time);

	virtual void                           evaluateTransformComponents(int localTransformIndex, Quaternion &rotation, Vector &translation);
	virtual void                           getObjectLocomotion(Quaternion &rotation, Vector &translation) const;

	virtual TrackAnimationController      *asTrackAnimationController();

	// tool use
	const Track *getTrackConstPtr(const AnimationTrackId &trackId) const;

private:

	class PhysicalTrackTemplate;
	class LogicalTrackTemplate;

	class MessageNameLookup;
	class TrackMapperStateChangeAction;
	friend class TrackMapperStateChangeAction;

	class AnimationFactory;
	class AddAnimationFactory;
	class PriorityBlendAnimationFactory;

	class AddAnimation;
	friend class AddAnimation;

	typedef stdmap<const CrcLowerString*, TrackTemplate*, LessPointerComparator>::fwd  TrackTemplateMap;

	typedef stdvector<const CrcLowerString*>::fwd                                      ConstCrcLowerStringVector;
	typedef stdvector<int>::fwd                                                        IntVector;
	typedef stdvector<AnimationTrackId>::fwd                                           TrackIdVector;
	typedef stdvector<TrackIndexResolver>::fwd                                         TrackIndexResolverVector;
	typedef stdvector<TrackTemplate*>::fwd                                             TrackTemplateVector;
	typedef stdvector<Track*>::fwd                                                     TrackVector;

private:

	static void                    remove();

	static const AnimationTrackId &registerPhysicalTrack(const CrcLowerString &trackName, AnimationFactory *animationFactory, bool allowSameTrackTrumping);
	static const AnimationTrackId &registerLogicalTrack(const CrcLowerString &trackName);

private:

	void         initializePhysicalTrackMap();
	void         updatePhysicalTrackMap();

	const Track &getTrack(const AnimationTrackId &trackId) const;
	Track       &getTrack(const AnimationTrackId &trackId);

	void         emitAnimationMessages(SkeletalAnimation &skeletalAnimation) const;

	void         validateChannelIndex() const;

	// disabled
	TrackAnimationController();
	TrackAnimationController(const TrackAnimationController&);
	TrackAnimationController &operator =(const TrackAnimationController&);

private:

	static const ConstCharCrcLowerString       cms_loopTrackName;
	static const ConstCharCrcLowerString       cms_addTrackName;
	static const ConstCharCrcLowerString       cms_actionTrackName;
	static const ConstCharCrcLowerString       cms_addCombinerTrackName;
	static const ConstCharCrcLowerString       cms_actionCombinerTrackName;
	static const ConstCharCrcLowerString       cms_logicalRootTrackName;
	static const ConstCharCrcLowerString       cms_logicalPrimaryTrackName;

	static bool                       ms_installed;
	static TrackTemplateVector        ms_trackTemplates;
	static TrackTemplateMap          *ms_trackTemplateMap; // NOTE: making this a non-pointer requires calling classes to need to know about LessPointerComparator!

	static AnimationTrackId           ms_loopTrackId;
	static AnimationTrackId           ms_addTrackId;
	static AnimationTrackId           ms_actionTrackId;
	static AnimationTrackId           ms_addCombinerTrackId;
	static AnimationTrackId           ms_actionCombinerTrackId;
	static AnimationTrackId           ms_logicalRootTrackId;
	static AnimationTrackId           ms_logicalPrimaryTrackId;

	static int                        ms_physicalTrackCount;
	static int                        ms_logicalTrackCount;

	static ConstCrcLowerStringVector  ms_emittedMessageNames;
	static IntVector                  ms_emittedMessageIndices;

	/**
	 * There is one AnimationTrackId that corresponds one-to-one with each
	 * TrackIndexResolver index.
	 */
	static TrackIdVector              ms_logicalTrackIdByResolverIndex;

	/**
	 * Map each track index to the ms_TrackTemplates index for the
	 * corresponding physical track template.
	 */
	static IntVector                  ms_physicalTrackTemplateByTrackIndex;

	static bool                       ms_logAnimationMessages;
	static bool                       ms_logUnappliedAnimationTime;

#ifdef _DEBUG
	static int                        ms_controllerCreationCount;
#endif

private:

	/**
	 * There is one Track instance for each physical track.
	 */
	TrackVector              *m_tracks;

	/**
	 * There is one TrackResolver instance for every logical track.
	 */
	TrackIndexResolverVector *m_trackIndexResolvers;

	/**
	 * There is one entry for every (physical + logical) track.
	 *
	 * This maps each TrackIdValue into a physical track index.
	 */
	IntVector                *m_physicalTrackMap;

	/**
	 * The channel index for the animation controller, assigned by the TransformAnimationResolver.
	 *
	 * This channel is used to tell an AnimationNotification which animation controller is sending
	 * animation-related information such as animation messages & animation notification state changes.
	 */
	int                       m_channel;
};

// ======================================================================

#endif
