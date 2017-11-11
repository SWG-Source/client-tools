//===================================================================
//
// ClientGlobalWaterManager2.h
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientGlobalWaterManager2_H
#define INCLUDED_ClientGlobalWaterManager2_H

//===================================================================

#include "clientTerrain/WaterManager.h"

class Appearance;
class BoxExtent;
class Rectangle2d;
class Shader;
class Vector;

//===================================================================

class ClientGlobalWaterManager2 : public WaterManager
{
public:

	static void install ();

public:

	ClientGlobalWaterManager2(const Appearance& appearance, float mapWidthInMeters, const char* shaderTemplateName, float shaderSize, float height, Rectangle2d const * clipRegion);
	virtual ~ClientGlobalWaterManager2 ();

	virtual void alter (float elapsedTime);
	virtual void draw () const;

private:

	static void remove ();

private:

	ClientGlobalWaterManager2 ();
	ClientGlobalWaterManager2 (const ClientGlobalWaterManager2& rhs);
	ClientGlobalWaterManager2& operator= (const ClientGlobalWaterManager2& rhs);

private:

	const Appearance&           m_appearance;

	class LocalShaderPrimitive;
	class StaticShaderPrimitive;
	class DynamicShaderPrimitive;
	LocalShaderPrimitive *m_localShaderPrimitive;
};

//===================================================================

#endif
