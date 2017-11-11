// ======================================================================
//
// MeshAppearanceTemplate.h
// Portions copyright 1998 Bootprint Entertainment
// Portions Copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MeshAppearanceTemplate_H
#define INCLUDED_MeshAppearanceTemplate_H

// ======================================================================

#include "sharedObject/AppearanceTemplate.h"
#include "sharedMath/Sphere.h"

class Iff;
class IndexedTriangleList;
class MeshAppearance;
class ShaderPrimitiveSet;
class ShaderPrimitiveSetTemplate;
class ShadowVolume;

namespace DPVS
{
	class Model;
};

// ======================================================================

// Mesh appearance class
//
// This class provides a way to render fixed 3d meshes.  There are no provisions for changing
// any of the mesh data at run time.

class MeshAppearanceTemplate : public AppearanceTemplate
{
	friend class MeshAppearance;

public:

	static void install();

	static void setBuildingAsynchronousLoaderData(bool buildingAsynchronousLoaderData);

public:

	MeshAppearanceTemplate(const char *name, Iff *iff);
	virtual ~MeshAppearanceTemplate(void);

	virtual ShaderPrimitiveSetTemplate * getShaderPrimitiveSetTemplate(void);
	virtual ShaderPrimitiveSetTemplate const * getShaderPrimitiveSetTemplate(void) const;

	virtual Appearance *createAppearance() const;

	bool                isLoaded() const;

	const Sphere       &getSphere() const;

	void renderVolumetricShadow(const MeshAppearance* appearance, bool const usesVertexShader) const;
	void renderSimpleShadow(Vector const & position_o, float const radius) const;
	void renderReticle(Vector const & position_o, float const radius) const;
	void clearUpwardIndices() const;

	DPVS::Model        *getDpvsTestShape() const;

	virtual void garbageCollect () const;

	bool hasOnlyNonCollidableShaderTemplates () const;

protected:

	typedef stdvector<MeshAppearance*>::fwd MeshAppearances;

private:

	static AppearanceTemplate *create(const char *name, Iff *iff);
	static void                asynchronousLoadCallback(void *data);

private:

	// disable these routines
	MeshAppearanceTemplate(void);
	MeshAppearanceTemplate(const MeshAppearanceTemplate &);
	MeshAppearanceTemplate &operator =(const MeshAppearanceTemplate &);

	void asynchronousLoadCallback();
	void removeAsynchronouslyLoadedAppearance(MeshAppearance *meshAppearance) const;

	void meshAppearanceCreated() const;
	void meshAppearanceDestroyed();

	void load(Iff & iff);  //lint !e1511 // member hides non-virtual member in base class
	void load_0002(Iff & iff);
	void load_0003(Iff & iff);
	void load_0004(Iff & iff);
	void load_0005(Iff & iff);

	void loadSphere_old(Iff & iff);

private:

	int mutable                 m_meshAppearanceReferenceCount;

	bool mutable                m_requestedAsynchronousLoad;

	// sphere enclosing all the mesh vertices
	Sphere                      m_sphere;

	// shaders and primitives for this mesh appearance
	ShaderPrimitiveSetTemplate *m_shaderPrimitiveSetTemplate;

	DPVS::Model                *m_dpvsTestShape;

	// shadow volume is shared among all MeshAppearances using this template
	mutable ShadowVolume       *m_shadowVolume;
	mutable MeshAppearances    *m_uninitializedMeshAppearances;
};

// ======================================================================

inline bool MeshAppearanceTemplate::isLoaded() const
{
	return m_shaderPrimitiveSetTemplate != NULL;
}

// ----------------------------------------------------------------------

inline DPVS::Model  *MeshAppearanceTemplate::getDpvsTestShape() const
{
	return m_dpvsTestShape;
}

// ======================================================================

#endif
