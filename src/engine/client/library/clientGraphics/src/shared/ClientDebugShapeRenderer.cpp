// ======================================================================
//
// ClientDebugShapeRenderer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ClientDebugShapeRenderer.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"

#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/PackedArgb.h"

#include "sharedObject/Object.h"

#include "sharedFoundation/MemoryBlockManager.h"

#include "sharedFoundation/ExitChain.h"

#include <vector>

// ----------------------------------------------------------------------

MemoryBlockManager *ClientDebugShapeRenderer::ms_memoryBlockManager;

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::install ( void )
{
	IS_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager = new MemoryBlockManager("ClientDebugShapeRenderer::memoryBlockManager", true, sizeof(ClientDebugShapeRenderer), 0, 0, 0);
	DebugShapeRenderer::setFactory( ClientDebugShapeRenderer::create );
	ExitChain::add(&remove, "ClientDebugShapeRenderer::remove");
}

void ClientDebugShapeRenderer::remove ( void )
{
	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

void *ClientDebugShapeRenderer::operator new(size_t size)
{
	UNREF(size);
	DEBUG_FATAL(size != sizeof(ClientDebugShapeRenderer), ("wrong size"));
	return ms_memoryBlockManager->allocate();
}

void  ClientDebugShapeRenderer::operator delete(void *memory)
{
	ms_memoryBlockManager->free(memory);
}

// ----------------------------------------------------------------------

ClientDebugShapeRenderer::ClientDebugShapeRenderer( Object const * object )
:
  m_object(object)
{
}

ClientDebugShapeRenderer::~ClientDebugShapeRenderer()
{
}

DebugShapeRenderer * ClientDebugShapeRenderer::create ( Object const * object )
{
	return new ClientDebugShapeRenderer( object );
}

// ----------------------------------------------------------------------

Vector ClientDebugShapeRenderer::getScale ( void ) const
{
    if(m_object && !m_worldSpace && m_applyScale)
    {
        return m_object->getScale();
    }
	else
	{
		return Vector::xyz111;
	}
}

Transform ClientDebugShapeRenderer::getTransform ( void ) const
{
	if(m_object && !m_worldSpace)
		return m_object->getTransform_o2w();

	return Transform::identity;
}

// ----------------------------------------------------------------------

UtilityDebugPrimitive::Style getStyle ( bool depthTest )
{
	if(depthTest)
		return UtilityDebugPrimitive::S_z;

	return UtilityDebugPrimitive::S_none;
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawCylinder( Cylinder const & cylinder, int tess )
{
	UNREF (cylinder);
	UNREF (tess);

#ifdef _DEBUG

    CylinderDebugPrimitive * prim = new CylinderDebugPrimitive(getStyle(m_depthTest), getTransform(), cylinder.getBase(), cylinder.getRadius(), cylinder.getHeight(), tess,2,4,8);

	prim->setColor(getColor());
	prim->setScale( getScale() );

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawOCylinder( OrientedCylinder const & ocylinder, int tess )
{
	UNREF (ocylinder);
	UNREF (tess);

#ifdef _DEBUG

	Transform shapeTransform = ocylinder.getTransform_l2p();

    Transform transform = getTransform().rotateTranslate_l2p(shapeTransform);

	Cylinder localCylinder = ocylinder.getLocalShape();

    CylinderDebugPrimitive * prim = new CylinderDebugPrimitive(getStyle(m_depthTest), transform, localCylinder.getBase(), localCylinder.getRadius(), localCylinder.getHeight(), tess,2,4,8);

	prim->setColor(getColor());
	prim->setScale( getScale() );

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawBox( AxialBox const & box )
{
	UNREF (box);

#ifdef _DEBUG

	drawOBox( OrientedBox(box) );

#endif
}

// ----------

void ClientDebugShapeRenderer::drawYBox ( YawedBox const & box )
{
	UNREF (box);

#ifdef _DEBUG

	drawOBox( OrientedBox(box) );

#endif
}

// ----------

void ClientDebugShapeRenderer::drawOBox ( OrientedBox const & box )
{
	UNREF (box);

#ifdef _DEBUG

	Transform shapeTransform = box.getTransform_l2p();

    Transform transform = getTransform().rotateTranslate_l2p(shapeTransform);

    BoxDebugPrimitive * prim = new BoxDebugPrimitive(getStyle(m_depthTest), transform, box.getLocalShape());

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawFrame( float radius )
{
	UNREF (radius);

#ifdef _DEBUG

	FrameDebugPrimitive * prim = new FrameDebugPrimitive(getStyle(m_depthTest), getTransform(), radius);

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawSphere( Sphere const & sphere, int tess )
{
	UNREF (sphere);
	UNREF (tess);

#ifdef _DEBUG

	SphereDebugPrimitive2 * prim = new SphereDebugPrimitive2(getStyle(m_depthTest), getTransform(), sphere.getCenter(), sphere.getRadius(), tess,8,7 );

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawMesh ( IndexedTriangleList const * mesh )
{
	if(!mesh) return;

	UNREF(mesh);

#ifdef _DEBUG

    IndexedDebugPrimitive * prim = new IndexedDebugPrimitive( getStyle(m_depthTest), 
                                                              getTransform(), 
															  IndexedDebugPrimitive::PT_TriangleList, 
                                                              mesh->getVertices().size(), 
                                                              mesh->getIndices().size() );


	prim->setColor(getColor());
	prim->setScale(getScale());
	prim->setFillMode(GFM_wire);
	prim->setCullMode(GCM_none);

	// ----------

	std::vector<Vector> const & vertices = mesh->getVertices();
	StaticVertexBuffer & vertexBuffer = prim->getVertexBuffer();

	vertexBuffer.lock();

		VertexBufferWriteIterator v = vertexBuffer.begin ();

		for(uint i = 0; i < vertices.size(); ++i, ++v)
		{
			v.setPosition (vertices [i]);
			v.setColor0 (prim->getColor());
		}

	vertexBuffer.unlock();

	// ----------

	std::vector<int> const & indices = mesh->getIndices();
	StaticIndexBuffer & indexBuffer = prim->getIndexBuffer();

	indexBuffer.lock();

		Index *ii = indexBuffer.begin ();

		for (uint j = 0; j < indices.size(); ++j, ++ii)
			*ii = static_cast<Index> (indices [j]);

	indexBuffer.unlock ();

	// ----------

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawMeshNormals ( IndexedTriangleList const * mesh )
{
	UNREF(mesh);

#ifdef _DEBUG
	if(!mesh) return;

	uint const numIndeces = mesh->getIndices().size();
	uint numPolys =  numIndeces / 3;
	
	if(numPolys == 0) return;


	//given a well formed mesh this should never happen, but just in case
	//we will toss out the left over verts
	if(numIndeces % 3 != 0)
	{
		--numPolys;
	}


	IndexedDebugPrimitive * const prim = new IndexedDebugPrimitive( getStyle(m_depthTest), 
		getTransform(), 
		IndexedDebugPrimitive::PT_LineList, 
		numPolys * 2, 
		numPolys * 2 );


	prim->setColor(getColor());
	prim->setScale(getScale());
	prim->setFillMode(GFM_wire);
	prim->setCullMode(GCM_none);



	// ----------

	//get the mesh data
	std::vector<Vector> const & meshVertices = mesh->getVertices();
	std::vector<int> const & meshIndices = mesh->getIndices();

	//write into the new prim buffer
	StaticVertexBuffer & vertexBuffer = prim->getVertexBuffer();

	vertexBuffer.lock();

	VertexBufferWriteIterator v = vertexBuffer.begin ();

	for(uint i = 0; i < numPolys; ++i)
	{
		uint startVertex = i * 3;
		Vector faceCenter = meshVertices[meshIndices[startVertex]] 
			+ meshVertices[meshIndices[startVertex+1]] 
			+ meshVertices[meshIndices[startVertex+2]];

		faceCenter /= 3.0f;

		Vector ab = meshVertices[meshIndices[startVertex+1]] - meshVertices[meshIndices[startVertex]];
		Vector ac = meshVertices[meshIndices[startVertex+2]] - meshVertices[meshIndices[startVertex]];

		Vector faceNormal = ab.cross(ac);
		faceNormal.approximateNormalize();

		Vector normalEndpoint = faceCenter + faceNormal;

		v.setPosition(faceCenter);
		v.setColor0(prim->getColor());
		++v;

		v.setPosition(normalEndpoint);
		v.setColor0(prim->getColor());
		++v;
	}

	vertexBuffer.unlock();

	// ----------

	StaticIndexBuffer & indexBuffer = prim->getIndexBuffer();

	indexBuffer.lock();

	Index *indices = indexBuffer.begin ();

	uint numNormalsX2 = numPolys*2;
	for (uint i = 0; i < numNormalsX2; ++i)
	{
		indices[i] = static_cast<Index>(i);
	}

	indexBuffer.unlock ();

	// ----------

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawLine ( Vector const & begin, Vector const & end )
{
	UNREF(begin);
	UNREF(end);

#ifdef _DEBUG

    Line3dDebugPrimitive * prim = new Line3dDebugPrimitive(getStyle(m_depthTest), getTransform(), begin,end,getColor());

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawLineList ( std::vector<Vector> const & verts )
{
	UNREF(verts);

#ifdef _DEBUG

	uint nVerts = verts.size();

	if(nVerts == 0) return;

    IndexedDebugPrimitive * prim = new IndexedDebugPrimitive( getStyle(m_depthTest), 
                                                              getTransform(), 
															  IndexedDebugPrimitive::PT_LineList, 
                                                              nVerts, 
                                                              nVerts );


	prim->setColor(getColor());
	prim->setScale(getScale());
	prim->setFillMode(GFM_wire);
	prim->setCullMode(GCM_none);

	// ----------

	StaticVertexBuffer & vertexBuffer = prim->getVertexBuffer();

	vertexBuffer.lock();

		VertexBufferWriteIterator v = vertexBuffer.begin ();

		uint i = 0;
		for(i = 0; i < nVerts; ++i, ++v)
		{
			v.setPosition (verts[i]);
			v.setColor0 (prim->getColor());
		}

	vertexBuffer.unlock();

	// ----------

	StaticIndexBuffer & indexBuffer = prim->getIndexBuffer();

	indexBuffer.lock();

		Index *indices = indexBuffer.begin ();

		for (i = 0; i < nVerts; i++) 
		{
			indices[i] = static_cast<Index>(i);
		}

	indexBuffer.unlock ();

	// ----------

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawPolygon ( std::vector<Vector> const & verts )
{
	UNREF(verts);

#ifdef _DEBUG

	uint nVerts = verts.size();
	uint nIndices = nVerts * 2;

    IndexedDebugPrimitive * prim = new IndexedDebugPrimitive( getStyle(m_depthTest), 
                                                              getTransform(), 
															  IndexedDebugPrimitive::PT_LineList, 
                                                              nVerts, 
                                                              nIndices );


	prim->setColor(getColor());
	prim->setScale(getScale());
	prim->setFillMode(GFM_wire);
	prim->setCullMode(GCM_none);

	// ----------

	StaticVertexBuffer & vertexBuffer = prim->getVertexBuffer();

	vertexBuffer.lock();

		VertexBufferWriteIterator v = vertexBuffer.begin ();

		uint i = 0;
		for(i = 0; i < nVerts; i++)
		{
			v.setPosition (verts[i]);
			v.setColor0 (prim->getColor());
			++v;
		}

	vertexBuffer.unlock();

	// ----------

	StaticIndexBuffer & indexBuffer = prim->getIndexBuffer();

	indexBuffer.lock();

		Index *indices = indexBuffer.begin ();

		for (i = 0; i < nVerts; i++) 
		{
			uint a = i;
			uint b = (i + 1) % nVerts;

			indices[i * 2 + 0] = static_cast<Index>(a);
			indices[i * 2 + 1] = static_cast<Index>(b);
		}

	indexBuffer.unlock ();

	// ----------

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawOctahedron( Vector const & center, float radius )
{
	UNREF (center);
	UNREF (radius);

#ifdef _DEBUG

    OctahedronDebugPrimitive * prim = new OctahedronDebugPrimitive(getStyle(m_depthTest), getTransform(), center, radius);

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}

// ----------------------------------------------------------------------

void ClientDebugShapeRenderer::drawXZCircle( Vector const & center, float radius )
{
	UNREF (center);
	UNREF (radius);

#ifdef _DEBUG

    CircleDebugPrimitive * prim = new CircleDebugPrimitive(getStyle(m_depthTest), getTransform(), center, radius, 16);

	prim->setColor(getColor());
	prim->setScale(getScale());

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(prim);

#endif
}


