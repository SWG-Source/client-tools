// ======================================================================
//
// ClientProceduralTerrainAppearance_ShaderSet.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearance_ShaderSet_H
#define INCLUDED_ClientProceduralTerrainAppearance_ShaderSet_H

// ======================================================================

#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/PackedArgb.h"

#include <vector>

class CollisionInfo;
class IndexedTriangleList;
class Plane;
class Shader;
class SystemIndexBuffer;
class SystemVertexBuffer;
class Transform;
class Vector;

// ======================================================================

class ClientProceduralTerrainAppearance::ShaderSet
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	explicit ShaderSet (const Shader* shader);
	~ShaderSet (void);

	void          addPrimitive (int x, int z, Vector const * positionList, Vector const * normalList, PackedArgb const * colorList, int numberOfTextureCoordinateSets, float baseUvScale, RotationType const * rotationType, const Sphere& sphere, const Rectangle2d& extent2d, bool excluded);
	void          chooseIndexBuffer(unsigned newHasLargerNeighborFlags, int numberOfTilesPerChunk);

	const Shader* getShader (void) const;
	void          render (const Camera* camera, const Shader* lotShader, const Shader* terrainCloudShader) const;
	void          renderNormals (void) const;

	bool          collide (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const;
	bool          getHeightAt (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const;

	void          getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const;
	void          renderShadowBlob (const Vector& position_o, float radius) const;
	void          renderReticle (const Vector& position_o, float radius) const;

public:

	static void  install (void);
	static void  remove (void);

public:

	class Primitive
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

		friend class ShaderSet;

	public:

		class PlaneArray;

	public:

		static void  install (void);
		static void  remove (void);

		static void getUv(RotationType rotationType, int index, float & u, float & v);

	public:

		Primitive (void);
		~Primitive (void);

		Vector const * getPositionList() const;
		Vector const * getNormalList() const;
		PackedArgb const * getColorList() const;
		int getNumberOfTextureCoordinateSets() const;
		float getBaseUvScale() const;
		RotationType const * getRotationType() const;
		SystemIndexBuffer const * getIndexBuffer() const;
		Sphere const & getSphere() const;
		Rectangle2d const & getExtent2d() const;

	private:

		bool                      m_excluded;
		int                       m_x;
		int                       m_z;
		Vector m_positionList[9];
		Vector m_normalList[9];
		PackedArgb m_colorList[9];
		int m_numberOfTextureCoordinateSets;
		float m_baseUvScale;
		RotationType m_rotationType[4];
		const SystemIndexBuffer*  m_indexBuffer;
		PlaneArray*               m_planeArray;
		Sphere                    m_sphere;
		Rectangle2d               m_extent2d;
	};

	typedef std::vector<Primitive*> PrimitiveList;

private:

	ShaderSet (void);
	ShaderSet(ShaderSet const & rhs);
	ShaderSet & operator=(ShaderSet const & rhs);

private:

	const Shader*  m_shader;

	PrimitiveList  m_primitiveList;
};

// ======================================================================

inline Sphere const & ClientProceduralTerrainAppearance::ShaderSet::Primitive::getSphere() const
{
	return m_sphere;
}

//----------------------------------------------------------------------

inline Rectangle2d const & ClientProceduralTerrainAppearance::ShaderSet::Primitive::getExtent2d() const
{
	return m_extent2d;
}

//----------------------------------------------------------------------

inline Vector const * ClientProceduralTerrainAppearance::ShaderSet::Primitive::getPositionList() const
{
	return m_positionList;
}

// ----------------------------------------------------------------------

inline Vector const * ClientProceduralTerrainAppearance::ShaderSet::Primitive::getNormalList() const
{
	return m_normalList;
}

// ----------------------------------------------------------------------

inline PackedArgb const * ClientProceduralTerrainAppearance::ShaderSet::Primitive::getColorList() const
{
	return m_colorList;
}

// ----------------------------------------------------------------------

inline int ClientProceduralTerrainAppearance::ShaderSet::Primitive::getNumberOfTextureCoordinateSets() const
{
	return m_numberOfTextureCoordinateSets;
}

// ----------------------------------------------------------------------

inline float ClientProceduralTerrainAppearance::ShaderSet::Primitive::getBaseUvScale() const
{
	return m_baseUvScale;
}

// ----------------------------------------------------------------------

inline ClientProceduralTerrainAppearance::RotationType const * ClientProceduralTerrainAppearance::ShaderSet::Primitive::getRotationType() const
{
	return m_rotationType;
}

// ----------------------------------------------------------------------

inline SystemIndexBuffer const * ClientProceduralTerrainAppearance::ShaderSet::Primitive::getIndexBuffer() const
{
	return m_indexBuffer;
}

// ======================================================================

#endif
