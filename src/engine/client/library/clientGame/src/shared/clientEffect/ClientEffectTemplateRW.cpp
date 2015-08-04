// ======================================================================
//
// ClientEffectTemplateRW.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientEffectTemplateRW.h"

#include "clientAudio/SoundTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedObject/AppearanceTemplateList.h"

#include <vector>

// ======================================================================

static const Tag TAG_CLEF = TAG (C,L,E,F);
static const Tag TAG_CPAP = TAG (C,P,A,P);
static const Tag TAG_PSND = TAG (P,S,N,D);
static const Tag TAG_CLGT = TAG (C,L,G,T);
static const Tag TAG_CAMS = TAG (C,A,M,S);
static const Tag TAG_FFBK = TAG (F,F,B,K);

// ======================================================================

ClientEffectTemplateRW::ClientEffectTemplateRW()
{
}

// ----------------------------------------------------------------------

ClientEffectTemplateRW::~ClientEffectTemplateRW()
{
}

// ----------------------------------------------------------------------

const ClientEffectTemplate::CreateAppearanceList& ClientEffectTemplateRW::getParticleSystems() const
{
	NOT_NULL (m_cpaFuncs);
	return *m_cpaFuncs;
}

// ----------------------------------------------------------------------

const ClientEffectTemplate::PlaySoundList& ClientEffectTemplateRW::getSounds() const
{
	NOT_NULL (m_psFuncs);
	return *m_psFuncs;
}

// ----------------------------------------------------------------------

const ClientEffectTemplate::CreateLightList& ClientEffectTemplateRW::getLights() const
{
	NOT_NULL (m_clFuncs);
	return *m_clFuncs;
}

// ----------------------------------------------------------------------

const ClientEffectTemplate::CameraShakeList& ClientEffectTemplateRW::getCameraShakes() const 
{
	NOT_NULL (m_csFuncs);
	return *m_csFuncs;
}

// ----------------------------------------------------------------------

const ClientEffectTemplate::ForceFeedbackList& ClientEffectTemplateRW::getForceFeedbackEffects() const 
{
	NOT_NULL (m_ffbFuncs);
	return *m_ffbFuncs;
}

// ----------------------------------------------------------------------

void ClientEffectTemplateRW::save(Iff& iff)
{
	iff.insertForm(TAG_CLEF);
		iff.insertForm(TAG_0003);

		{
			ClientEffectTemplate::CreateAppearanceList::const_iterator i = m_cpaFuncs->begin();
			for (; i != m_cpaFuncs->end(); ++i)
			{
				iff.insertChunk(TAG_CPAP);
					iff.insertChunkString(i->appearanceTemplateName.c_str());
					iff.insertChunkData(i->timeInSeconds);
					iff.insertChunkData(i->softParticleTerminate);
					iff.insertChunkData(i->minScale);
					iff.insertChunkData(i->maxScale);
					iff.insertChunkData(i->minPlaybackRate);
					iff.insertChunkData(i->maxPlaybackRate);
					iff.exitChunk(TAG_CPAP);
			}
		}

		{
			ClientEffectTemplate::PlaySoundList::const_iterator i =  m_psFuncs->begin();
			for (; i != m_psFuncs->end(); ++i)
			{
				iff.insertChunk(TAG_PSND);
					iff.insertChunkString(i->soundTemplateName.c_str());
				iff.exitChunk(TAG_PSND);
			}
		}

		{
			ClientEffectTemplate::CreateLightList::const_iterator i =  m_clFuncs->begin();
			for(; i != m_clFuncs->end(); ++i)
			{
				iff.insertChunk(TAG_CLGT);
					iff.insertChunkData(i->r);
					iff.insertChunkData(i->g);
					iff.insertChunkData(i->b);
					iff.insertChunkData(i->timeInSeconds);
					iff.insertChunkData(i->constantAttenuation);
					iff.insertChunkData(i->linearAttenuation);
					iff.insertChunkData(i->quadraticAttenuation);
					iff.insertChunkData(i->range);
				iff.exitChunk(TAG_CLGT);
			}
		}

		{
			ClientEffectTemplate::CameraShakeList::const_iterator i = m_csFuncs->begin();
			for (; i != m_csFuncs->end(); ++i)
			{
				iff.insertChunk(TAG_CAMS);
					iff.insertChunkData(i->magnitudeInMeters);
					iff.insertChunkData(i->frequencyInHz);
					iff.insertChunkData(i->timeInSeconds);
					iff.insertChunkData(i->falloffRadius);
				iff.exitChunk(TAG_CAMS);
			}
		}

		{
			ClientEffectTemplate::ForceFeedbackList::const_iterator i = m_ffbFuncs->begin();
			for (; i != m_ffbFuncs->end(); ++i)
			{
				iff.insertChunk(TAG_FFBK);
					iff.insertChunkString(i->forceFeedbackFile.c_str());
					iff.insertChunkData(i->iterations);
					iff.insertChunkData(i->range);
				iff.exitChunk(TAG_FFBK);
			}
		}

		iff.exitForm(TAG_0003);
	iff.exitForm(TAG_CLEF);
}

// ----------------------------------------------------------------------

void ClientEffectTemplateRW::clear ()
{
	ClientEffectTemplate::clear ();
}

// ======================================================================

void ClientEffectTemplateRW::addAppearance (const std::string& appearanceTemplateName, const float time, float minScale, float maxScale, float minPlaybackRate, float maxPlaybackRate, bool softTerminate)
{
	CreateAppearanceFunc appearance;
	appearance.appearanceTemplateName = appearanceTemplateName;
	appearance.appearanceTemplate = AppearanceTemplateList::fetch (appearanceTemplateName.c_str ());
	appearance.timeInSeconds = time;
	appearance.softParticleTerminate = softTerminate;
	appearance.minScale = minScale;
	appearance.maxScale = maxScale;
	appearance.minPlaybackRate = minPlaybackRate;
	appearance.maxPlaybackRate = maxPlaybackRate;

	NOT_NULL (m_cpaFuncs);
	m_cpaFuncs->push_back (appearance);
}

//----------------------------------------------------------------------

void ClientEffectTemplateRW::addSound (const std::string& soundTemplateName)
{
	PlaySoundFunc sound;
	sound.soundTemplateName = soundTemplateName;
	sound.soundTemplate = SoundTemplateList::fetch (soundTemplateName.c_str ());

	NOT_NULL (m_psFuncs);
	m_psFuncs->push_back (sound);
}

//----------------------------------------------------------------------

void ClientEffectTemplateRW::addLight (const ClientEffectTemplate::CreateLightFunc& light)
{
	NOT_NULL (m_clFuncs);
	m_clFuncs->push_back (light);
}

//----------------------------------------------------------------------

void ClientEffectTemplateRW::addCameraShake (const ClientEffectTemplate::CameraShakeFunc& cameraShake)
{
	NOT_NULL (m_csFuncs);
	m_csFuncs->push_back (cameraShake);
}

//----------------------------------------------------------------------

void ClientEffectTemplateRW::addForceFeedbackEffect (const ClientEffectTemplate::ForceFeedbackFunc& forceFeedbackEffect)
{
	NOT_NULL (m_ffbFuncs);
	m_ffbFuncs->push_back (forceFeedbackEffect);
}

// ======================================================================

