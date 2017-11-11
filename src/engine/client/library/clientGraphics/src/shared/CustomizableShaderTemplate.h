// ======================================================================
//
// CustomizableShaderTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CustomizableShaderTemplate_H
#define INCLUDED_CustomizableShaderTemplate_H

// ======================================================================

#include "clientGraphics/ShaderTemplate.h"

class CrcString;
class Iff;
class MemoryBlockManager;
class PackedArgb;
class StaticShader;
class Texture;

// ======================================================================

class CustomizableShaderTemplate: public ShaderTemplate
{
public:

	typedef stdvector<int>::fwd                 IntVector;
	typedef stdvector<PackedArgb>::fwd          PackedArgbVector;
	typedef stdvector<std::string const*>::fwd  StringVector;

	// public interface should ignore these; it is an implementation artifact
	// that they are public.
	class MaterialIntOperation;
	class AmbientMaterialIntOperation;
	class DiffuseMaterialIntOperation;
	class EmissiveMaterialIntOperation;

	friend class AmbientMaterialIntOperation;
	friend class DiffuseMaterialIntOperation;
	friend class EmissiveMaterialIntOperation;

public:

	static void  install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

public:

	virtual int                    getCustomizationVariableCount() const;
	virtual const std::string     &getCustomizationVariableName(int index) const;
	bool                           isCustomizationVariablePrivate(int index) const;
	virtual CustomizationVariable *createCustomizationVariable(int index) const;

	virtual bool                   isOpaqueSolid() const;
	virtual bool                   isCollidable() const;
	virtual bool                   castsShadows() const;
	virtual const Shader          *fetchShader() const;
	virtual Shader                *fetchModifiableShader() const;

	int                            getIntVariableCount() const;
	const std::string             &getIntVariableName(int index) const;
	bool                           isIntVariablePrivate(int index) const;

	bool                           applyShaderSettings(const IntVector &intValues, StaticShader &shader) const;

	const ShaderTemplate          &getBaseShaderTemplate() const;

	const Texture                 *fetchTexture(int templateTextureIndex) const;

	bool                           getHueInfo(IntVector const &intValues, StringVector &variableNames, PackedArgbVector &colors) const;

private:

	class CustomizedMaterial;
	friend class CustomizedMaterial;

	class TextureIntOperation;
	friend class TextureIntOperation;

	class TextureFactorIntOperation;
	friend class TextureFactorIntOperation;

	class IntVariableFactory;
	class RangedIntVariableFactory;
	class PaletteColorVariableFactory;

	class CachedTexture;

	typedef stdvector<CachedTexture*>::fwd              CachedTextureVector;
	typedef stdvector<CustomizedMaterial*>::fwd         CustomizedMaterialVector;
	typedef stdvector<TextureFactorIntOperation*>::fwd  TextureFactorIntOperationVector;
	typedef stdvector<TextureIntOperation*>::fwd        TextureIntOperationVector;
	typedef stdvector<IntVariableFactory*>::fwd         IntVariableFactoryVector;

private:

	static void            remove();
	static ShaderTemplate *create(const CrcString &name, Iff &iff);

private:

	CustomizableShaderTemplate(const CrcString &name, Iff &iff);
	virtual ~CustomizableShaderTemplate();

	void     load_0000(Iff &iff);
	void     load_0001(Iff &iff);

	int      submitRangedIntVariable(const std::string &variableName, bool variableIsPrivate, int minValueInclusive, int defaultValue, int maxValueExclusive);
	int      submitPaletteColorVariable(const std::string &variableName, bool variableIsPrivate, const char *palettePathName, int defaultValue);

	bool     findIntVariable(const std::string &variableName, bool variableIsPrivate, int &index, IntVariableFactory *&variableFactory);

	// disabled
	CustomizableShaderTemplate();
	CustomizableShaderTemplate(const CustomizableShaderTemplate&);
	CustomizableShaderTemplate &operator =(const CustomizableShaderTemplate&);

private:

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;

	static bool                              ms_debugLogChanges;

private:

	const ShaderTemplate            *m_baseShaderTemplate;

	IntVariableFactoryVector *const  m_intVariableFactoryVector;

	CustomizedMaterialVector        *m_customizedMaterialVector;
	TextureIntOperationVector       *m_textureIntOperationVector;
	TextureFactorIntOperationVector *m_textureFactorIntOperationVector;

	CachedTextureVector             *m_cachedTextureVector;
};

// ======================================================================

inline const ShaderTemplate &CustomizableShaderTemplate::getBaseShaderTemplate() const
{
	return *m_baseShaderTemplate;
}

// ======================================================================

#endif
