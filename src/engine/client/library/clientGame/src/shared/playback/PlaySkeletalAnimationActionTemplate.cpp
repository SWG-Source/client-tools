// ======================================================================
//
// PlaySkeletalAnimationActionTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlaySkeletalAnimationActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/PlaySkeletalAnimationAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================
// local constants
// ======================================================================

const Tag TAG_AANM = TAG(A,A,N,M);
const Tag TAG_FTRK = TAG(F,T,R,K);
const Tag TAG_NONE = TAG(N,O,N,E);
const Tag TAG_PSKA = TAG(P,S,K,A);
const Tag TAG_PSTR = TAG(P,S,T,R);

// ======================================================================
// static member variable definitions
// ======================================================================

bool PlaySkeletalAnimationActionTemplate::ms_installed;

// ======================================================================
// public static member functions
// ======================================================================

void PlaySkeletalAnimationActionTemplate::install()
{
	InstallTimer const installTimer("PlaySkeletalAnimationActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("PlaySkeletalAnimationActionTemplate already installed"));

	// register action template
	PlaybackScriptTemplate::registerActionTemplate(TAG_PSKA, create);

	ms_installed = true;
	ExitChain::add(remove, "PlaySkeletalAnimationActionTemplate");
}

// ======================================================================
// public member functions
// ======================================================================

PlaySkeletalAnimationActionTemplate::PlaySkeletalAnimationActionTemplate(int actorIndex, Tag animationNameVariable, bool enablePostureChange, Tag newServerPostureVariable, Tag specialAttackEffectVariable) :
	PlaybackActionTemplate(),
	m_actorIndex(actorIndex),
	m_animationNameVariable(animationNameVariable),
	m_postureChangeEnabled(enablePostureChange),
	m_newServerPostureVariable(newServerPostureVariable),
	m_specialAttackEffectVariable(specialAttackEffectVariable),
	m_faceTrackingEnabled(false),
	m_faceTrackTargetActorIndex(-1),
	m_stopAnimationOnAbort(false)
{
	DEBUG_FATAL(!ms_installed, ("not installed."));
}

// ----------------------------------------------------------------------

PlaySkeletalAnimationActionTemplate::~PlaySkeletalAnimationActionTemplate()
{
}

// ----------------------------------------------------------------------

PlaybackAction *PlaySkeletalAnimationActionTemplate::createPlaybackAction() const
{
	// use a simple Passthrough playback action for the non-static action
	return new PlaySkeletalAnimationAction(*this);
}

// ======================================================================
// private static member functions
// ======================================================================

void PlaySkeletalAnimationActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("PlaySkeletalAnimationActionTemplate not installed"));

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_PSKA);
	
	ms_installed = false;
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *PlaySkeletalAnimationActionTemplate::create(Iff &iff)
{
	return new PlaySkeletalAnimationActionTemplate(iff);
}

// ======================================================================
// private member functions
// ======================================================================

PlaySkeletalAnimationActionTemplate::PlaySkeletalAnimationActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_actorIndex(0),
	m_animationNameVariable(TAG_AANM),
	m_postureChangeEnabled(false),
	m_newServerPostureVariable(TAG_NONE),
	m_specialAttackEffectVariable(TAG_NONE),
	m_faceTrackingEnabled(false),
	m_faceTrackTargetActorIndex(-1),
	m_stopAnimationOnAbort(false)
{
	DEBUG_FATAL(!ms_installed, ("not installed."));

	iff.enterForm(TAG_PSKA);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported SkeletalAnimationAction version [%s]", buffer));
					iff.exitForm(TAG_PSKA, true);
				}
		}

	iff.exitForm(TAG_PSKA);
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
		{
			m_actorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_actorIndex < 0, ("invalid actor index %d", m_actorIndex));

			m_animationNameVariable = static_cast<Tag>(iff.read_uint32());

			// read unused trigger index variable.
			IGNORE_RETURN(iff.read_int16());
		}
		iff.exitChunk(TAG_INFO);

		//-- Handle potential posture changing support.
		if (iff.enterChunk(TAG_PSTR, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_postureChangeEnabled      = true;
			m_newServerPostureVariable = static_cast<Tag>(iff.read_uint32());

			iff.exitChunk(TAG_PSTR);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
		iff.enterChunk(TAG_INFO);
		{
			m_actorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_actorIndex < 0, ("invalid actor index %d", m_actorIndex));

			m_animationNameVariable = static_cast<Tag>(iff.read_uint32());

			// read unused trigger index variable.
			IGNORE_RETURN(iff.read_int16());
		}
		iff.exitChunk(TAG_INFO);

		//-- Handle optional posture changing support.
		if (iff.enterChunk(TAG_PSTR, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_postureChangeEnabled      = true;
			m_newServerPostureVariable = static_cast<Tag>(iff.read_uint32());

			iff.exitChunk(TAG_PSTR);
		}

		//-- Handle optional face tracking support.
		if (iff.enterChunk(TAG_FTRK, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_faceTrackingEnabled       = true;
			m_faceTrackTargetActorIndex = static_cast<int>(iff.read_int16());

			iff.exitChunk(TAG_FTRK);
		}

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void PlaySkeletalAnimationActionTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
		iff.enterChunk(TAG_INFO);
		{
			m_actorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_actorIndex < 0, ("invalid actor index %d", m_actorIndex));

			m_animationNameVariable = static_cast<Tag>(iff.read_uint32());
			m_stopAnimationOnAbort  = (iff.read_int8() != 0);
		}
		iff.exitChunk(TAG_INFO);

		//-- Handle optional posture changing support.
		if (iff.enterChunk(TAG_PSTR, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_postureChangeEnabled      = true;
			m_newServerPostureVariable = static_cast<Tag>(iff.read_uint32());

			iff.exitChunk(TAG_PSTR);
		}

		//-- Handle optional face tracking support.
		if (iff.enterChunk(TAG_FTRK, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_faceTrackingEnabled       = true;
			m_faceTrackTargetActorIndex = static_cast<int>(iff.read_int16());

			iff.exitChunk(TAG_FTRK);
		}

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------
void PlaySkeletalAnimationActionTemplate::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);
		iff.enterChunk(TAG_INFO);
		{
			m_actorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_actorIndex < 0, ("invalid actor index %d", m_actorIndex));

			m_animationNameVariable = static_cast<Tag>(iff.read_uint32());
			m_stopAnimationOnAbort  = (iff.read_int8() != 0);

			m_specialAttackEffectVariable = static_cast<Tag>(iff.read_uint32());
		}
		iff.exitChunk(TAG_INFO);

		//-- Handle optional posture changing support.
		if (iff.enterChunk(TAG_PSTR, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_postureChangeEnabled      = true;
			m_newServerPostureVariable = static_cast<Tag>(iff.read_uint32());

			iff.exitChunk(TAG_PSTR);
		}

		//-- Handle optional face tracking support.
		if (iff.enterChunk(TAG_FTRK, true))
		{
			// The mere presence of this chunk indicates the data supports posture changing.
			m_faceTrackingEnabled       = true;
			m_faceTrackTargetActorIndex = static_cast<int>(iff.read_int16());

			iff.exitChunk(TAG_FTRK);
		}

	iff.exitForm(TAG_0003);
}

// ======================================================================
