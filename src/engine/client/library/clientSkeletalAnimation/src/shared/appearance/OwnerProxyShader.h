// ======================================================================
//
// OwnerProxyShader.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_OwnerProxyShader_H
#define INCLUDED_OwnerProxyShader_H

// ======================================================================

#include "clientGraphics/Shader.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class MemoryBlockManager;
class OwnerProxyShaderTemplate;

// ======================================================================

class OwnerProxyShader: public Shader
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install(bool stitchedSkinInheritsFromSelf = false);

	static void setEnabled(bool enabled);

public:

	explicit OwnerProxyShader(const OwnerProxyShaderTemplate &shaderTemplate);

	virtual Shader               *convertToModifiableShader() const;

	virtual bool                  obeysCustomizationData() const;
	virtual void                  setCustomizationData(CustomizationData *customizationData);
	virtual void                  addCustomizationVariables(CustomizationData &customizationData) const;

	virtual const StaticShader   &prepareToView() const;
	virtual bool                  usesVertexShader() const;
	virtual const StaticShader   *getStaticShader() const;
	virtual StaticShader         *getStaticShader();

	Shader const                 *getRealShader() const;

private:

	static void remove();

private:

	virtual                        ~OwnerProxyShader();

	void                            assignShader(CustomizationData *customizationData);
	Shader                         *fetchNoRenderShader() const;

	const OwnerProxyShaderTemplate &getProxyShaderTemplate() const;
	const std::string               buildOwnerShaderTemplateName(const std::string &appearanceTemplateName) const;

	// Disabled.
	OwnerProxyShader();
	OwnerProxyShader(const OwnerProxyShader&);
	OwnerProxyShader &operator =(const OwnerProxyShader&);

private:

	static const std::string  cms_sharedOwnerDirectoryName;
	static const std::string  cms_shaderTemplateDirectory;
	static const std::string  cms_shaderTemplateExtension;
	static const char *const  cms_directorySeparatorCharacters;
	static const char         cms_extensionCharacter;
	
	static       bool         cms_stitchedSkinInheritsFromSelf;

private:

	Shader *m_shader;


};

// ======================================================================

#endif
