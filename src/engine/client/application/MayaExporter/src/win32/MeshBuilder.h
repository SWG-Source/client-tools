// ======================================================================
//
// MeshBuilder.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/VectorArgb.h"
#include "ExporterDefines.h"
#include "DoubleVector.h"

#include "MayaHierarchy.h"
#include <vector>

class Extent;
class Hardpoint;
class Iff;
class MemoryBlockManager;
class Messenger;

typedef std::vector<Extent*> ExtentVec2;
typedef std::vector<Hardpoint*> HardpointVec;

// ======================================================================
// Provide an abstract interface for building a mesh or mesh-related
// complex object.  The MayaMeshReader class will build mesh objects
// using a MeshBuilder interface for output.
//
// This abstract base class corresponds to the builder from the 
// Builder Pattern (see Design Patterns, Gamma et. al).  The
// MayaMeshReader corresponds to the director, directing the construction
// process without knowing the details of what is being built.

class MeshBuilder
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum
	{
		MAX_UV_COUNT = 8
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Face
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	public:

		Vector      positionArray[3];
		Vector      vertexNormalArray[3];
		bool        diffuseColorsValid;
		VectorArgb  vertexDiffuseColorArray[3];
		Vector      faceNormal;

		int         uvCount;
		real        uArray[MAX_UV_COUNT][3];
		real        vArray[MAX_UV_COUNT][3];

		bool        dot3TransformMatrixValid;
		bool        dot3TransformMatrixFlipped;
		DoubleVector du;
		DoubleVector dv;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void   install(Messenger *messenger);
	static void   remove(void);

	static bool   collectHardpoints(MObject const * rootMayaObject, HardpointVec & outArray, char const * prefix, bool spam);
	static void   computeDot3Transform(Face &face, int normalMapTextureCoordinateIndex);

public:

	explicit MeshBuilder(MObject const & rootMayaObject);
	virtual ~MeshBuilder(void);

	virtual bool  hasFrames (void) const;
	virtual bool  setFrame(int animationFrameNumber, int firstAnimationFrameNumber, int lastAnimationFrameNumber, real framesPerSecond);
	virtual bool  setShaderTemplate(const char *shaderTemplateName, int *shaderTemplateIndex, bool dot3, int normalMapTextureCoordinateIndex);

	virtual void setShaderNormalsInfo(
		const char                *i_normalsBaseFileName, 
		CompressType               i_normalsCompressType,
		bool                       wrapU,
		bool                       wrapV,
		float                      heightMapScale
	);

	virtual bool  validateFace(Face &face) const;
	virtual bool  addTriangleFace(const Face &face);

	virtual bool  specifyNoMoreData(void);
	virtual void  getDot3TextureCoordinateInfo(const char *shaderTemplateName, bool &hasDot3Coordinate, int &textureCoordinateIndex) const;

	virtual bool  addNode ( MayaHierarchy::Node const * node );

	void          attachExtent ( Extent * pNewExtent );
	void          attachCollisionExtent ( Extent * pNewExtent );
	Extent const * getExtent ( void ) const;
	Extent const * getCollisionExtent ( void ) const;

	bool        addCollisionNode ( MayaHierarchy::Node const * node );

	// ----------

	Extent *    createExtent            ( MayaHierarchy::Node const * node );

	Extent *    createExtent_Extent     ( MayaHierarchy::Node const * node );
	Extent *    createExtent_Collision  ( MayaHierarchy::Node const * node );
	Extent *    createExtent_Primitive  ( MayaHierarchy::Node const * node );
	Extent *    createExtent_CMesh      ( MayaHierarchy::Node const * node );

	Extent *    createExtent_Lod        ( MayaHierarchy::Node const * node );
	Extent *    createExtent_Mesh       ( MayaHierarchy::Node const * node );
	Extent *    createExtent_Component  ( MayaHierarchy::Node const * node );

	Extent *    createExtent_ComponentParts ( MayaHierarchy::Node const * node );

	void        createChildExtents      ( MayaHierarchy::Node const * node, ExtentVec2 & extents, bool onlyAppearances );

	// ----------

	Extent *    createCylinderExtent    ( real radius, real height, Vector translate, Vector scale );
	Extent *    createSphereExtent      ( real radius, Vector translate, Vector scale );
	Extent *    createBoxExtent         ( real width, real height, real depth, Vector translate, Vector scale );
	Extent *    createDetailExtent      ( ExtentVec2 const & extents );
	Extent *    createComponentExtent   ( ExtentVec2 const & extents );

	// ----------

	bool        extractFloors           ( MayaHierarchy::Node const * node );
	void        addFloorName ( const char * name );

	virtual bool  write (Iff & iff) const;
	bool          writeExtents(Iff & iff) const;
	bool          writeHardpoints(Iff & iff) const;
	bool          writeFloors(Iff & iff) const;

	bool          collectHardpoints(void);

	void          processDot3Transform(Face &face) const;

protected:

	MayaHierarchy::Node const * findCollisionGroup ( MayaHierarchy::Node const * node );

	typedef std::vector<Extent*>    CollisionExtents;

	bool                m_dot3;
	int                 m_normalMapTextureCoordinateIndex;
	std::vector<std::string>           m_floors;
	HardpointVec      m_hardpoints;
	Extent *            m_extent;
	Extent *            m_collisionExtent;

private:

	MeshBuilder();
	MeshBuilder(MeshBuilder const &);
	MeshBuilder & operator=(MeshBuilder const &);

private:

	MObject const m_rootMayaObject;
};

// ======================================================================

#endif
