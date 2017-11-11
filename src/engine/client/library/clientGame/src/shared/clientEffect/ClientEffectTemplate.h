// ======================================================================
//
// ClientEffectTemplate.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================


#ifndef INCLUDED_ClientEffectTemplate_H
#define INCLUDED_ClientEffectTemplate_H

// ======================================================================

class AppearanceTemplate;
class CellProperty;
class ClientEffect;
class CrcLowerString;
class ForceFeedbackEffectTemplate;
class Iff;
class Object;
class SoundTemplate;
class Transform;
class Vector;

// ======================================================================

/** This class represents a template used to create ClientEffects.  It stores lists of simple "commands" to run.
 *  Since a client effect is one-shot with no timing information, there is no guaranteed order in which the commands
 *  will run (although they occur in the same update loop).  Instance specific information is not stored here, such as
 *  the location in space or object on which to play the effect.  That data is stored within the ClientEffect or derived class
 */
class ClientEffectTemplate
{
/* Allow the client effects access to it's template data (the commands to run).  These classes directly access 
   the data structures for the commands, making them implementation-dependent on this class.
*/
friend class ClientEffect;
friend class PositionClientEffect;
friend class ObjectClientEffect;

public:
	//define all the different command types
	struct CreateAppearanceFunc
	{
		CreateAppearanceFunc();

		std::string appearanceTemplateName;
		const AppearanceTemplate* appearanceTemplate;
		float timeInSeconds;
		bool softParticleTerminate;
		float minScale;
		float maxScale;
		float minPlaybackRate;
		float maxPlaybackRate;
		bool  ignoreDuration;
	};

	struct PlaySoundFunc
	{
		PlaySoundFunc ();

		std::string soundTemplateName;
		const SoundTemplate* soundTemplate;
	};

	struct CreateLightFunc
	{
		CreateLightFunc ();

		uint8 r;
		uint8 g;
		uint8 b;
		float timeInSeconds;
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
		float range;
	};

	struct CameraShakeFunc
	{
		CameraShakeFunc ();

		float magnitudeInMeters;
		float frequencyInHz;
		float timeInSeconds;
		float falloffRadius;
	};

	struct ForceFeedbackFunc
	{
		ForceFeedbackFunc ();

		std::string forceFeedbackFile;
		ForceFeedbackEffectTemplate const * forceFeedbackTemplate;
		int iterations;
		float range;
	};

	typedef stdvector<CreateAppearanceFunc>::fwd CreateAppearanceList;
	typedef stdvector<PlaySoundFunc>::fwd        PlaySoundList;
	typedef stdvector<CreateLightFunc>::fwd      CreateLightList;
	typedef stdvector<CameraShakeFunc>::fwd      CameraShakeList;
	typedef stdvector<ForceFeedbackFunc>::fwd    ForceFeedbackList;

///////////////////////////////

public:
	ClientEffectTemplate(Iff& iff, const CrcLowerString& effectName);
	virtual ~ClientEffectTemplate(void);

	const CrcLowerString& getCrcName() const;
	const char* getName() const;

	ClientEffect* createClientEffect(const CellProperty* cell, const Vector& position, const Vector& up) const;
	ClientEffect* createClientEffect(Object* object, const CrcLowerString& hardpoint) const;
	ClientEffect* createClientEffect(Object* object, Transform const & transform) const;

	void fetch() const;
	void release() const;
	int  getReferenceCount() const;

	void load(Iff& iff);

protected:
	ClientEffectTemplate();
	void load_0001(Iff& iff);
	void load_0002(Iff& iff);
	void load_0003(Iff& iff);

	void clear ();

	void loadFromParticleEffectTemplate (const char * const appearanceTemplateName);
	void loadFromSoundTemplate (const std::string& soundTemplateName);


private:
	//disabled
	ClientEffectTemplate(const ClientEffectTemplate&);
	ClientEffectTemplate& operator=(const ClientEffectTemplate&);

protected:
	mutable int    m_referenceCount;
	///the name of the effect template file
	CrcLowerString* const       m_name;

	CreateAppearanceList* const m_cpaFuncs;
	PlaySoundList* const        m_psFuncs;
	CreateLightList* const      m_clFuncs;
	CameraShakeList* const      m_csFuncs;
	ForceFeedbackList* const    m_ffbFuncs;
};

// ======================================================================

#endif
