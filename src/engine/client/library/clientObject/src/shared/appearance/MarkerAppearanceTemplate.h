// ======================================================================
//
// MarkerAppearanceTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MarkerAppearanceTemplate_H
#define INCLUDED_MarkerAppearanceTemplate_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/AppearanceTemplate.h"

class Iff;
class Shader;
class ShaderTemplate;
class MemoryBlockManager;

// ======================================================================

class MarkerAppearanceTemplate: public AppearanceTemplate
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	enum AnimationMode
	{
		AM_none,
		AM_timeBasedWrap,
		AM_timeBasedPingPong,

		AM_COUNT
	};

public:

	static void install();

public:

	virtual ~MarkerAppearanceTemplate();

	virtual Appearance *createAppearance() const;

	float               getParentRadiusMultiplier() const;
	float               getParentRadiusAddition() const;

	const Shader       *fetchShader() const;

	int                 getTextureStartIndex() const;
	int                 getTextureStopIndex() const;
	AnimationMode       getTextureAnimationMode() const;
	float               getTexturesPerSecond() const;

	void                getUvCoordinates(int textureIndex, float &left, float &top, float &right, float &bottom) const;

private:

	static void remove();
	static AppearanceTemplate *create(const char *name, Iff *iff);

private:

	MarkerAppearanceTemplate(const char *name, Iff &iff);

	void                load_0000(Iff &iff);

	// Disabled.
	MarkerAppearanceTemplate();
	MarkerAppearanceTemplate(const MarkerAppearanceTemplate&);
	MarkerAppearanceTemplate &operator =(const MarkerAppearanceTemplate&);

private:

	float                 m_parentRadiusMultiplier;
	float                 m_parentRadiusAddition;

	const ShaderTemplate *m_shaderTemplate;

	int                   m_xTextureCount;
	int                   m_yTextureCount;
	float                 m_uTextureInterval;
	float                 m_vTextureInterval;

	int                   m_textureStartIndex;
	int                   m_textureStopIndex;
	AnimationMode         m_textureAnimationMode;
	float                 m_texturesPerSecond;

};

// ======================================================================

#endif
