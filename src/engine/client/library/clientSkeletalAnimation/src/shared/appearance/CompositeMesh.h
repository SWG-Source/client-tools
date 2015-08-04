// ==================================================================
//
// CompositeMesh.h
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#ifndef COMPOSITE_MESH_H
#define COMPOSITE_MESH_H

// ==================================================================
// forward declarations

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Appearance;
class Camera;
class CustomizationData;
class MeshConstructionHelper;
class MeshGenerator;
class ShaderPrimitive;
class ShaderTemplate;
class Skeleton;
class TextureRendererTemplate;
class Transform;
class TransformNameMap;
class Vector;

// ==================================================================

class CompositeMesh
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct TextureShaderMap
	{
		int m_textureRendererIndex;
		int m_shaderIndex;
		Tag m_shaderTextureTag;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<ShaderPrimitive*>::fwd  ShaderPrimitiveVector;


public:

	static void install();

public:

	CompositeMesh();
	~CompositeMesh();

	// mesh generator management
	void                      addMeshGenerator(const MeshGenerator *meshGenerator, CustomizationData *customizationData);
	void                      removeMeshGenerator(const MeshGenerator *meshGenerator);
	void                      removeAllMeshGenerators();

	int                       getMeshGeneratorCount() const;
	void                      getMeshGenerator(int index, int *layer, const MeshGenerator **meshGenerator) const;

	// mesh construction
	void                      applySkeletonModifications(Skeleton &skeleton) const;
	void                      addShaderPrimitives(Appearance &appearance, int lodIndex, const TransformNameMap &transformNameMap, ShaderPrimitiveVector &shaderPrimitives) const;

private:

	struct GeneratorLayer;
	struct GeneratorContainer;

private:

	static void remove();

private:

	GeneratorContainer *m_meshGenerators;

private:

	// disable these
	CompositeMesh(const CompositeMesh&);
	const CompositeMesh &operator =(const CompositeMesh&);

};

// ==================================================================

#endif
