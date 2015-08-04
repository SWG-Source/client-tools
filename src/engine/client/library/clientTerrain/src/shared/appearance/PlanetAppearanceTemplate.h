// ======================================================================
//
// PlanetAppearanceTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetAppearanceTemplate_H
#define INCLUDED_PlanetAppearanceTemplate_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedObject/AppearanceTemplate.h"

class Iff;
class Shader;
class Sphere;

// ======================================================================

class PlanetAppearanceTemplate : public AppearanceTemplate
{
public:

	static void install();

	static AppearanceTemplate * create(char const * filename, Iff * iff);

public:

	PlanetAppearanceTemplate(char const * filename, Iff * iff);
	virtual ~PlanetAppearanceTemplate();

	virtual Appearance * createAppearance() const;

	int getNumberOfLatitudeLines() const;
	int getNumberOfLongitudeLines() const;
	float getSurfaceRotationRate() const;
	Shader const * fetchSurfaceShader() const;
	float getSurfaceRadius() const;
	float getSurfaceUScaleMAIN() const;
	float getSurfaceVScaleMAIN() const;
	float getSurfaceUScaleDETA() const;
	float getSurfaceVScaleDETA() const;
	float getCloudRotationRate() const;
	Shader const * fetchCloudShader() const;
	float getCloudRadius() const;
	float getCloudUScaleMAIN() const;
	float getCloudVScaleMAIN() const;
	Shader const * fetchHaloShader() const;
	float getHaloScale() const;

	Sphere const getSphere() const;

private:

	static void remove();

private:

	static Tag const cms_planetAppearanceTemplateTag;

private:

	PlanetAppearanceTemplate();
	PlanetAppearanceTemplate(PlanetAppearanceTemplate const &);
	PlanetAppearanceTemplate & operator=(PlanetAppearanceTemplate const &);

	void load(Iff & iff);
	void load_0000(Iff & iff);

private:

	int m_numberOfLatitudeLines;
	int m_numberOfLongitudeLines;
	float m_surfaceRotationRate;
	Shader const * m_surfaceShader;
	float m_surfaceRadius;
	float m_surfaceUScaleMAIN;
	float m_surfaceVScaleMAIN;
	float m_surfaceUScaleDETA;
	float m_surfaceVScaleDETA;
	float m_cloudRotationRate;
	Shader const * m_cloudShader;
	float m_cloudRadius;
	float m_cloudUScaleMAIN;
	float m_cloudVScaleMAIN;
	Shader const * m_haloShader;
	float m_haloScale;
}; 

// ======================================================================

#endif
 
