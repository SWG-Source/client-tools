// ============================================================================
//
// LightningManager.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_LightningManager_H
#define INCLUDED_LightningManager_H

class LightningBolt;
class Vector;

//-----------------------------------------------------------------------------
class LightningManager
{
public:

	static void                 install();
	static void                 remove();
	static void                 alter(float const deltaTime);

	static int                  getLightningBoltCount();
	static LightningBolt const &getLightningBolt(int const index);

private:

	// Disabled

	LightningManager();
	~LightningManager();
	LightningManager(const LightningManager &);
	LightningManager &operator =(const LightningManager &);
};

// ============================================================================

#endif // INCLUDED_LightningManager_H
