// ======================================================================
//
// ClientEffectTemplate.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientEffectTemplate.h"

#include "clientAudio/SoundTemplateList.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "clientGame/ClientEffect.h"
#include "clientGame/ClientEffectTemplateList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/AppearanceTemplateList.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_CLEF = TAG (C,L,E,F);
	const Tag TAG_CPAP = TAG (C,P,A,P);
	const Tag TAG_PSND = TAG (P,S,N,D);
	const Tag TAG_CLGT = TAG (C,L,G,T);
	const Tag TAG_CAMS = TAG (C,A,M,S);
	const Tag TAG_FFBK = TAG (F,F,B,K);

	const Tag TAG_PEFT = TAG (P,E,F,T);
	const Tag TAG_SD3D = TAG (S,D,3,D);
	const Tag TAG_SD2D = TAG (S,D,2,D);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class CreateAppearanceFuncDeleter
	{
	public:

		void operator () (const ClientEffectTemplate::CreateAppearanceFunc& createAppearanceFunc) const
		{
			if (createAppearanceFunc.appearanceTemplate)
				AppearanceTemplateList::release (createAppearanceFunc.appearanceTemplate);
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PlaySoundFuncDeleter
	{
	public:

		void operator () (const ClientEffectTemplate::PlaySoundFunc& playSoundFunc) const
		{
			if (playSoundFunc.soundTemplate)
				SoundTemplateList::release (playSoundFunc.soundTemplate);
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ForceFeedbackFuncDeleter
	{
	public:

		void operator () (ClientEffectTemplate::ForceFeedbackFunc const & forceFeedbackFunc) const
		{
			if (forceFeedbackFunc.forceFeedbackTemplate)
				ForceFeedbackEffectTemplateList::release (forceFeedbackFunc.forceFeedbackTemplate);
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

// ======================================================================

ClientEffectTemplate::CreateAppearanceFunc::CreateAppearanceFunc() :
	appearanceTemplateName (),
	appearanceTemplate (0),
	timeInSeconds (0.f),
	softParticleTerminate(false),
	minScale(1.0f),
	maxScale(1.0f),
	minPlaybackRate(1.0f),
	maxPlaybackRate(1.0f),
	ignoreDuration(false)
{
}

// ----------------------------------------------------------------------

ClientEffectTemplate::PlaySoundFunc::PlaySoundFunc () :
	soundTemplateName (),
	soundTemplate (0)
{
}

// ----------------------------------------------------------------------

ClientEffectTemplate::CreateLightFunc::CreateLightFunc () :
	r (0),
	g (0),
	b (0),
	timeInSeconds (0.f),
	constantAttenuation (0.f),
	linearAttenuation (0.f),
	quadraticAttenuation (0.f),
	range (0.f)
{
}

// ----------------------------------------------------------------------

ClientEffectTemplate::CameraShakeFunc::CameraShakeFunc () :
	magnitudeInMeters (0.f),
	frequencyInHz (0.f),
	timeInSeconds (0.f),
	falloffRadius (0.f)
{
}

// ----------------------------------------------------------------------

ClientEffectTemplate::ForceFeedbackFunc::ForceFeedbackFunc () :
forceFeedbackFile(),
forceFeedbackTemplate(NULL),
iterations(1),
range(0.f)
{
}

// ======================================================================

/** Construct a ClientEffectTemplate
 */
ClientEffectTemplate::ClientEffectTemplate(Iff& iff, const CrcLowerString& effectName) :
	m_referenceCount(0),
	m_name(new CrcLowerString (effectName)),
	m_cpaFuncs(new CreateAppearanceList),
	m_psFuncs(new PlaySoundList),
	m_clFuncs(new CreateLightList),
	m_csFuncs(new CameraShakeList),
	m_ffbFuncs(new ForceFeedbackList)
{
	load(iff);
}

// ----------------------------------------------------------------------

ClientEffectTemplate::ClientEffectTemplate() :
	m_referenceCount(0),
	m_name(new CrcLowerString ("")),
	m_cpaFuncs(new CreateAppearanceList),
	m_psFuncs(new PlaySoundList),
	m_clFuncs(new CreateLightList),
	m_csFuncs(new CameraShakeList),
	m_ffbFuncs(new ForceFeedbackList)
{
}

// ----------------------------------------------------------------------

/** Destroy a ClientEffectTemplate.
 */
ClientEffectTemplate::~ClientEffectTemplate(void)
{
	clear ();

	delete m_name;
	delete m_cpaFuncs;
	delete m_psFuncs;
	delete m_clFuncs;
	delete m_csFuncs;
	delete m_ffbFuncs;
}

//----------------------------------------------------------------------

/** Get the CrcLowerString for this ClientEffectTemplate.
 */
const CrcLowerString &ClientEffectTemplate::getCrcName() const
{
	NOT_NULL (m_name);
	return *m_name;
}

// ----------------------------------------------------------------------

/** Get the name of this ClientEffectTemplate.
 *  This routine may return NULL.
 */
const char *ClientEffectTemplate::getName() const
{
	NOT_NULL (m_name);
	return m_name->getString();
}

// ======================================================================

/** General load rountine, calls version-specific internal versions
 *  @pre iff open, positioned outside form of a clienteffecttemplate (tag = CLEF)
 *  @post iff open, positioned just after the clienteffectemplate form
 */
void ClientEffectTemplate::load(Iff& iff)
{
	// check to see if this is a particle effect or sound template
	Tag currentTag = iff.getCurrentName();

	switch (currentTag)
	{
		case TAG_PEFT:
			{
				loadFromParticleEffectTemplate(iff.getFileName());
			}
			break;
		case TAG_SD2D:
		case TAG_SD3D:
			{
				loadFromSoundTemplate(iff.getFileName());
			}
			break;
		case TAG_CLEF:
			{
				iff.enterForm(TAG_CLEF);

					switch (iff.getCurrentName())
					{
						case TAG_0001:
							load_0001(iff);
							break;

						//particle soft terminate
						case TAG_0002:
							load_0002(iff);
							break;

						//particle size/speed scaling min/maxes
						case TAG_0003:
							load_0003(iff);
							break;

						default:
							{
								char buffer[128];
								iff.formatLocation(buffer, sizeof(buffer));
								DEBUG_FATAL(true, ("bad version number at %s", buffer));
							}
					}

				iff.exitForm(TAG_CLEF);
			}
			break;
		default:
			break;
	}
}

// ----------------------------------------------------------------------

/** Load routine for version 0001 of clienteffecttemplates (the initial release)
 *  @pre iff open, positioned outside version 0001 clienteffecttemplate (tag = 0001)
 *  @post iff open, positioned just after the 0001 versioned-clienteffectemplate
 */
void ClientEffectTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	//for each command, load it
	while(!iff.atEndOfForm())
	{
		switch (iff.getCurrentName())
		{
			case TAG_CPAP:
			{
				iff.enterChunk(TAG_CPAP);
					CreateAppearanceFunc f;
					iff.read_string(f.appearanceTemplateName);
					f.timeInSeconds = iff.read_float();
					f.appearanceTemplate = AppearanceTemplateList::fetch (f.appearanceTemplateName.c_str ());
				iff.exitChunk(TAG_CPAP);
				m_cpaFuncs->push_back(f);
				break;
			}

			case TAG_PSND:
			{
				iff.enterChunk(TAG_PSND);
					PlaySoundFunc f;
					iff.read_string(f.soundTemplateName);
					f.soundTemplate = SoundTemplateList::fetch (f.soundTemplateName.c_str ());
				iff.exitChunk(TAG_PSND);
				m_psFuncs->push_back(f);
				break;
			}

			case TAG_CLGT:
			{
				iff.enterChunk(TAG_CLGT);
					CreateLightFunc f;
					f.r                    = iff.read_uint8();
					f.g                    = iff.read_uint8();
					f.b                    = iff.read_uint8();
					f.timeInSeconds        = iff.read_float();
					f.constantAttenuation  = iff.read_float();
					f.linearAttenuation    = iff.read_float();
					f.quadraticAttenuation = iff.read_float();
					f.range                = iff.read_float();
				iff.exitChunk(TAG_CLGT);
				m_clFuncs->push_back(f);
				break;
			}

			case TAG_CAMS:
			{
				iff.enterChunk(TAG_CAMS);
					CameraShakeFunc f;
					f.magnitudeInMeters = iff.read_float();
					f.frequencyInHz = iff.read_float();
					f.timeInSeconds = iff.read_float();
					f.falloffRadius = iff.read_float();
				iff.exitChunk(TAG_CAMS);
				m_csFuncs->push_back(f);
				break;
			}

			case TAG_FFBK:
			{
				iff.enterChunk(TAG_FFBK);
					ForceFeedbackFunc f;
					iff.read_string(f.forceFeedbackFile);
					f.forceFeedbackTemplate = ForceFeedbackEffectTemplateList::fetch (f.forceFeedbackFile.c_str ());
					f.iterations = iff.read_int32();
					f.range = iff.read_float();
				iff.exitChunk(TAG_FFBK);
				m_ffbFuncs->push_back(f);
				break;
			}

			default:
			{
				char buffer[128];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_WARNING(true, ("bad command type at %s", buffer));

				iff.enterChunk();
				iff.exitChunk(true);
			}
		}
	}
	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

/** Load routine for version 0002 of clienteffecttemplates (the initial release)
 *  Version 0002 adds a particle effect soft terminate field
 *  @pre iff open, positioned outside version 0002 clienteffecttemplate (tag = 0002)
 *  @post iff open, positioned just after the 0002 versioned-clienteffectemplate
 */
void ClientEffectTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
	//for each command, load it
	while(!iff.atEndOfForm())
	{
		switch (iff.getCurrentName())
		{
			case TAG_CPAP:
			{
				iff.enterChunk(TAG_CPAP);
					CreateAppearanceFunc f;
					iff.read_string(f.appearanceTemplateName);
					f.timeInSeconds = iff.read_float();
					f.softParticleTerminate = iff.read_bool8();
					f.appearanceTemplate = AppearanceTemplateList::fetch (f.appearanceTemplateName.c_str ());
				iff.exitChunk(TAG_CPAP);
				m_cpaFuncs->push_back(f);
				break;
			}

			case TAG_PSND:
			{
				iff.enterChunk(TAG_PSND);
					PlaySoundFunc f;
					iff.read_string(f.soundTemplateName);
					f.soundTemplate = SoundTemplateList::fetch (f.soundTemplateName.c_str ());
				iff.exitChunk(TAG_PSND);
				m_psFuncs->push_back(f);
				break;
			}

			case TAG_CLGT:
			{
				iff.enterChunk(TAG_CLGT);
					CreateLightFunc f;
					f.r                    = iff.read_uint8();
					f.g                    = iff.read_uint8();
					f.b                    = iff.read_uint8();
					f.timeInSeconds        = iff.read_float();
					f.constantAttenuation  = iff.read_float();
					f.linearAttenuation    = iff.read_float();
					f.quadraticAttenuation = iff.read_float();
					f.range                = iff.read_float();
				iff.exitChunk(TAG_CLGT);
				m_clFuncs->push_back(f);
				break;
			}

			case TAG_CAMS:
			{
				iff.enterChunk(TAG_CAMS);
					CameraShakeFunc f;
					f.magnitudeInMeters = iff.read_float();
					f.frequencyInHz = iff.read_float();
					f.timeInSeconds = iff.read_float();
					f.falloffRadius = iff.read_float();
				iff.exitChunk(TAG_CAMS);
				m_csFuncs->push_back(f);
				break;
			}

			case TAG_FFBK:
			{
				iff.enterChunk(TAG_FFBK);
					ForceFeedbackFunc f;
					iff.read_string(f.forceFeedbackFile);
					f.forceFeedbackTemplate = ForceFeedbackEffectTemplateList::fetch (f.forceFeedbackFile.c_str ());
					f.iterations = iff.read_int32();
					f.range = iff.read_float();
				iff.exitChunk(TAG_FFBK);
				m_ffbFuncs->push_back(f);
				break;
			}

			default:
			{
				char buffer[128];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_WARNING(true, ("bad command type at %s", buffer));

				iff.enterChunk();
				iff.exitChunk(true);
			}
		}
	}
	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

/** Load routine for version 0003 of clienteffecttemplates (the initial release)
 *  Version 0003 adds particle appearance scale min, scale max, playback min, and playback max
 *  @pre iff open, positioned outside version 0003 clienteffecttemplate (tag = 0003)
 *  @post iff open, positioned just after the 0003 versioned-clienteffectemplate
 */
void ClientEffectTemplate::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);
	//for each command, load it
	while(!iff.atEndOfForm())
	{
		switch (iff.getCurrentName())
		{
			case TAG_CPAP:
			{
				iff.enterChunk(TAG_CPAP);
					CreateAppearanceFunc f;
					iff.read_string(f.appearanceTemplateName);
					f.timeInSeconds = iff.read_float();
					f.softParticleTerminate = iff.read_bool8();
					f.minScale = iff.read_float();
					f.maxScale = iff.read_float();
					f.minPlaybackRate = iff.read_float();
					f.maxPlaybackRate = iff.read_float();
					f.appearanceTemplate = AppearanceTemplateList::fetch (f.appearanceTemplateName.c_str ());
				iff.exitChunk(TAG_CPAP);
				m_cpaFuncs->push_back(f);
				break;
			}

			case TAG_PSND:
			{
				iff.enterChunk(TAG_PSND);
					PlaySoundFunc f;
					iff.read_string(f.soundTemplateName);
					f.soundTemplate = SoundTemplateList::fetch (f.soundTemplateName.c_str ());
				iff.exitChunk(TAG_PSND);
				m_psFuncs->push_back(f);
				break;
			}

			case TAG_CLGT:
			{
				iff.enterChunk(TAG_CLGT);
					CreateLightFunc f;
					f.r                    = iff.read_uint8();
					f.g                    = iff.read_uint8();
					f.b                    = iff.read_uint8();
					f.timeInSeconds        = iff.read_float();
					f.constantAttenuation  = iff.read_float();
					f.linearAttenuation    = iff.read_float();
					f.quadraticAttenuation = iff.read_float();
					f.range                = iff.read_float();
				iff.exitChunk(TAG_CLGT);
				m_clFuncs->push_back(f);
				break;
			}

			case TAG_CAMS:
			{
				iff.enterChunk(TAG_CAMS);
					CameraShakeFunc f;
					f.magnitudeInMeters = iff.read_float();
					f.frequencyInHz = iff.read_float();
					f.timeInSeconds = iff.read_float();
					f.falloffRadius = iff.read_float();
				iff.exitChunk(TAG_CAMS);
				m_csFuncs->push_back(f);
				break;
			}

			case TAG_FFBK:
			{
				iff.enterChunk(TAG_FFBK);
					ForceFeedbackFunc f;
					iff.read_string(f.forceFeedbackFile);
					f.forceFeedbackTemplate = ForceFeedbackEffectTemplateList::fetch (f.forceFeedbackFile.c_str ());
					f.iterations = iff.read_int32();
					f.range = iff.read_float();
				iff.exitChunk(TAG_FFBK);
				m_ffbFuncs->push_back(f);
				break;
			}

			default:
			{
				char buffer[128];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_WARNING(true, ("bad command type at %s", buffer));

				iff.enterChunk();
				iff.exitChunk(true);
			}
		}
	}
	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

/** Create a position-based effect from this template
 */
ClientEffect* ClientEffectTemplate::createClientEffect(const CellProperty* cell, const Vector& position, const Vector& up) const
{
	return new PositionClientEffect(this, cell, position, up);
}

// ----------------------------------------------------------------------

/** Create an object-based effect from this template
 */
ClientEffect* ClientEffectTemplate::createClientEffect(Object* object, const CrcLowerString& hardPointName) const
{
	return new ObjectClientEffect(this, object, hardPointName);
}

//----------------------------------------------------------------------

ClientEffect * ClientEffectTemplate::createClientEffect(Object* object, Transform const & transform) const
{
	return new ObjectClientEffect(this, object, transform);
}

// ----------------------------------------------------------------------

/** Increment the reference count to this template
 */
void ClientEffectTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

/** Decrement the reference count to this template, deleting it if necessary
 */
void ClientEffectTemplate::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		//ensure list is no longer tracking us
		ClientEffectTemplateList::stopTracking(this);

		//delete this instance
		delete const_cast<ClientEffectTemplate*>(this);
	}
}

// ----------------------------------------------------------------------

int ClientEffectTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ======================================================================

void ClientEffectTemplate::clear ()
{
	*m_name = CrcLowerString::empty;

	std::for_each (m_cpaFuncs->begin (), m_cpaFuncs->end (), CreateAppearanceFuncDeleter ());
	m_cpaFuncs->clear ();

	std::for_each (m_psFuncs->begin (), m_psFuncs->end (), PlaySoundFuncDeleter ());
	m_psFuncs->clear ();

	std::for_each (m_ffbFuncs->begin (), m_ffbFuncs->end (), ForceFeedbackFuncDeleter ());
	m_ffbFuncs->clear ();
	
	m_clFuncs->clear ();
	m_csFuncs->clear ();
	m_ffbFuncs->clear();
}

// ======================================================================

void ClientEffectTemplate::loadFromParticleEffectTemplate (const char * const appearanceTemplateName)
{
	CreateAppearanceFunc appearance;
	appearance.appearanceTemplateName = appearanceTemplateName;
	appearance.appearanceTemplate = AppearanceTemplateList::fetch (appearanceTemplateName);

	appearance.softParticleTerminate = true;
	appearance.ignoreDuration = true;

	NOT_NULL (m_cpaFuncs);
	m_cpaFuncs->push_back (appearance);
}

void ClientEffectTemplate::loadFromSoundTemplate (const std::string& soundTemplateName)
{
	PlaySoundFunc sound;
	sound.soundTemplateName = soundTemplateName;
	sound.soundTemplate = SoundTemplateList::fetch (soundTemplateName.c_str ());

	NOT_NULL (m_psFuncs);
	m_psFuncs->push_back (sound);
}


