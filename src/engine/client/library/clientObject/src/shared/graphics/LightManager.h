//
// LightManager.h
// asommers 2000-10-25
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_LightManager_H
#define INCLUDED_LightManager_H

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

class Light;
class Iff;

//-------------------------------------------------------------------

class LightManager
{
public:

	class LightData
	{
	public:

		char       name [100];
		Vector     position;
		real       theta;
		real       phi;
		int        type;
		VectorArgb color;
		VectorArgb specularColor;
		bool       disabled;
		bool       dot3;
		bool       shadowSource;

		real       constantAttenuation;
		real       linearAttenuation;
		real       quadraticAttenuation;
		real       range;
		real       spotTheta;
		real       spotPhi;
		real       spotFalloff;

	public:

		LightData (void);
	};

private:

	ArrayList<LightData> lightDataList;

public:

	LightManager (void);
	~LightManager (void);

	int              getNumberOfLights (void) const;
	const LightData& getLight (int index) const;
	LightData&       getLight (int index);
	void             setLight (int index, const LightData& newLight);
	void             addLight (const LightData& newLight);
	void             removeLight (int index);
	void             setLights (const ArrayList<LightData>& newLightList);
	const ArrayList<LightData>& getLights (void) const;

	void             load (const char* filename);
	void             save (const char* filename) const;

	void             load (Iff&);
	void             save (Iff& iff) const;

	Light*           createLight (int index) const;
};

//-------------------------------------------------------------------

#endif
