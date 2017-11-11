//===================================================================
//
// MissileManager.h
//
// copyright 2004, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_MissileManager_H
#define INCLUDED_MissileManager_H

//===================================================================

class AppearanceTemplate;
class ClientEffectTemplate;
class Countermeasure;
class Missile;
class NetworkId;
class Object;
class Vector;

//===================================================================

class MissileManager
{
public:
	static void install                       ();
	static void remove                        ();

public:
	static void addMissile                    (int const missileId, NetworkId const &source, NetworkId const &target, Vector const & sourceLocation, Vector const & targetLocation, int const impactTime, int const missileTypeId, int const weaponIndex, int const targetComponent);
	static void removeMissile                 (int missileId);

	static void addCountermeasure             (Countermeasure * const countermeasure);
	static void removeCountermeasure          (Countermeasure const * const countermeasure);
	
	static void onServerMissileHitTarget      (int missileId);
	static void onServerMissileMissedTarget   (int missileId);
	static void onServerMissileCountermeasured(int const missileId, int const countermeasureType);
	static void onServerMissileCountermeasureFailed(NetworkId const & shipId, int const countermeasureType);

	static float getTargetAcquisitionSeconds(int missileType);
	static float getTargetAcquisitionAngle(int missileType);

	static float getDefaultTargetAcquisitionSeconds();
	static float getDefaultTargetAcquisitionAngle();

	static float getTime(int missileId);
	static float getRange(int missileId);
	static float getSpeed(int missileId);

private:
	static Missile * getMissile               (int missileId);

private:
	struct MissileTypeDataRecord
	{
	public:

		MissileTypeDataRecord();
		void preloadResources();
		void releaseResources();

	public:

		int m_typeId;
		const AppearanceTemplate * m_missileAppearance;
		const AppearanceTemplate * m_trailAppearance;
		const ClientEffectTemplate * m_fireEffect;
		const ClientEffectTemplate * m_hitEffect;
		const ClientEffectTemplate * m_countermeasureEffect;
		float m_speed;
		float m_targetAcquisitionSeconds;
		float m_targetAcquisitionAngle;
		float m_time;
		std::string m_missileAppearanceName;
		std::string m_trailAppearanceName;
		std::string m_fireEffectName;
		std::string m_hitEffectName;
		std::string m_countermeasureEffectName;

	private:

		bool m_preloaded;
	};


	struct CountermeasureTypeDataRecord
	{
	public:

		CountermeasureTypeDataRecord();
		void preloadResources();
		void releaseResources();

	public:

		int m_typeId;
		AppearanceTemplate const * m_appearance;
		std::string m_appearanceName;
		int m_minNumber;
		int m_maxNumber;
		float m_minSpeed;
		float m_maxSpeed;
		float m_maxScatterAngle;

	private:

		bool m_preloaded;
	};
	
	static Countermeasure * launchCountermeasure(Object const & sourceObject, int const countermeasureType);

	typedef stdmap<int, MissileTypeDataRecord>::fwd MissileTypeDataType;
	typedef stdmap<int, Missile*>::fwd MissilesType;
	typedef stdvector<Countermeasure*>::fwd CountermeasuresType;
	typedef stdmap<int, CountermeasureTypeDataRecord>::fwd CountermeasureTypeDataType;

	static bool           ms_installed;
	static MissilesType * ms_missiles;
	static MissileTypeDataType * ms_missileTypeData;
	static CountermeasuresType * ms_countermeasures;
	static size_t ms_maxCountermeasures;
	static CountermeasureTypeDataType * ms_countermeasureTypeData;
};

//===================================================================

#endif
