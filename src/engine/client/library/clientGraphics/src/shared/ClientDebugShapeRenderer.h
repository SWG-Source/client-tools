// ======================================================================
//
// ClientDebugShapeRenderer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_ClientDebugShapeRenderer_H
#define INCLUDED_ClientDebugShapeRenderer_H

#include "sharedMath/DebugShapeRenderer.h"

class Cylinder;
class AxialBox;
class Sphere;
class IndexedTriangleList;
class Object;
class MemoryBlockManager;

class ClientDebugShapeRenderer : public DebugShapeRenderer
{
public:

	static void  install        ( void );

	static void *operator new(size_t size);
	static void  operator delete(void *memory);

public:

	ClientDebugShapeRenderer( Object const * object );
	virtual ~ClientDebugShapeRenderer();

	virtual Vector      getScale        ( void ) const;
	virtual Transform   getTransform    ( void ) const;

	virtual void drawFrame      ( float radius );

	virtual void drawSphere     ( Sphere const & sphere, int tess = 16 );
	virtual void drawCylinder   ( Cylinder const & cylinder, int tess = 16 );
	virtual void drawOCylinder  ( OrientedCylinder const & cylinder, int tess = 16 );
	virtual void drawBox        ( AxialBox const & box );
	virtual void drawYBox       ( YawedBox const & box );
	virtual void drawOBox       ( OrientedBox const & box );

	virtual void drawMesh       ( IndexedTriangleList const * mesh );
	virtual void drawMeshNormals( IndexedTriangleList const * mesh );
	virtual void drawLine       ( Vector const & begin, Vector const & end );
	virtual void drawLineList   ( VectorVec const & verts );
	virtual void drawPolygon    ( VectorVec const & verts );
	virtual void drawOctahedron ( Vector const & center, float radius );
	virtual void drawXZCircle   ( Vector const & center, float radius );

private:

	static void  remove         ( void );
	static DebugShapeRenderer * create ( Object const * object );

private:

	static MemoryBlockManager *ms_memoryBlockManager;

protected:

	Object const * m_object;
};

#endif


