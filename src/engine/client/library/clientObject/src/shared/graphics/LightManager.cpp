//
// LightManager.cpp
// asommers 2000-10-25
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "clientObject/FirstClientObject.h"
#include "clientObject/LightManager.h"

#include "clientGraphics/Light.h"
#include "sharedFile/Iff.h"

//-------------------------------------------------------------------

LightManager::LightData::LightData (void) :
	position (),
	theta (0),
	phi (0),
	type (Light::T_parallel),
	color (VectorArgb::solidWhite),
	specularColor (VectorArgb::solidBlack),
	disabled (false),
	dot3 (false),
	shadowSource (false),
	constantAttenuation (0),
	linearAttenuation (0),
	quadraticAttenuation (0),
	range (0),
	spotTheta (0),
	spotPhi (0),
	spotFalloff (0)
{
	name [0] = 0;
}

//-------------------------------------------------------------------
	
LightManager::LightManager (void)
{
}

//-------------------------------------------------------------------

LightManager::~LightManager (void)
{
}

//-------------------------------------------------------------------

void LightManager::load (Iff& iff)
{
	//-- clear light list
	lightDataList.clear ();

	//-- add data
	iff.enterForm (TAG (L,G,T,S));
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			{
				iff.enterForm (TAG_0000);
				{
					LightManager::LightData data;

					int n = iff.getNumberOfBlocksLeft ();
					int i;
					for (i = 0; i < n; i++)
					{
						iff.enterChunk (TAG (L,G,H,T));
						{
							iff.read_string (data.name, 100);
							data.color         = iff.read_floatVectorArgb ();
							data.disabled      = iff.read_int32 () != 0;
							data.phi           = iff.read_float ();
							data.position      = iff.read_floatVector ();
							data.theta         = iff.read_float ();
							data.type          = iff.read_int32 ();

							lightDataList.add (data);
						}
						iff.exitChunk ();
					}
				}
				iff.exitForm ();
			}
			break;

		case TAG_0001:
			{
				iff.enterForm (TAG_0001);
				{
					LightManager::LightData data;

					int n = iff.getNumberOfBlocksLeft ();
					int i;
					for (i = 0; i < n; i++)
					{
						iff.enterChunk (TAG (L,G,H,T));
						{
							iff.read_string (data.name, 100);
							data.color         = iff.read_floatVectorArgb ();
							data.specularColor = iff.read_floatVectorArgb ();
							data.disabled      = iff.read_int32 () != 0;
							data.phi           = iff.read_float ();
							data.position      = iff.read_floatVector ();
							data.theta         = iff.read_float ();
							data.type          = iff.read_int32 ();

							lightDataList.add (data);
						}
						iff.exitChunk ();
					}
				}
				iff.exitForm ();
			}
			break;

		case TAG_0002:
			{
				iff.enterForm (TAG_0002);
				{
					LightManager::LightData data;

					int n = iff.getNumberOfBlocksLeft ();
					int i;
					for (i = 0; i < n; i++)
					{
						iff.enterChunk (TAG (L,G,H,T));
						{
							iff.read_string (data.name, 100);
							data.color         = iff.read_floatVectorArgb ();
							data.specularColor = iff.read_floatVectorArgb ();
							data.disabled      = iff.read_int32 () != 0;
							data.phi           = iff.read_float ();
							data.position      = iff.read_floatVector ();
							data.theta         = iff.read_float ();
							data.type          = iff.read_int32 ();
							data.constantAttenuation  = iff.read_float ();
							data.linearAttenuation    = iff.read_float ();
							data.quadraticAttenuation = iff.read_float ();
							data.range                = iff.read_float ();
							data.spotTheta            = iff.read_float ();
							data.spotPhi              = iff.read_float ();
							data.spotFalloff          = iff.read_float ();

							lightDataList.add (data);
						}
						iff.exitChunk ();
					}
				}
				iff.exitForm ();
			}
			break;

		case TAG_0003:
			{
				iff.enterForm (TAG_0003);
				{
					LightManager::LightData data;

					int n = iff.getNumberOfBlocksLeft ();
					int i;
					for (i = 0; i < n; i++)
					{
						iff.enterChunk (TAG (L,G,H,T));
						{
							iff.read_string (data.name, 100);
							data.color         = iff.read_floatVectorArgb ();
							data.specularColor = iff.read_floatVectorArgb ();
							data.disabled      = iff.read_bool8 ();
							data.dot3          = iff.read_bool8 ();
							data.shadowSource  = iff.read_bool8 ();
							data.phi           = iff.read_float ();
							data.position      = iff.read_floatVector ();
							data.theta         = iff.read_float ();
							data.type          = iff.read_int32 ();
							data.constantAttenuation  = iff.read_float ();
							data.linearAttenuation    = iff.read_float ();
							data.quadraticAttenuation = iff.read_float ();
							data.range                = iff.read_float ();
							data.spotTheta            = iff.read_float ();
							data.spotPhi              = iff.read_float ();
							data.spotFalloff          = iff.read_float ();

							lightDataList.add (data);
						}
						iff.exitChunk ();
					}
				}
				iff.exitForm ();
			}
			break;

		}
	}
	iff.exitForm ();
}

//-------------------------------------------------------------------

void LightManager::save (Iff& iff) const
{
	iff.insertForm (TAG (L,G,T,S));
	{
		iff.insertForm (TAG_0003);
		{
			LightData data;

			int i;
			for (i = 0; i < lightDataList.getNumberOfElements (); i++)
			{
				data = lightDataList [i];

				iff.insertChunk (TAG (L,G,H,T));
				{
					iff.insertChunkString (data.name);
					iff.insertChunkFloatVectorArgb (data.color);
					iff.insertChunkFloatVectorArgb (data.specularColor);
					iff.insertChunkData (data.disabled ? static_cast<int8> (1) : static_cast<int8> (0));
					iff.insertChunkData (data.dot3 ? static_cast<int8> (1) : static_cast<int8> (0));
					iff.insertChunkData (data.shadowSource ? static_cast<int8> (1) : static_cast<int8> (0));
					iff.insertChunkData (data.phi);
					iff.insertChunkFloatVector (data.position);
					iff.insertChunkData (data.theta);
					iff.insertChunkData (static_cast<int32> (data.type));
					iff.insertChunkData (data.constantAttenuation);
					iff.insertChunkData (data.linearAttenuation);
					iff.insertChunkData (data.quadraticAttenuation);
					iff.insertChunkData (data.range);
					iff.insertChunkData (data.spotTheta);
					iff.insertChunkData (data.spotPhi);
					iff.insertChunkData (data.spotFalloff);
				}
				iff.exitChunk ();
			}
		}
		iff.exitForm ();
	}
	iff.exitForm ();
}

//-------------------------------------------------------------------

void LightManager::load (const char* filename)
{
	Iff iff;
	if (iff.open (filename, true))
		load (iff);
	else
		WARNING (true, ("LightManager::load - [%s] not found", filename));
}

//-------------------------------------------------------------------

void LightManager::save (const char* filename) const
{
	Iff iff (10000);

	save (iff);

	if(!iff.write (filename))
		WARNING (true, ("LightManager::save - [%s] could not be written", filename));
}

//-------------------------------------------------------------------

int LightManager::getNumberOfLights (void) const
{
	return lightDataList.getNumberOfElements ();
}

//-------------------------------------------------------------------

const LightManager::LightData& LightManager::getLight (int index) const
{
	return lightDataList [index];
}

//-------------------------------------------------------------------

LightManager::LightData& LightManager::getLight (int index)
{
	return lightDataList [index];
}

//-------------------------------------------------------------------

void LightManager::setLight (int index, const LightManager::LightData& newLight)
{
	lightDataList [index] = newLight;
}

//-------------------------------------------------------------------

void LightManager::addLight (const LightManager::LightData& newLight)
{
	lightDataList.add (newLight);
}

//-------------------------------------------------------------------

Light* LightManager::createLight (int i) const
{
	Light* light = 0;

	if (!lightDataList [i].disabled)
	{
		switch (lightDataList [i].type)
		{
		case Light::T_ambient:
		case Light::T_parallel:
		case Light::T_point:
		case Light::T_spot:
			light = new Light (static_cast<Light::Type> (lightDataList [i].type), lightDataList [i].color);
			break;

		case Light::T_OBSOLETE_parallelPoint:
			WARNING(true, ("parallel point lights no longer supported\n"));
			break;

		default:
			break;
		}

		if (light)
		{
			light->setDebugName (lightDataList [i].name);
			light->yaw_o (lightDataList [i].theta);
			light->pitch_o (lightDataList [i].phi);
			light->setDiffuseColor (lightDataList [i].color);
			light->setSpecularColor (lightDataList [i].specularColor);
			light->setPosition_p (lightDataList [i].position);
			light->setConstantAttenuation (lightDataList [i].constantAttenuation);
			light->setLinearAttenuation (lightDataList [i].linearAttenuation);
			light->setQuadraticAttenuation (lightDataList [i].quadraticAttenuation);
			light->setRange (lightDataList [i].range);
			light->setSpotFallOff (lightDataList [i].spotFalloff);
			light->setSpotPhi (lightDataList [i].spotPhi);
			light->setSpotTheta (lightDataList [i].spotTheta);
		}
	}

	return light;
}

//-------------------------------------------------------------------

void LightManager::setLights (const ArrayList<LightManager::LightData>& newLightList)
{
	lightDataList = newLightList;
}

//-------------------------------------------------------------------

const ArrayList<LightManager::LightData>& LightManager::getLights (void) const
{
	return lightDataList;
}

//-------------------------------------------------------------------

void LightManager::removeLight (int index)
{
	lightDataList.removeIndexAndCompactList (index);
}

//-------------------------------------------------------------------

