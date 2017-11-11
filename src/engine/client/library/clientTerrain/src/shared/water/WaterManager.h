// =====================================================================
//
// WaterManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
// =====================================================================

#ifndef INCLUDED_WaterManager_H
#define INCLUDED_WaterManager_H

// =====================================================================

class BoxExtent;
class Camera;
class Object;

// =====================================================================

class WaterManager
{
public:

	WaterManager ();
	virtual ~WaterManager () = 0;

	virtual void alter (float elapsedTime) = 0;
	virtual void draw () const = 0;

private:

	WaterManager (const WaterManager& rhs);
	WaterManager& operator= (const WaterManager& rhs);
};

// =====================================================================

#endif
