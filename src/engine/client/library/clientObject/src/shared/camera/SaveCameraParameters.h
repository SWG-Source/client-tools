//
// SaveCameraParameters.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_SaveCameraParameters_H
#define INCLUDED_SaveCameraParameters_H

//-------------------------------------------------------------------

#include "clientObject/GameCamera.h"
#include "sharedMath/PackedRgb.h"
#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

class SaveCameraParameters
{
private:

	real          farDistance;
	real          nearDistance;
	real          fov;

public:

	SaveCameraParameters (void);
	~SaveCameraParameters (void);

	void save (const GameCamera* camera);
	void setUnderWater (GameCamera* camera, PackedRgb& backgroundColor);
	void restore (GameCamera* camera);
};

//-------------------------------------------------------------------

inline SaveCameraParameters::SaveCameraParameters (void)
{
}

//-------------------------------------------------------------------

inline SaveCameraParameters::~SaveCameraParameters (void)
{
}

//-------------------------------------------------------------------

inline void SaveCameraParameters::save (const GameCamera* camera)
{
	fov           = camera->getHorizontalFieldOfView ();
	nearDistance  = camera->getNearPlane ();
	farDistance   = camera->getFarPlane ();
}

//-------------------------------------------------------------------

inline void SaveCameraParameters::setUnderWater (GameCamera* camera, PackedRgb& backgroundColor)
{
	backgroundColor.r = 77;
	backgroundColor.g = 115;
	backgroundColor.b = 113;

	camera->setNearPlane (0.1f);
	camera->setFarPlane  (500.f);
	camera->setHorizontalFieldOfView (fov);
	camera->setUnderWaterThisFrame (true);
}

//-------------------------------------------------------------------

inline void SaveCameraParameters::restore (GameCamera* camera)
{
	camera->setNearPlane (nearDistance);
	camera->setFarPlane  (farDistance);
	camera->setHorizontalFieldOfView (fov);
	camera->setUnderWaterThisFrame (false);
}

//-------------------------------------------------------------------

#endif
