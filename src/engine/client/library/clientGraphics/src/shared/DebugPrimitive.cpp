// ======================================================================
//
// DebugPrimitive.cpp
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/DebugPrimitive.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

DebugPrimitive::DebugPrimitive()
{
}

// ----------------------------------------------------------------------

DebugPrimitive::~DebugPrimitive()
{
}

// ======================================================================

VertexBufferFormat UtilityDebugPrimitive::getVertexBufferFormat(Style style)
{
	VertexBufferFormat format;

	format.setPosition();
	format.setColor0();
	if (style == S_litZ)
		format.setNormal();

	return format;
}

// ----------------------------------------------------------------------

UtilityDebugPrimitive::UtilityDebugPrimitive(Style style, const Transform &objectToWorld, PackedArgb const & color )
: DebugPrimitive(),
  m_style(style),
  m_objectToWorld(objectToWorld),
  m_scale( Vector::xyz111 ),
  m_color(color)
{
}

// ----------------------------------------------------------------------

PackedArgb const & UtilityDebugPrimitive::getColor ( void )
{
	return m_color;
}

void UtilityDebugPrimitive::setColor ( PackedArgb const & color )
{
	m_color = color;
}

// ----------------------------------------------------------------------

void UtilityDebugPrimitive::render()
{
	switch (m_style)
	{
		case S_none:
			Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorStaticShader());
			break;

		case S_alpha:
			Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorAStaticShader());
			break;

		case S_z:
			Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
			break;

		case S_litZ:
			Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorLZStaticShader());
			break;

		default:
			DEBUG_FATAL(true, ("unreachable"));
	};

	Graphics::setObjectToWorldTransformAndScale(m_objectToWorld, m_scale);
}

// ======================================================================

MemoryBlockManager* Line3dDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void Line3dDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("Line3dDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("Line3dDebugPrimitive::ms_memoryBlockManager", true, sizeof(Line3dDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "Line3dDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void Line3dDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("Line3dDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* Line3dDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (Line3dDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void Line3dDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

Line3dDebugPrimitive::Line3dDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &start, const Vector &end, const PackedArgb &color)
: UtilityDebugPrimitive(style, objectToWorld,color),
	m_start(start),
	m_end(end)
{
}

// ----------------------------------------------------------------------

void Line3dDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawLine(m_start, m_end, m_color);
}

// ======================================================================

MemoryBlockManager* Line2dDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void Line2dDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("Line2dDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("Line2dDebugPrimitive::ms_memoryBlockManager", true, sizeof(Line2dDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "Line2dDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void Line2dDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("Line2dDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* Line2dDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (Line2dDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void Line2dDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

Line2dDebugPrimitive::Line2dDebugPrimitive(Style style, float x0, float y0, float x1, float y1, const PackedArgb &color)
: UtilityDebugPrimitive(style, Transform::identity,color),
	m_x0(x0),
	m_y0(y0),
	m_x1(x1),
	m_y1(y1)
{
}

// ----------------------------------------------------------------------

void Line2dDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawLine(m_x0, m_y0, m_x1, m_y1, m_color);
}

// ======================================================================

MemoryBlockManager* FrameDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void FrameDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("FrameDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("FrameDebugPrimitive::ms_memoryBlockManager", true, sizeof(FrameDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "FrameDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void FrameDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("FrameDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* FrameDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (FrameDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void FrameDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

FrameDebugPrimitive::FrameDebugPrimitive(Style style, const Transform &objectToWorld, float radius)
: UtilityDebugPrimitive(style, objectToWorld),
	m_radius(radius)
{
}

// ----------------------------------------------------------------------

void FrameDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawFrame(m_radius);
}

// ======================================================================

MemoryBlockManager* FrustumDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void FrustumDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("FrustumDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("FrustumDebugPrimitive::ms_memoryBlockManager", true, sizeof(FrustumDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "FrustumDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void FrustumDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("FrustumDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* FrustumDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (FrustumDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void FrustumDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

FrustumDebugPrimitive::FrustumDebugPrimitive(Style style, const Transform &objectToWorld, const Vector *vertices, const PackedArgb &color)
: UtilityDebugPrimitive(style, objectToWorld,color),
	m_vertices()
{
	memcpy(m_vertices, vertices, sizeof(m_vertices));
}

// ----------------------------------------------------------------------

void FrustumDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawFrustum(m_vertices, m_color);
}

// ======================================================================

MemoryBlockManager* SphereDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void SphereDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("SphereDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("SphereDebugPrimitive::ms_memoryBlockManager", true, sizeof(SphereDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "SphereDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void SphereDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("SphereDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* SphereDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (SphereDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SphereDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

SphereDebugPrimitive::SphereDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius, int rings, int segments)
: UtilityDebugPrimitive(style, objectToWorld),
	m_center(center),
	m_radius(radius),
	m_rings(rings),
	m_segments(segments)
{
}

// ----------------------------------------------------------------------

void SphereDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawSphere(m_center, m_radius, m_rings, m_segments);
}

// ======================================================================

MemoryBlockManager* SphereDebugPrimitive2::ms_memoryBlockManager;

//-------------------------------------------------------------------

void SphereDebugPrimitive2::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("SphereDebugPrimitive2 already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("SphereDebugPrimitive2::ms_memoryBlockManager", true, sizeof(SphereDebugPrimitive2), 0, 0, 0);

	ExitChain::add (remove, "SphereDebugPrimitive2::remove");
}

//-------------------------------------------------------------------

void SphereDebugPrimitive2::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("SphereDebugPrimitive2 is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* SphereDebugPrimitive2::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (SphereDebugPrimitive2), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SphereDebugPrimitive2::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

SphereDebugPrimitive2::SphereDebugPrimitive2(Style style, const Transform &objectToWorld, const Vector &center, float radius, int tessTheta, int longitudes, int latitudes )
: UtilityDebugPrimitive(style, objectToWorld),
	m_center(center),
	m_radius(radius),
	m_tessTheta(tessTheta),
	m_longitudes(longitudes),
	m_latitudes(latitudes)
{
}

// ----------------------------------------------------------------------

void SphereDebugPrimitive2::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawSphere2(m_center, m_radius, m_tessTheta, m_longitudes, m_latitudes, m_color);
}

// ======================================================================

MemoryBlockManager* CircleDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void CircleDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("CircleDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("CircleDebugPrimitive::ms_memoryBlockManager", true, sizeof(CircleDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "CircleDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void CircleDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("CircleDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* CircleDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (CircleDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void CircleDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

CircleDebugPrimitive::CircleDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius, int tessTheta )
: UtilityDebugPrimitive(style, objectToWorld),
	m_center(center),
	m_radius(radius),
	m_tessTheta(tessTheta)
{
}

// ----------------------------------------------------------------------

void CircleDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawXZCircle(m_center, m_radius, m_tessTheta, m_color);
}

// ======================================================================

MemoryBlockManager* OctahedronDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void OctahedronDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("OctahedronDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("OctahedronDebugPrimitive::ms_memoryBlockManager", true, sizeof(OctahedronDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "OctahedronDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void OctahedronDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("OctahedronDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* OctahedronDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (OctahedronDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void OctahedronDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

OctahedronDebugPrimitive::OctahedronDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius )
: UtilityDebugPrimitive(style, objectToWorld),
	m_center(center),
	m_radius(radius)
{
}

// ----------------------------------------------------------------------

void OctahedronDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawOctahedron(m_center, m_radius, m_color);
}

// ======================================================================

MemoryBlockManager* CylinderDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void CylinderDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("CylinderDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("CylinderDebugPrimitive::ms_memoryBlockManager", true, sizeof(CylinderDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "CylinderDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void CylinderDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("CylinderDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* CylinderDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (CylinderDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void CylinderDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

CylinderDebugPrimitive::CylinderDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &base, float radius, float height, int tessTheta, int tessRho, int tessZ, int nSpokes)
: UtilityDebugPrimitive(style, objectToWorld),
	m_base(base),
	m_radius(radius),
	m_height(height),
	m_tessTheta(tessTheta),
	m_tessRho(tessRho),
	m_tessZ(tessZ),
	m_nSpokes(nSpokes)
{
}

// ----------------------------------------------------------------------

void CylinderDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawCylinder(m_base, m_radius, m_height, m_tessTheta, m_tessRho, m_tessZ, m_nSpokes, m_color);
}

// ======================================================================

MemoryBlockManager* BoxDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void BoxDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("BoxDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("BoxDebugPrimitive::ms_memoryBlockManager", true, sizeof(BoxDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "BoxDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void BoxDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("BoxDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* BoxDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (BoxDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void BoxDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

BoxDebugPrimitive::BoxDebugPrimitive(Style style, Transform const & objectToWorld, AxialBox const & box, PackedArgb const & color) :
	UtilityDebugPrimitive(style, objectToWorld, color),
	m_box(box)
{
}

// ----------------------------------------------------------------------

void BoxDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();
	Graphics::drawBox(m_box.getMin(),m_box.getMax(), m_color);
}

// ======================================================================

MemoryBlockManager* IndexedDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void IndexedDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("IndexedDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("IndexedDebugPrimitive::ms_memoryBlockManager", true, sizeof(IndexedDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "IndexedDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void IndexedDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("IndexedDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* IndexedDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (IndexedDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void IndexedDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

IndexedDebugPrimitive::IndexedDebugPrimitive(Style style, const Transform &objectToWorld, PrimitiveType primitiveType, int numberOfVertices, int numberOfIndices)
: UtilityDebugPrimitive(style, objectToWorld),
	m_primitiveType(primitiveType),
	m_vertexBuffer(getVertexBufferFormat(style), numberOfVertices),
	m_indexBuffer(numberOfIndices),
	m_fillMode(GFM_solid),
	m_cullMode(GCM_counterClockwise)
{
}

// ----------------------------------------------------------------------

StaticVertexBuffer &IndexedDebugPrimitive::getVertexBuffer()
{
	return m_vertexBuffer;
}

// ----------------------------------------------------------------------

StaticIndexBuffer  &IndexedDebugPrimitive::getIndexBuffer()
{
	return m_indexBuffer;
}

// ----------------------------------------------------------------------

void IndexedDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();

	Graphics::setVertexBuffer(m_vertexBuffer);
	Graphics::setIndexBuffer(m_indexBuffer);

	const GlFillMode fillMode = Graphics::getFillMode ();
	Graphics::setFillMode (m_fillMode);

	const GlCullMode cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (m_cullMode);

	switch (m_primitiveType)
	{
		case PT_LineList:
			Graphics::drawIndexedLineList();
			break;

		case PT_TriangleList:
			Graphics::drawIndexedTriangleList();
			break;

		default:
			DEBUG_FATAL(true, ("unreachable"));
	};


	Graphics::setFillMode (fillMode);
	Graphics::setCullMode (cullMode);
}

void IndexedDebugPrimitive::setFillMode( GlFillMode newMode )
{
	m_fillMode = newMode;
}

void IndexedDebugPrimitive::setCullMode( GlCullMode newMode )
{
	m_cullMode = newMode;
}

// ======================================================================

MemoryBlockManager* VertexNormalsDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void VertexNormalsDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("VertexNormalsDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("VertexNormalsDebugPrimitive::ms_memoryBlockManager", true, sizeof(VertexNormalsDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "VertexNormalsDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void VertexNormalsDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("VertexNormalsDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* VertexNormalsDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (VertexNormalsDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void VertexNormalsDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

VertexNormalsDebugPrimitive::VertexNormalsDebugPrimitive(const Transform &objectToWorld, const StaticVertexBuffer &staticVertexBuffer)
: UtilityDebugPrimitive(S_z, objectToWorld),
	m_vertexBuffer(staticVertexBuffer)
{
	
}

// ----------------------------------------------------------------------

void VertexNormalsDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();

	m_vertexBuffer.lockReadOnly();
		Graphics::drawVertexNormals(m_vertexBuffer.beginReadOnly(), m_vertexBuffer.endReadOnly());
	m_vertexBuffer.unlock();
}

// ======================================================================

MemoryBlockManager* VertexMatrixFramesDebugPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void VertexMatrixFramesDebugPrimitive::install ()
{
	DEBUG_FATAL (ms_memoryBlockManager, ("VertexMatrixFramesDebugPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("VertexMatrixFramesDebugPrimitive::ms_memoryBlockManager", true, sizeof(VertexMatrixFramesDebugPrimitive), 0, 0, 0);

	ExitChain::add (remove, "VertexMatrixFramesDebugPrimitive::remove");
}

//-------------------------------------------------------------------

void VertexMatrixFramesDebugPrimitive::remove ()
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("VertexMatrixFramesDebugPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* VertexMatrixFramesDebugPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (VertexMatrixFramesDebugPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void VertexMatrixFramesDebugPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

VertexMatrixFramesDebugPrimitive::VertexMatrixFramesDebugPrimitive(const Transform &objectToWorld, const StaticVertexBuffer &staticVertexBuffer)
: UtilityDebugPrimitive(S_z, objectToWorld),
	m_vertexBuffer(staticVertexBuffer)
{
}

// ----------------------------------------------------------------------

void VertexMatrixFramesDebugPrimitive::render()
{
	UtilityDebugPrimitive::render();

	m_vertexBuffer.lockReadOnly();
		Graphics::drawVertexMatrixFrames(m_vertexBuffer.beginReadOnly(), m_vertexBuffer.endReadOnly(), 0.01f);
	m_vertexBuffer.unlock();
}

// ======================================================================
