//
// PreferencesManager.h
// asommers 2000-10-25
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef PREFERENCESMANAGER_H
#define PREFERENCESMANAGER_H

//-------------------------------------------------------------------

#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

class Iff;

//-------------------------------------------------------------------

class PreferencesManager
{
public:

	static const VectorArgb color161;

private:

	bool  dirty;

public:

	//-- editor preferences
	int           cameraMode;
	int           centerOn;
	int           movementMode;
	bool          zsortSelectionBoxes;

	//-- camera preferences
	bool          fogEnabled;
	real          fogDensity;
	real          farDistance;
	int           fov;
	real          nearDistance;
	real          endDistance;
	real          startDistance;
	int           fogMode;

	VectorArgb    fogColor;
	VectorArgb    clearColor;

	bool          showAxis;
	bool          showGrid;

//	bool          depthFog;
//	VectorArgb    fogColorLow;
//	VectorArgb    fogColorHigh;
//	real          fogLowDistance;
//	real          fogHighDistance;
//	real          fogMinAngle;
//	real          fogMaxAngle;
//	VectorArgb    clearColorLow;
//	VectorArgb    clearColorHigh;

private:

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);
	void load_0002 (Iff& iff);

public:

	PreferencesManager (void);
	~PreferencesManager (void);

	bool   isDirty (void) const;
	void   setDirty (void);
	void   setClean (void);

	void   load (Iff& iff);
	void   save (Iff& iff) const;
};

//-------------------------------------------------------------------

inline bool PreferencesManager::isDirty (void) const
{
	return dirty;
}

//-------------------------------------------------------------------

inline void PreferencesManager::setDirty (void)
{
	dirty = true;
}

//-------------------------------------------------------------------

inline void PreferencesManager::setClean (void)
{
	dirty = false;
}

//-------------------------------------------------------------------

#endif
