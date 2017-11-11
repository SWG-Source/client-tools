// ======================================================================
//
// BeamAppearanceTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_BeamAppearanceTemplate_H
#define INCLUDED_BeamAppearanceTemplate_H

// ======================================================================

#include "sharedObject/AppearanceTemplate.h"
#include "sharedMath/VectorArgb.h"

class Appearance;
class ShaderTemplate;

// ======================================================================

class BeamAppearanceTemplate : public AppearanceTemplate
{
public:

	static void install();
	static AppearanceTemplate * create(char const * fileName, Iff * iff);

public:

	BeamAppearanceTemplate(char const * fileName, Iff * iff);
	virtual ~BeamAppearanceTemplate();

	virtual Appearance * createAppearance() const;

private:

	BeamAppearanceTemplate();
	BeamAppearanceTemplate(BeamAppearanceTemplate const &);
	BeamAppearanceTemplate & operator=(BeamAppearanceTemplate const &);

	void load(Iff * iff); 
	void load_0000(Iff * iff);

private:

	float m_length;
	float m_width;
	VectorArgb m_color;
	ShaderTemplate const * m_shaderTemplate;
};

// ======================================================================

#endif
