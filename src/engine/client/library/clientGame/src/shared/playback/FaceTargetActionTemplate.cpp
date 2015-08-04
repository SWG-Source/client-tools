// ======================================================================
//
// FaceTargetActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FaceTargetActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/FaceTargetAction.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_FACE = TAG(F,A,C,E);
const Tag TAG_IDRD = TAG(I,D,R,D);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool FaceTargetActionTemplate::ms_installed;

// ======================================================================
// class FaceTargetActionTemplate: public static member functions
// ======================================================================

void FaceTargetActionTemplate::install()
{
	InstallTimer const installTimer("FaceTargetActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("FaceTargetActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_FACE, create);

	ms_installed = true;
	ExitChain::add(remove, "FaceTargetActionTemplate");
}

// ======================================================================
// class FaceTargetActionTemplate: public member functions
// ======================================================================

PlaybackAction *FaceTargetActionTemplate::createPlaybackAction() const
{
	return new FaceTargetAction(*this);
}

// ======================================================================
// class FaceTargetActionTemplate: private static member functions
// ======================================================================

void FaceTargetActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("FaceTargetActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_FACE);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *FaceTargetActionTemplate::create(Iff &iff)
{
	return new FaceTargetActionTemplate(iff);
}

// ======================================================================
// class FaceTargetActionTemplate: private member functions
// ======================================================================

FaceTargetActionTemplate::FaceTargetActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_rotationActorIndex(-1),
	m_targetActorIndex(-1),
	m_maxRotationalSpeedVariable(TAG_NONE),
	m_onTargetThresholdAngleVariable(TAG_NONE),
	m_maxTimeToLiveVariable(TAG_NONE),
	m_ignoreDeadReckoningDurationVariable(TAG_IDRD)
{
	DEBUG_FATAL(!ms_installed, ("FaceTargetActionTemplate not installed."));

	iff.enterForm(TAG_FACE);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("unsupported FaceTargetActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_FACE);
}

// ----------------------------------------------------------------------

void FaceTargetActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_rotationActorIndex = static_cast<int>(iff.read_int16());
			m_targetActorIndex   = static_cast<int>(iff.read_int16());

			m_maxRotationalSpeedVariable     = static_cast<Tag>(iff.read_uint32());
			m_onTargetThresholdAngleVariable = static_cast<Tag>(iff.read_uint32());
			m_maxTimeToLiveVariable          = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void FaceTargetActionTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_INFO);

			m_rotationActorIndex                  = static_cast<int>(iff.read_int16());
			m_targetActorIndex                    = static_cast<int>(iff.read_int16());

			m_maxRotationalSpeedVariable          = static_cast<Tag>(iff.read_uint32());
			m_onTargetThresholdAngleVariable      = static_cast<Tag>(iff.read_uint32());
			m_maxTimeToLiveVariable               = static_cast<Tag>(iff.read_uint32());
			m_ignoreDeadReckoningDurationVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0001);
}

// ======================================================================
