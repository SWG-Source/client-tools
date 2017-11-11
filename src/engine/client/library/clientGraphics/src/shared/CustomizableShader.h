// ======================================================================
//
// CustomizableShader.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CustomizableShader_H
#define INCLUDED_CustomizableShader_H

// ======================================================================

#include "clientGraphics/Shader.h"

class CustomizableShaderTemplate;
class PackedArgb;

// ======================================================================

class CustomizableShader: public Shader
{
friend class CustomizableShaderTemplate;

public:

	typedef stdvector<PackedArgb>::fwd          PackedArgbVector;
	typedef stdvector<std::string const*>::fwd  StringVector;

public:

	virtual Shader             *convertToModifiableShader() const;

	virtual bool                obeysCustomizationData() const;
	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;

	virtual float               alter(float time) const;

	virtual const StaticShader &prepareToView() const;

	virtual bool                usesVertexShader() const;

	virtual const StaticShader *getStaticShader() const;
	virtual StaticShader       *getStaticShader();

	Shader                     *getBaseShader();

	bool                        getHueInfo(StringVector &variableNames, PackedArgbVector &colors) const;

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	static void handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context);

private:

	explicit CustomizableShader(const CustomizableShaderTemplate &customizableShaderTemplate);
	virtual ~CustomizableShader();

	const CustomizableShaderTemplate &getCustomizableShaderTemplate() const;

	void                              handleCustomizationModification(const CustomizationData &customizationData);

	// disabled
	CustomizableShader();
	CustomizableShader(const CustomizableShader&);
	CustomizableShader &operator =(const CustomizableShader&);

private:

	static const std::string  cms_privateVariablePathPrefix;
	static const std::string  cms_sharedVariablePathPrefix;

private:

	Shader    *const   m_baseShader;

	CustomizationData *m_customizationData;
	IntVector *const   m_intValues;
	mutable bool       m_isModified;

};

// ======================================================================

inline Shader *CustomizableShader::getBaseShader()
{
	return m_baseShader;
}

// ======================================================================

#endif
