//
// SpriteAppearanceTemplate.cpp
// asommers 10-13-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "clientObject/FirstClientObject.h"
#include "clientObject/SpriteAppearanceTemplate.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/SpriteAppearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/FileName.h"

// ----------------------------------------------------------------------

bool SpriteAppearanceTemplate::installed;

//-------------------------------------------------------------------

const Tag TAG_QUAD = TAG (Q,U,A,D);
const Tag TAG_SPRT = TAG (S,P,R,T);

//-------------------------------------------------------------------

void SpriteAppearanceTemplate::install (void)
{
	DEBUG_FATAL (installed, ("SpriteAppearanceTemplate already installed"));
	installed = true;

	//-- add binding 
	AppearanceTemplateList::assignBinding (TAG_QUAD, create);
	AppearanceTemplateList::assignBinding (TAG_SPRT, create);

	ExitChain::add (remove, "SpriteAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

void SpriteAppearanceTemplate::remove(void)
{
	DEBUG_FATAL (!installed, ("SpriteAppearanceTemplate not installed"));
	installed = false;

	//-- remove binding
	AppearanceTemplateList::removeBinding (TAG_QUAD);
	AppearanceTemplateList::removeBinding (TAG_SPRT);
}

// ----------------------------------------------------------------------

AppearanceTemplate* SpriteAppearanceTemplate::create (const char* filename, Iff* iff)
{
	return new SpriteAppearanceTemplate (filename, iff);
}

// ----------------------------------------------------------------------

SpriteAppearanceTemplate::SpriteAppearanceTemplate (const char* filename, Iff* iff) :
	AppearanceTemplate (filename),
	sphere (),
	color (),
	shaderTemplate (0)
{
	load (iff);
}

//-------------------------------------------------------------------

SpriteAppearanceTemplate::~SpriteAppearanceTemplate (void)
{
	shaderTemplate->release();
}

//-------------------------------------------------------------------

Appearance* SpriteAppearanceTemplate::createAppearance () const
{
	return new SpriteAppearance (this, sphere.getCenter(), sphere.getRadius(), shaderTemplate->fetchModifiableShader (), color);
}

//-------------------------------------------------------------------

void SpriteAppearanceTemplate::load (Iff* iff)
{
	NOT_NULL (iff);

	if (iff->getCurrentName () == TAG_QUAD)
		iff->enterForm (TAG_QUAD);
	else
		iff->enterForm (TAG_SPRT);

		switch (iff->getCurrentName())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			{
				FATAL (true, ("SpriteAppearanceTemplate::load - unsupported format"));
			}
		}

	iff->exitForm ();

	//-- create extent
	BoxExtent* internalExtent = new BoxExtent ();

	const Vector &center = sphere.getCenter();
	const real radius = sphere.getRadius();
	Vector vMin = CONST_REAL (0.5) * Vector (center.x - radius, center.y - radius, center.z - radius);
	Vector vMax = CONST_REAL (0.5) * Vector (center.x + radius, center.y + radius, center.z + radius);

	internalExtent->setMin (vMin);
	internalExtent->setMax (vMax);
	internalExtent->calculateCenterAndRadius ();

	//-- this gets handed off to the base appearanceTemplate
	setExtent (ExtentList::fetch (internalExtent));
}

//-------------------------------------------------------------------

void SpriteAppearanceTemplate::load_0000 (Iff* iff)
{
	NOT_NULL (iff);

	iff->enterChunk (TAG_0000);

		sphere.setCenter (iff->read_floatVector ());
		sphere.setRadius (iff->read_float ());
		color  = iff->read_floatVectorArgb ();

		char nameBuffer [100];
		iff->read_string (nameBuffer, 100);

		shaderTemplate = ShaderTemplateList::fetch (FileName (FileName::P_shader, nameBuffer));

	iff->exitChunk (TAG_0000);
}

//-------------------------------------------------------------------

//const Sphere& SpriteAppearanceTemplate::getSphere(void) const
//{
//	return sphere;
//}

//-------------------------------------------------------------------

