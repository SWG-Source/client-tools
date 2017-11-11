// ======================================================================
//
// ReticleManager.h
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ReticleManager_H
#define INCLUDED_ReticleManager_H

// ======================================================================
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class Appearance;
class ReticleObject;
class Vector;

// ======================================================================

class ReticleManager
{
public:
	enum ReticleManagerType
	{
		RMT_HeavyWeapons = 0,
		RMT_StoryTeller,
		RMT_GrenadeTarget,

		RMT_TotalTypes,
	};

	static void install ();

	static bool getEnabled ();
	static void setEnabled (bool enabled);

	static void enableReticleObject (const ReticleObject& ReticleObject);
	static void disableReticleObject (const ReticleObject& ReticleObject);

	static void renderReticles (const Appearance& appearance);
	static void addVertex (const Vector& position, const float u, const float v);

	static void update (float elapsedTime);

	static ReticleObject *getReticleObject();
	static void giveBackReticleObject(ReticleObject *reticle);

	static void setLastGroundReticlePoint(const Vector &lastGroundReticlePoint);
	static const Vector &getLastGroundReticlePoint();

	static void setLastGroundReticleCell(const NetworkId &lastGroundReticleCell);
	static const NetworkId &getLastGroundReticleCell();

	static void setReticleCurrentlyValid(bool reticleValid);
	static bool getReticleCurrentlyValid(void);
	
	static void setRenderReticles(bool reticleValid);
	static bool getRenderReticles(void);

	static void setReticleType(ReticleManagerType const & type);

private:

	ReticleManager ();
	~ReticleManager ();
	ReticleManager (const ReticleManager&);
	ReticleManager& operator= (const ReticleManager&);
};

// ======================================================================

#endif
