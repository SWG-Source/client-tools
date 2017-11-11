//
// SpriteAppearanceTemplate.h
// asommers 10-13-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_SpriteAppearanceTemplate_H
#define INCLUDED_SpriteAppearanceTemplate_H

//-------------------------------------------------------------------

#include "sharedObject/AppearanceTemplate.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

class Appearance;
class ShaderTemplate;

//-------------------------------------------------------------------

class SpriteAppearanceTemplate : public AppearanceTemplate
{
private:

	static bool installed;

private:

	Sphere                sphere;
	VectorArgb            color;
	const ShaderTemplate* shaderTemplate;

private:

	static void remove ();

private:

	SpriteAppearanceTemplate (void);
	SpriteAppearanceTemplate (const SpriteAppearanceTemplate&);
	SpriteAppearanceTemplate& operator= (const SpriteAppearanceTemplate&);

	void load (Iff* iff);  

private:

	void load_0000 (Iff* iff);

public:

	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	SpriteAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~SpriteAppearanceTemplate (void);

	//virtual const Sphere&     getSphere (void) const;

	virtual Appearance*       createAppearance () const;

	static void               install (void);
};

//-------------------------------------------------------------------

#endif
