// ======================================================================
//
// DebugPrimitive.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
//
// ======================================================================

#ifndef INCLUDED_DebugPrimitive_H
#define INCLUDED_DebugPrimitive_H

// ======================================================================

class MemoryBlockManager;
class VertexBufferFormat;

#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/AxialBox.h"

#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/StaticIndexBuffer.h"

// ======================================================================

class DebugPrimitive
{
public:

	DebugPrimitive();
	virtual ~DebugPrimitive();
	virtual void render() = 0;	

private:
	
	DebugPrimitive(const DebugPrimitive &);
	DebugPrimitive &operator =(const DebugPrimitive &);
};

// ======================================================================

class UtilityDebugPrimitive : public DebugPrimitive
{
public:

	enum Style
	{
		S_none,
		S_alpha,
		S_z,
		S_litZ,
	};

	UtilityDebugPrimitive(Style style, const Transform &objectToWorld, PackedArgb const & color = PackedArgb::solidGray);
	virtual void render() = 0;

	PackedArgb const & getColor ( void );
	void setColor( PackedArgb const & color );

	void setScale ( Vector const & scale );

protected:

	static VertexBufferFormat getVertexBufferFormat(Style style);

private:
	
	UtilityDebugPrimitive(const UtilityDebugPrimitive &);
	UtilityDebugPrimitive &operator =(const UtilityDebugPrimitive &);

protected:

	Style      m_style;
	Transform  m_objectToWorld;
	Vector     m_scale;
	PackedArgb m_color;
};

// ----------

inline void UtilityDebugPrimitive::setScale ( Vector const & scale )
{
	m_scale = scale;
}

// ======================================================================

class Line3dDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	Line3dDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &start, const Vector &end, const PackedArgb &color);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	Line3dDebugPrimitive(const Line3dDebugPrimitive &);
	Line3dDebugPrimitive &operator =(const Line3dDebugPrimitive &);

private:

	Vector     m_start;
	Vector     m_end;
};

// ======================================================================

class Line2dDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	Line2dDebugPrimitive(Style style, float x0, float y0, float x1, float y1, const PackedArgb &color);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	Line2dDebugPrimitive(const Line2dDebugPrimitive &);
	Line2dDebugPrimitive &operator =(const Line2dDebugPrimitive &);

private:

	float      m_x0;
	float      m_y0;
	float      m_x1;
	float      m_y1;
};

// ======================================================================

class FrameDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	FrameDebugPrimitive(Style sytle, const Transform &objectToWorld, float radius);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	FrameDebugPrimitive(const FrameDebugPrimitive &);
	FrameDebugPrimitive &operator =(const FrameDebugPrimitive &);

private:

	float      m_radius;
};

// ======================================================================

class FrustumDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	FrustumDebugPrimitive(Style style, const Transform &objectToWorld, const Vector *vertices, const PackedArgb &color);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	FrustumDebugPrimitive(const FrustumDebugPrimitive &);
	FrustumDebugPrimitive &operator =(const FrustumDebugPrimitive &);

private:

	Vector     m_vertices[8];
};

// ======================================================================

class SphereDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	SphereDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius, int rings, int segments);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	SphereDebugPrimitive(const SphereDebugPrimitive &);
	SphereDebugPrimitive &operator =(const SphereDebugPrimitive &);

private:

	Vector     m_center;
	float      m_radius;
	int        m_rings;
	int        m_segments;
};

// ======================================================================

class SphereDebugPrimitive2: public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	SphereDebugPrimitive2(Style style, const Transform &objectToWorld, const Vector &center, float radius, int tessTheta, int nLongitudes, int nLatitudes);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	SphereDebugPrimitive2(const SphereDebugPrimitive2&);
	SphereDebugPrimitive2&operator =(const SphereDebugPrimitive2&);

private:

	Vector     m_center;
	float      m_radius;
	int        m_tessTheta;
	int        m_longitudes;
	int        m_latitudes;
};

// ----------------------------------------------------------------------

class CircleDebugPrimitive: public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	CircleDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius, int tessTheta );
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	CircleDebugPrimitive(const CircleDebugPrimitive&);
	CircleDebugPrimitive&operator =(const CircleDebugPrimitive&);

private:

	Vector     m_center;
	float      m_radius;
	int        m_tessTheta;
};

// ----------------------------------------------------------------------

class OctahedronDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	OctahedronDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &center, float radius);
	virtual void render();

private:

static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	OctahedronDebugPrimitive(const OctahedronDebugPrimitive&);
	OctahedronDebugPrimitive&operator =(const OctahedronDebugPrimitive&);

private:

	Vector     m_center;
	float      m_radius;
};


// ----------------------------------------------------------------------

class CylinderDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	CylinderDebugPrimitive(Style style, const Transform &objectToWorld, const Vector &base, float radius, float height, int tessTheta, int tessRho, int tessZ, int nSpokes);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	CylinderDebugPrimitive(const CylinderDebugPrimitive &);
	CylinderDebugPrimitive &operator =(const CylinderDebugPrimitive &);

private:

	Vector     m_base;
	float      m_radius;
	float      m_height;

	int        m_tessTheta;
	int        m_tessRho;
	int        m_tessZ;
	int        m_nSpokes;
};

// ----------------------------------------------------------------------

class BoxDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	BoxDebugPrimitive(Style style, Transform const & objectToWorld, AxialBox const & box, PackedArgb const & color = PackedArgb::solidGray);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	BoxDebugPrimitive(const BoxDebugPrimitive &);
	BoxDebugPrimitive &operator =(const BoxDebugPrimitive &);

	AxialBox m_box;
};

// ======================================================================

class IndexedDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	enum PrimitiveType
	{
		PT_LineList,
		PT_TriangleList
	};

	IndexedDebugPrimitive(Style style, const Transform &objectToWorld, PrimitiveType primitiveType, int numberOfVertices, int numberOfIndices);
	virtual void render();

	StaticVertexBuffer &getVertexBuffer();
	StaticIndexBuffer  &getIndexBuffer();

	void setFillMode ( GlFillMode newMode );
	void setCullMode ( GlCullMode newMode );

private:
	
	IndexedDebugPrimitive(const IndexedDebugPrimitive &);
	IndexedDebugPrimitive &operator =(const IndexedDebugPrimitive &);

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:

	PrimitiveType       m_primitiveType;

	// @todo make these persistant dynamic
	StaticVertexBuffer  m_vertexBuffer;
	StaticIndexBuffer   m_indexBuffer;
	GlFillMode		    m_fillMode;
	GlCullMode          m_cullMode;
	
};

// ======================================================================

class VertexNormalsDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	VertexNormalsDebugPrimitive(const Transform &objectToWorld, const StaticVertexBuffer &staticVertexBuffer);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	VertexNormalsDebugPrimitive(const VertexNormalsDebugPrimitive &);
	VertexNormalsDebugPrimitive &operator =(const VertexNormalsDebugPrimitive &);

private:

	const StaticVertexBuffer  &m_vertexBuffer;
};

// ======================================================================

class VertexMatrixFramesDebugPrimitive : public UtilityDebugPrimitive
{
public:

	static void  install ();
	static void  remove ();

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	VertexMatrixFramesDebugPrimitive(const Transform &objectToWorld, const StaticVertexBuffer &staticVertexBuffer);
	virtual void render();

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:
	
	VertexMatrixFramesDebugPrimitive(const VertexMatrixFramesDebugPrimitive &);
	VertexMatrixFramesDebugPrimitive &operator =(const VertexMatrixFramesDebugPrimitive &);

private:

	const StaticVertexBuffer  &m_vertexBuffer;
};

// ======================================================================

#endif
