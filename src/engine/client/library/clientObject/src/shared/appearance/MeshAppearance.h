// ======================================================================
//
// MeshAppearance.h
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MeshAppearance_H
#define INCLUDED_MeshAppearance_H

// ======================================================================

class MeshAppearanceTemplate;
class ShaderPrimitiveSet;

#include "sharedObject/Appearance.h"

// ======================================================================

class MeshAppearance: public Appearance
{
	friend class MeshAppearanceTemplate;

public:

	static void install ();
	static void setCollideAgainstAllGeometry (bool collideAgainstAllGeometry);
	static void setFadeInEnabled(bool enabled);

public:

	MeshAppearance(const MeshAppearanceTemplate *newAppearanceTemplate);
	virtual ~MeshAppearance();

	virtual bool                isLoaded() const;

	virtual void                addToWorld();
	virtual void                removeFromWorld();

	virtual void                setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	virtual float               alter(float time);
	virtual const Sphere       &getSphere() const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;

	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;

	virtual void                render() const;
	virtual void drawDebugShapes (DebugShapeRenderer* renderer) const;

	const ShaderPrimitiveSet   *getShaderPrimitiveSet() const;

	virtual void                setTexture(Tag tag, const Texture &texture);
	virtual void renderShadowBlob (const Vector& position_o, float radius) const;
	virtual void renderReticle (const Vector& position_o, float radius) const;
	virtual AxialBox const getTangibleExtent() const;

#ifdef _DEBUG
	virtual int  getPolygonCount () const;
	virtual void debugDump(std::string &result, int indentLevel) const;
#endif

private:

	// disable these routines
	MeshAppearance();
	MeshAppearance(const MeshAppearance &);
	MeshAppearance &operator =(const MeshAppearance &);

	const MeshAppearanceTemplate *getMeshAppearanceTemplate() const;
	void                          create();
	virtual DPVS::Object         *getDpvsObject() const;

protected:

	ShaderPrimitiveSet *m_shaderPrimitiveSet;
	DPVS::Object       *m_dpvsObject;

	bool                m_needsSetTexture;
	bool                m_obeyOpaqueAlpha;
	bool                m_obeyAlphaAlpha;
	Tag                 m_setTextureTag;
	const Texture*      m_setTextureTexture;
	float               m_opaqueAlpha;
	float               m_alphaAlpha;

	CustomizationData  *m_customizationData;

	bool mutable m_lastCouldRenderShadow;
	bool mutable m_lastCouldRenderReticles;
};

// ======================================================================

inline const ShaderPrimitiveSet *MeshAppearance::getShaderPrimitiveSet() const
{
	return m_shaderPrimitiveSet;
}

// ======================================================================

#endif
