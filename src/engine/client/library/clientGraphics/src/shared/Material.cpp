// ======================================================================
//
// Material.cpp
// copyright 2000 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Material.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Tag.h"

// ======================================================================

const Tag TAG_MATL = TAG(M,A,T,L);

// ======================================================================

const Material Material::defaultMaterial(
	VectorArgb(CONST_REAL(1), CONST_REAL(1), CONST_REAL(1), CONST_REAL(1)),
	VectorArgb(CONST_REAL(1), CONST_REAL(1), CONST_REAL(1), CONST_REAL(1)),
	VectorArgb(CONST_REAL(1), CONST_REAL(0), CONST_REAL(0), CONST_REAL(0)),
	VectorArgb(CONST_REAL(1), CONST_REAL(0), CONST_REAL(0), CONST_REAL(0)),
	CONST_REAL(0));

// ----------------------------------------------------------------------

Material::Material(void) :
	ambientColor(VectorArgb::solidWhite),
	diffuseColor(VectorArgb::solidWhite),
	emissiveColor(VectorArgb::solidBlack),
	specularColor(VectorArgb::solidBlack),
	specularPower(0)
{
}

// ----------------------------------------------------------------------
	
Material::Material(const VectorArgb &newAmbientColor, const VectorArgb &newDiffuseColor, const VectorArgb &newEmissiveColor, const VectorArgb &newSpecularColor, real newSpecularPower) :
	ambientColor(newAmbientColor),
	diffuseColor(newDiffuseColor),
	emissiveColor(newEmissiveColor),
	specularColor(newSpecularColor),
	specularPower(newSpecularPower)
{
}

// ----------------------------------------------------------------------
	
Material::~Material(void)
{
}

// ----------------------------------------------------------------------

Material::Material(Iff &iff)
{
	load(iff);
}

// ----------------------------------------------------------------------

void Material::load(Iff &iff)
{
	iff.enterChunk(TAG_MATL);
		ambientColor  = iff.read_floatVectorArgb();
		diffuseColor  = iff.read_floatVectorArgb();
		emissiveColor = iff.read_floatVectorArgb();
		specularColor = iff.read_floatVectorArgb();
		specularPower = iff.read_float();
	iff.exitChunk(TAG_MATL);
}

// ----------------------------------------------------------------------

void Material::write(Iff &iff) const
{
	iff.insertChunk(TAG_MATL);

		iff.insertChunkFloatVectorArgb(ambientColor);
		iff.insertChunkFloatVectorArgb(diffuseColor);
		iff.insertChunkFloatVectorArgb(emissiveColor);
		iff.insertChunkFloatVectorArgb(specularColor);
		iff.insertChunkData(static_cast<float>(specularPower));

	iff.exitChunk(TAG_MATL);
}

// ======================================================================
