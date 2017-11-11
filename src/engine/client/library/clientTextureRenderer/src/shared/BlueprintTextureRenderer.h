// ======================================================================
//
// BlueprintTextureRenderer.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_BlueprintTextureRenderer_H
#define INCLUDED_BlueprintTextureRenderer_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "clientTextureRenderer/TextureRenderer.h"

class BlueprintTextureRendererTemplate;
class BlueprintSharedTextureRenderer;

// ======================================================================

class BlueprintTextureRenderer: public TextureRenderer
{
friend class BlueprintTextureRendererTemplate;

public:

	virtual bool           render();
	virtual void           setCustomizationData(CustomizationData *customizationData);
	virtual void           addCustomizationVariables(CustomizationData &customizationData) const;
	virtual const Texture *fetchTexture() const;
	virtual bool           isTextureReady() const;

private:

	typedef stdvector<int>::fwd      IntVector;

private:

	static void handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context);

private:

	explicit  BlueprintTextureRenderer(const BlueprintTextureRendererTemplate &BlueprintTextureRendererTemplate);
	virtual  ~BlueprintTextureRenderer();

	const BlueprintTextureRendererTemplate &getBlueprintTextureRendererTemplate() const;
	void                                    handleCustomizationModification(const CustomizationData &customizationData);


	// disabled
	BlueprintTextureRenderer();
	BlueprintTextureRenderer(const BlueprintTextureRenderer&);
	BlueprintTextureRenderer &operator =(const BlueprintTextureRenderer&);

private:

	static const std::string  cms_privateVariableNamePrefix;
	static const std::string  cms_sharedVariableNamePrefix;

private:

	CustomizationData                            *m_customizationData;
	IntVector *const                              m_intValues;

	mutable bool                                  m_isModified;
	mutable const BlueprintSharedTextureRenderer *m_sharedTextureRenderer;
	mutable const BlueprintSharedTextureRenderer *m_previousSharedTextureRenderer;

};

// ======================================================================

#endif
