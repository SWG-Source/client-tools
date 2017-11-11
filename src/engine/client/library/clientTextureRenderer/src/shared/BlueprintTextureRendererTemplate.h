// ======================================================================
//
// BlueprintTextureRendererTemplate.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BlueprintTextureRendererTemplate_H
#define INCLUDED_BlueprintTextureRendererTemplate_H

// ======================================================================

#include "clientTextureRenderer/TextureRendererTemplate.h"

class BlueprintSharedTextureRenderer;
class BlueprintTextureRenderer;
class CustomizationVariable;
class Iff;
class LessPointerComparator;
class Shader;
class ShaderTemplate;
class StaticIndexBuffer;
class StaticVertexBuffer;
class Texture;
class TextureRenderer;

// ======================================================================

class BlueprintTextureRendererTemplate: public TextureRendererTemplate
{
public:

	typedef stdvector<int>::fwd                    IntVector;
	typedef stdvector<const ShaderTemplate*>::fwd  ShaderTemplateContainer;
	typedef stdvector<Shader*>::fwd                ShaderVector;

	//-- ignore these, declared as part of public interface solely to allow derivation of hidden classes within implementation
	class CameraSetup;
	class PrepareCommand;
	class RenderCommand;

public:

	static void install();
	static void remove();

public:

	virtual TextureRenderer       *createTextureRenderer() const;
	virtual void                   addCustomizationVariables(CustomizationData &customizationData) const;

	virtual int                    getCustomizationVariableCount() const;
	virtual const std::string     &getCustomizationVariableName(int index) const;
	virtual bool                   isCustomizationVariablePrivate(int index) const;

	virtual CustomizationVariable *createCustomizationVariable(int index) const;

	const StaticVertexBuffer      &getVertexBuffer(int index) const;
	const StaticIndexBuffer       &getIndexBuffer(int index) const;
	const ShaderTemplateContainer &getShaderTemplates() const;

	int                            getTextureCount() const;
	const Texture                 *fetchTexture(int index) const;

	void                           prepareShaders(const IntVector &intValues, ShaderVector &shaders) const;
	bool                           render(Texture *destinationTexture, const IntVector &intValues, ShaderVector &shaders) const;

	// internal use functions
	int                            submitRangedIntVariable(const std::string &variableName, int minValueInclusive, int maxValueExclusive, bool variableIsPrivate = true);
	int                            submitPaletteColorVariable(const std::string &variableName, const char *palettePathName, bool variableIsPrivate = true);

	const BlueprintSharedTextureRenderer *fetchSharedTextureRenderer(const IntVector &intValues) const;
	void                                  notifySharedTextureDestruction(const BlueprintSharedTextureRenderer *sharedTextureRenderer) const;

private:

	class VariableFactory;
	class BasicRangedIntVariableFactory;
	class PaletteColorVariableFactory;

	typedef stdvector<StaticIndexBuffer*>::fwd                        IndexBufferContainer;
	typedef stdvector<PrepareCommand*>::fwd                           PrepareCommandContainer;
	typedef stdvector<RenderCommand*>::fwd                            RenderCommandContainer;
	typedef stdvector<std::string>::fwd                               StringContainer;
	typedef stdvector<const Texture*>::fwd                            TextureContainer;
	typedef stdvector<VariableFactory*>::fwd                          VariableFactoryVector;
	typedef stdvector<StaticVertexBuffer*>::fwd                       VertexBufferContainer;

	typedef stdmap<const IntVector*, BlueprintSharedTextureRenderer*, LessPointerComparator>::fwd  SharedTextureRendererMap;

private:

	static TextureRendererTemplate *create(Iff *iff, const char *name);

private:

	BlueprintTextureRendererTemplate(Iff *iff, const char *name);
	~BlueprintTextureRendererTemplate();

	void  load_0001(Iff *iff);
	void  load_0002(Iff *iff);

	bool  findVariableFactory(const std::string &variableName, bool variableIsPrivate, VariableFactory *&variableFactory, int &factoryIndex);

	// disabled
	BlueprintTextureRendererTemplate();
	BlueprintTextureRendererTemplate(const BlueprintTextureRendererTemplate&);
	BlueprintTextureRendererTemplate &operator =(const BlueprintTextureRendererTemplate&);

private:

	CameraSetup                         *m_cameraSetup;
	PrepareCommandContainer             *m_prepareCommands;
	RenderCommandContainer              *m_renderCommands;
	ShaderTemplateContainer             *m_shaderTemplates;
	StringContainer                     *m_textureNames;
	TextureContainer                    *m_textures;
	VertexBufferContainer               *m_vertexBuffers;
	IndexBufferContainer                *m_indexBuffers;

	VariableFactoryVector               *m_variableFactories;

	mutable SharedTextureRendererMap    *m_sharedTextureRendererMap;
};

// ======================================================================

inline const BlueprintTextureRendererTemplate::ShaderTemplateContainer &BlueprintTextureRendererTemplate::getShaderTemplates() const
{
	return *m_shaderTemplates;
}

// ======================================================================

#endif
