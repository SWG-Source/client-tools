// ======================================================================
//
// Material.h
// copyright 2000 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Material_H
#define INCLUDED_Material_H

// ======================================================================

class Iff;
#include "sharedMath/VectorArgb.h"

// ======================================================================

class Material
{
public:

	static const Material defaultMaterial;

public:

	DLLEXPORT Material();
	Material(const VectorArgb &newAmbientColor, const VectorArgb &newDiffuseColor, const VectorArgb &newEmissiveColor, const VectorArgb &newSpecularColor, real newSpecularPower);
	Material(Iff &iff);
	DLLEXPORT ~Material();

	void              load(Iff &iff);
	void              write(Iff &iff) const;

	void              setAmbientColor(const VectorArgb &newAmbientColor);
	const VectorArgb &getAmbientColor() const;

	void              setDiffuseColor(const VectorArgb &newDiffuseColor);
	const VectorArgb &getDiffuseColor() const;

	void              setSpecularColor(const VectorArgb &newSpecularColor);
	const VectorArgb &getSpecularColor() const;

	void              setEmissiveColor(const VectorArgb &newEmissiveColor);
	const VectorArgb &getEmissiveColor() const;

	void              setSpecularPower(real newSpecularPower);
	real              getSpecularPower() const;

private:

	VectorArgb   ambientColor;
	VectorArgb   diffuseColor;
	VectorArgb   emissiveColor;
	VectorArgb   specularColor;
	real         specularPower;
};

// ----------------------------------------------------------------------

inline const VectorArgb &Material::getAmbientColor() const
{
	return ambientColor;
}

// ----------------------------------------------------------------------

inline void Material::setAmbientColor(const VectorArgb &newAmbientColor)
{
	ambientColor = newAmbientColor;
}

// ----------------------------------------------------------------------

inline const VectorArgb &Material::getDiffuseColor() const
{
	return diffuseColor;
}

// ----------------------------------------------------------------------

inline void Material::setDiffuseColor(const VectorArgb &newDiffuseColor)
{
	diffuseColor = newDiffuseColor;
}

// ----------------------------------------------------------------------

inline const VectorArgb &Material::getSpecularColor() const
{
	return specularColor;
}

// ----------------------------------------------------------------------

inline void Material::setSpecularColor(const VectorArgb &newSpecularColor)
{
	specularColor = newSpecularColor;
}

// ----------------------------------------------------------------------

inline const VectorArgb &Material::getEmissiveColor() const
{
	return emissiveColor;
}

// ----------------------------------------------------------------------

inline void Material::setEmissiveColor(const VectorArgb &newEmissiveColor)
{
	emissiveColor = newEmissiveColor;
}

// ----------------------------------------------------------------------

inline real Material::getSpecularPower() const
{
	return specularPower;
}

// ----------------------------------------------------------------------

inline void Material::setSpecularPower(real newSpecularPower)
{
	specularPower = newSpecularPower;
}

// ----------------------------------------------------------------------

#endif

