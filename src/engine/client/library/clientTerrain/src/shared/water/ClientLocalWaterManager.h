//===================================================================
//
// ClientLocalWaterManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientLocalWaterManager_H
#define INCLUDED_ClientLocalWaterManager_H

//===================================================================

#include "clientTerrain/WaterManager.h"

class Appearance;
class Vector2d;
class Vector;

//===================================================================

class ClientLocalWaterManager : public WaterManager
{
public:

	explicit ClientLocalWaterManager (const Appearance& m_appearance);
	virtual ~ClientLocalWaterManager (void);

	virtual void alter (real time);
	virtual void draw () const;

	//-- used for arbitrary polygonal horizontal water tables
	void addWater (char const * debugName, const char* shaderTemplateName, real shaderSize, const stdvector<Vector2d>::fwd& pointList, real height, const Vector2d& direction, real velocity);

	//-- used for water tables created by the old river affector
	void addWater (char const * debugName, const char* shaderTemplateName, real shaderSize, const stdvector<Vector>::fwd& pointList, const Vector2d& direction, real velocity);

	void addRibbonEndCap (char const * debugName, const char* shaderTemplateName, real shaderSize, const stdvector<Vector2d>::fwd& pointList, real height, const Vector2d& direction, float velocity, const Vector& origin);
	
	//-- used for adding strips from ribbon affectors
	void addRibbonStrip (char const * debugName, const char* shaderTemplateName, real shaderSize, const stdvector<Vector>::fwd& pointList, const stdvector<Vector>::fwd& directionList, real velocity);
private:

	ClientLocalWaterManager (const ClientLocalWaterManager& rhs);
	ClientLocalWaterManager& operator= (const ClientLocalWaterManager& rhs);

private:

	friend class LocalShaderPrimitiveDefault_DrawFunctor;
	friend class LocalShaderPrimitiveRibbonStrip_DrawFunctor;

	class LocalShaderPrimitiveDefault;
	typedef stdvector<LocalShaderPrimitiveDefault*>::fwd LocalShaderPrimitiveDefaultList;

	class LocalShaderPrimitiveRibbonStrip;
	typedef stdvector<LocalShaderPrimitiveRibbonStrip*>::fwd LocalShaderPrimitiveRibbonStripList;

private:

	const Appearance&         m_appearance;
	LocalShaderPrimitiveDefaultList* m_localShaderPrimitiveDefaultList;
	LocalShaderPrimitiveRibbonStripList* m_localShaderPrimitiveRibbonStripList;
};

//===================================================================

#endif
