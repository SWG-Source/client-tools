//======================================================================
//
// NebulaManagerClient.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaManagerClient_H
#define INCLUDED_NebulaManagerClient_H

//======================================================================

class ClientObject;
class EnvironmentalHitData;
class NebulaLightningData;
class NebulaLightningHitData;
class VectorArgb;
class Vector;
class VectorArgb;

//----------------------------------------------------------------------

class NebulaManagerClient
{
public:

	static void install();

	static void render();
	static void loadScene(std::string const & sceneId);
	static void clear();
	static void update(float deltaTimeSecs);

	static VectorArgb const & getCurrentNebulaCameraHue();

	static void handlePlayerEnter(int id);
	static void handlePlayerExit(int id);

	static void enqueueLightning(NebulaLightningData const & nebulaLightningData);

	static void handleServerHit(ClientObject const & victim, NebulaLightningHitData const & nebulaLightningHitData);
	static void handleServerEnvironmentalDamage(ClientObject const & victim, EnvironmentalHitData const & environmentalHitData);

	static void updateClientNebulaData(Vector const & center, float rangeSquared);

	static void generateShaderPrimitives();

	//----------------------------------------------------------------------

	class Config
	{
	public:

		static void setOrientedPercent(float f);
		static void setNumShells(int i);
		static void setRegenerate(bool b);
		static void setRandomSeedOffset(int i);
		static void setColorVariance(VectorArgb const & v);
		static void setQuadGenerationRadius(float f);
		static void setRenderNear(bool b);
		static void setRenderFar(bool b);

		static float getOrientedPercent();
		static int getNumShells();
		static bool getRegenerate();
		static int getRandomSeedOffset();
		static VectorArgb const & getColorVariance();
		static float getQuadGenerationRadius();
		static bool getRenderNear();
		static bool getRenderFar();


	private:
		static float ms_orientedPercent;
		static int ms_numShells;
		static bool ms_regenerate;
		static int ms_randomSeedOffset;
		static VectorArgb ms_colorVariance;
		static float ms_quadGenerationRadius;
		static bool ms_renderNear;
		static bool ms_renderFar;
	};

private:

	static void remove();
	static void generateTestLightningEvents(float elapsedTime);
	static void handleEnqueuedLightningEvents();
	static void updateCameraHue();

	static VectorArgb ms_currentNebulaCameraHue;

	static NebulaLightningData const * findNebulaLightningData(uint16 lightningId);

};

//----------------------------------------------------------------------

inline VectorArgb const & NebulaManagerClient::getCurrentNebulaCameraHue()
{
	return ms_currentNebulaCameraHue;
}

//======================================================================

#endif
