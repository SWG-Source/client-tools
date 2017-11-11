// ======================================================================
//
// VehicleHoverDynamicsClient.h
// copyright 2003, sony online entertainment
//
//
// ======================================================================

#ifndef INCLUDED_VehicleHoverDynamicsClient_H
#define INCLUDED_VehicleHoverDynamicsClient_H

// ======================================================================

#include "sharedGame/VehicleHoverDynamics.h"
#include "clientAudio/SoundId.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/TemporaryCrcString.h"

class ParticleEffectAppearance;
class SoundId;
class ClientDataFile;
class VehicleThrusterSoundData;
class VehicleGroundEffectData;
class AppearanceTemplate;

// ======================================================================

class VehicleHoverDynamicsClient : public VehicleHoverDynamics
{
public:

	explicit VehicleHoverDynamicsClient (Object* newOwner, float yaw, float hoverHeight, const char * clientDataFilename);
	virtual ~VehicleHoverDynamicsClient ();

	virtual float alter(float time);

	typedef stdvector<ParticleEffectAppearance *>::fwd ParticleEffectVector;
	typedef stdvector<SoundId>::fwd SoundIdVector;

protected:

	virtual void showDebugBoxHoverPlane (const Vector & lookAhead) const;
	virtual void showDebugBoxHeight     (const Vector & lookAhead) const;
	virtual void updateCrumbTrail       () const;
	void         updateGroundEffects    (float elapsedTime, float speedPercent);
	void         updateLightningEffects (float elapsedTime);
	
private:
	
	// disable these
	VehicleHoverDynamicsClient ();
	VehicleHoverDynamicsClient (const VehicleHoverDynamicsClient&);
	VehicleHoverDynamicsClient& operator= (const VehicleHoverDynamicsClient&);
	
private:
	
	class SoundIdData
	{
	public:
		
		SoundIdData () :
			m_idle (),
			m_run (),
			m_accel (),
			m_decel (),
			m_fadeInVolume (1.0f),
			m_fadeInRate   (1.0f),
			m_fadeInOffset (0.0f),
			m_fadeInLoaded (false),
			m_fadeInSoundId (),
			m_stateCurrent (SS_idle),
			m_stateTarget  (SS_idle),
			m_dynamics (0),
			m_accelTotalSoundTime (0),
			m_decelTotalSoundTime (0)
		{
		}
		
		SoundIdData (VehicleHoverDynamicsClient & vhdc) :
			m_idle (),
			m_run (),
			m_accel (),
			m_decel (),
			m_fadeInVolume (1.0f),
			m_fadeInRate   (1.0f),
			m_fadeInOffset (0.0f),
			m_fadeInLoaded (false),
			m_fadeInSoundId (),
			m_stateCurrent (SS_idle),
			m_stateTarget  (SS_idle),
			m_dynamics (&vhdc),
			m_accelTotalSoundTime (0),
			m_decelTotalSoundTime (0)		
		{
		}
		
		enum SoundState
		{
			SS_idle,
				SS_accel,
				SS_decel,
				SS_run
		};
		
		SoundId m_idle;
		SoundId m_run;
		SoundId m_accel;
		SoundId m_decel;
		
		float      m_fadeInVolume;
		float      m_fadeInRate;
		float      m_fadeInOffset;
		bool       m_fadeInLoaded;
		SoundId    m_fadeInSoundId;
		
		SoundState m_stateCurrent;
		SoundState m_stateTarget;

		VehicleHoverDynamicsClient * m_dynamics;

		int        m_accelTotalSoundTime;
		int        m_decelTotalSoundTime; 

		SoundId & getSoundIdByState (SoundState s)
		{
			if (s == SS_idle)
				return m_idle;
			else if (s == SS_accel)
				return m_accel;
			else if (s == SS_decel)
				return m_decel;
			else
				return m_run;
		}

		SoundState getOppositeState (SoundState s)
		{
			if (s == SS_idle)
				return SS_run;
			else if (s == SS_accel)
				return SS_decel;
			else if (s == SS_decel)
				return SS_accel;
			else
				return SS_idle;
		}

		SoundState getIntermediateTransitionState (SoundState s)
		{
			if (s == SS_idle)
				return SS_accel;
			else if (s == SS_run)
				return SS_decel;
			
			return SS_run;
		}

		int & getTotalTimeHolder (SoundState s)
		{
			if (s == SS_accel)
				return m_accelTotalSoundTime;

			return m_decelTotalSoundTime;
		}


		void update (float elapsedTime, float desiredSpeed, float speedPercent);
		void fadeSoundIn (SoundId & id, float fadeInTime, int offsetStartMs);

		void transitionToIntermediateState ();
		void transitionToFinalState ();
		void transitionToReverseState ();
	};
	
	void updateSounds (float desiredSpeed, float elapsedTime, float speedPercent);
	void updateSoundData (const VehicleThrusterSoundData & vtsd);
	void updateSoundId (SoundIdData & data, SoundId & soundId, const std::string & soundTemplateName);
	void updateSoundFadeIn (SoundIdData & data);

	ParticleEffectVector * m_engineParticleEffects;
	
	SoundIdData m_engineSounds;
	SoundIdData m_engineSoundsDamaged;

	float      m_lastDamageLevel;

	const ClientDataFile * m_clientDataFile;

	int        m_hasAltered;

	class GroundEffectClientData
	{
	public:
		Object * obj;
		SoundId  soundId;
		Transform transform;
		ParticleEffectAppearance * pea;
		Object * old_obj;
		ParticleEffectAppearance * old_pea;

		float baseEffectScale;

		const VehicleGroundEffectData * vged;

		GroundEffectClientData (Object & parent, const VehicleGroundEffectData & _vged);

		void manageEffectOnOff (Object & parent, bool valid);
		void manageEffect (Object & parent, float speedPercent);
	};

	typedef stdvector<GroundEffectClientData>::fwd GroundEffectClientDataVector;
	GroundEffectClientDataVector * m_groundEffects;

	float m_lastSpeedPercent; 

	float m_lastHeadTurnPercent;


	class LightningEffectData
	{
	public:
		ConstWatcher<Object> m_parentObject;
		Object * m_lightningObject;
		TemporaryCrcString m_startHardpoint;
		TemporaryCrcString m_endHardpoint;

		LightningEffectData (Object & parent, char const * const startHardpoint, char const * const endHardpoint, char const * const appearancePath);
		~LightningEffectData();
		void update(float deltaTimeSeconds);
	};

	typedef stdvector<LightningEffectData *>::fwd LightningEffectDataVector;
	LightningEffectDataVector * m_lightningEffects;
};

// ======================================================================

#endif
