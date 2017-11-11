// =====================================================================
//
// ClientGlobalWaterManager2.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// =====================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientGlobalWaterManager2.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/ShaderCapability.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Plane.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"

#include <algorithm>
#include <vector>

// =====================================================================
// ClientGlobalWaterManager2Namespace
// =====================================================================

namespace ClientGlobalWaterManager2Namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_MAIN = TAG (M,A,I,N);
	const Tag TAG_WATR = TAG (W,A,T,R);

#ifdef _DEBUG
	bool      ms_reportClientGlobalWaterManager2;
	bool      ms_renderGlobalWaterManager2Normals;
	bool      ms_noRenderWater;
#endif


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ClientGlobalWaterManager2Namespace;

//===================================================================
//#define FIELD_OF_VIEW (90.0f/360.0f * PI_TIMES_2)
//#define MAX_RES 1024
//#define PIXEL_ANGLE (FIELD_OF_VIEW / MAX_RES)
#define MESH_RESOLUTION			64
#define MAX_DETAIL_RADIUS		32
#define WATER_QUANTIZATION		1.5 // meters
//#define MESH_BOX     1024

#define QUANTIZE(_x_) (WATER_QUANTIZATION * ceil(double(_x_) * (1.0/WATER_QUANTIZATION)))

#define POS_WATER_QUANTIZATION (WATER_QUANTIZATION * 1.0)
#define POS_QUANTIZE(_x_) (POS_WATER_QUANTIZATION * floor(double(_x_) * (1.0/POS_WATER_QUANTIZATION) + 0.5) )

// =====================================================================
// ClientGlobalWaterManager2::LocalShaderPrimitive
// =====================================================================

class ClientGlobalWaterManager2::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	LocalShaderPrimitive (const Appearance& appearance, float mapWidthInMeters, const char* shaderTemplateName, float shaderSize, float height, Rectangle2d const * clipRegion);
	virtual ~LocalShaderPrimitive ();

	virtual float               alter (float elapsedTime);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;

	bool isPotentiallyVisible() const;

	virtual float getRadius() const { return 4000.0f; };

protected:

	enum {
		 Z_QUADRANT    = (1<<0)
		,X_QUADRANT    = (1<<1)
		,NZ_QUADRANT   = (1<<2)
		,NX_QUADRANT   = (1<<3)
		,ALL_QUADRANTS = Z_QUADRANT | X_QUADRANT | NZ_QUADRANT | NX_QUADRANT
	};

	typedef std::vector<int>    IndexArray;
	typedef std::vector<Vector> VertexArray;

	void _reset() const;

	unsigned _getPotentiallyVisibleQuadrants(const Camera &i_camera) const;

	VertexBufferFormat _getVertexFormat() const;

	void _fillGraphicsBuffersFFP(VertexBufferWriteIterator i_vv, Index *i_ii, Vector normal) const;
	void _fillGraphicsBuffersVS(VertexBufferWriteIterator i_vv, Index *i_ii) const;

	void _tesselate(
		const Vector &referencePosition_w, 
		const float fieldOfView,
		const float farPlane,
		const float resolution,
		const unsigned quadrants
		) const;

	bool               _isEmpty()          const { return m_isEmpty; };
	float              _height()           const { return m_height; }
	float              _heightDelta()      const { return m_heightDelta; }
	bool               _usesVertexShader() const { return m_usesVertexShader; }
	float              _shaderSize()       const { return m_shaderSize; }
	double             _time()             const { return m_time; }
	const Rectangle2d &_clipRegion()       const { return m_clipRegion; }
	bool               _usesClipRegion()   const { return m_usesClipRegion; }

	mutable VertexArray         m_vertexArray;
	mutable IndexArray          m_indexArray;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive& rhs);             //lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive)
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive& rhs);  //lint -esym(754, LocalShaderPrimitive::operator=)

	struct TessVertList
	{
		TessVertList(int i_num_verts, const Vector *i_verts, int i_index_offset)
			: num_verts(i_num_verts), verts(i_verts), index_offset(i_index_offset)
		{}

		int num_verts;
		const Vector *verts;
		int index_offset;
	};
	void _tesselateStrip(IndexArray &o_indeces, TessVertList vl1, TessVertList vl2) const;

	const Appearance&           m_appearance;
	const float                 m_mapWidthInMeters;
	Shader* const               m_shader;
	const bool                  m_usesVertexShader;
	const bool                  m_usesClipRegion;
	mutable bool                m_isEmpty;
	const float                 m_shaderSize;
	const float                 m_height;
	const float                 m_heightDelta;
	const Rectangle2d           m_clipRegion;
	double                      m_time;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClientGlobalWaterManager2::StaticShaderPrimitive : public ClientGlobalWaterManager2::LocalShaderPrimitive
{
public:

	StaticShaderPrimitive(
		const Appearance& appearance, 
		float mapWidthInMeters, 
		const char* shaderTemplateName, 
		float shaderSize, 
		float height, 
		Rectangle2d const * clipRegion
		);
	virtual ~StaticShaderPrimitive();

	virtual void prepareToDraw () const;
	virtual void draw () const;

private:

	StaticShaderPrimitive(const StaticShaderPrimitive& rhs); // Disabled
	StaticShaderPrimitive&operator= (const StaticShaderPrimitive& rhs); // Disabled

	void _buildGraphicsBuffers() const;

	mutable unsigned            m_renderQuadrants;
	mutable StaticVertexBuffer *m_vertexBuffer;
	mutable StaticIndexBuffer  *m_indexBuffer;
	mutable bool   m_above;
	mutable float  m_fieldOfView;
	mutable float  m_farPlane;
	mutable float  m_resolution;
};

ClientGlobalWaterManager2::StaticShaderPrimitive::StaticShaderPrimitive(
	const Appearance& appearance, 
	float mapWidthInMeters, 
	const char* shaderTemplateName, 
	float shaderSize, 
	float height, 
	Rectangle2d const * clipRegion
	)
:	LocalShaderPrimitive(appearance, mapWidthInMeters, shaderTemplateName, shaderSize, height, clipRegion),
	m_renderQuadrants(ALL_QUADRANTS),
	m_vertexBuffer(0),
	m_indexBuffer(0),
	m_above(false),
	m_fieldOfView(0),
	m_farPlane(0),
	m_resolution(0)
{
}

ClientGlobalWaterManager2::StaticShaderPrimitive::~StaticShaderPrimitive()
{
	delete m_vertexBuffer;
	delete m_indexBuffer;
}

void ClientGlobalWaterManager2::StaticShaderPrimitive::_buildGraphicsBuffers() const
{
	delete m_vertexBuffer;
	m_vertexBuffer=0;
	delete m_indexBuffer;
	m_indexBuffer=0;

	_reset();

	_tesselate(Vector(0.0f, _height()+2.0f, 0.0f), m_fieldOfView, m_farPlane, m_resolution, ALL_QUADRANTS);

	if (_isEmpty())
	{
		return;
	}

	m_vertexBuffer = new StaticVertexBuffer(_getVertexFormat(), m_vertexArray.size());
	m_indexBuffer = new StaticIndexBuffer(m_indexArray.size());

	m_vertexBuffer->lock();
	VertexBufferWriteIterator vv = m_vertexBuffer->begin();

	m_indexBuffer->lock();
	Index *ii = m_indexBuffer->begin();

	if (_usesVertexShader())
	{
		_fillGraphicsBuffersVS(vv, ii);
	}
	else
	{
		const Vector normal = (m_above) ? Vector(0,1,0) : Vector(0,-1,0);
		_fillGraphicsBuffersFFP(vv, ii, normal);
	}

	m_vertexBuffer->unlock ();
	m_indexBuffer->unlock();
}

void ClientGlobalWaterManager2::StaticShaderPrimitive::prepareToDraw () const
{
	const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();
	const float fieldOfView = camera.getHorizontalFieldOfView();
	const float farPlane = camera.getFarPlane();
	const float resolution = float(camera.getViewportWidth());

	const bool above = camera.getPosition_w ().y > _height();

	m_renderQuadrants = _getPotentiallyVisibleQuadrants(camera);
	if (m_renderQuadrants==0)
	{
		return;
	}

	if (  !m_vertexBuffer
		|| fieldOfView != m_fieldOfView
		|| farPlane != m_farPlane
		|| resolution != m_resolution
		|| !_usesVertexShader() && above!=m_above
		)
	{
		m_fieldOfView = fieldOfView;
		m_farPlane    = farPlane;
		m_resolution  = resolution;
		m_above       = above;
		_buildGraphicsBuffers();
	}

	if (_isEmpty())
	{
		return;
	}

	const Vector normal = (above) ? Vector(0,1,0) : Vector(0,-1,0);
	const float ooShaderSize = 1.0f / _shaderSize();
	const float stime = float(_time() * 2.0);
	const Vector referencePosition_w = camera.getPosition_w();
	const float xref = float(POS_QUANTIZE(referencePosition_w.x));
	const float zref = float(POS_QUANTIZE(referencePosition_w.z));

	Graphics::setVertexShaderUserConstants(0, normal.y, stime, ooShaderSize, _heightDelta());
	Graphics::setVertexShaderUserConstants(1, 0.0f, 0.0f, 0.0f, 0.0f);
	Graphics::setVertexShaderUserConstants(2, xref, 0.0f, zref, 0.0f);
	Graphics::setVertexShaderUserConstants(3, _clipRegion().x0, -65536.0f, _clipRegion().y0, -65536.0f);
	Graphics::setVertexShaderUserConstants(4, _clipRegion().x1,  65536.0f, _clipRegion().y1,  65536.0f);

	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111); 
	//Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer(*m_vertexBuffer);
	Graphics::setIndexBuffer(*m_indexBuffer);
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::StaticShaderPrimitive::draw () const
{
	if (m_renderQuadrants==0)
	{
		return;
	}
	if (_isEmpty())
	{
		return;
	}

	const GlCullMode cullMode = Graphics::getCullMode ();

	if (ShaderPrimitiveSorter::getCurrentCamera().isUnderWater())
		Graphics::setCullMode (GCM_clockwise);
	else
		Graphics::setCullMode (GCM_counterClockwise);

	const int numVerts = m_vertexArray.size();
	const int numQuadVerts = numVerts / 4;

	const int numIndeces = m_indexArray.size();
	const int numQuadIndeces = numIndeces / 4;
	const int numQuadPrimitives = numQuadIndeces / 3;

	if (m_renderQuadrants & Z_QUADRANT)
	{
		Graphics::drawIndexedTriangleList(0, 0 * numQuadVerts, numQuadVerts, 0 * numQuadIndeces, numQuadPrimitives);
	}
	if (m_renderQuadrants & X_QUADRANT)
	{
		Graphics::drawIndexedTriangleList(0, 1 * numQuadVerts, numQuadVerts, 1 * numQuadIndeces, numQuadPrimitives);
	}
	if (m_renderQuadrants & NZ_QUADRANT)
	{
		Graphics::drawIndexedTriangleList(0, 2 * numQuadVerts, numQuadVerts, 2 * numQuadIndeces, numQuadPrimitives);
	}
	if (m_renderQuadrants & NX_QUADRANT)
	{
		Graphics::drawIndexedTriangleList(0, 3 * numQuadVerts, numQuadVerts, 3 * numQuadIndeces, numQuadPrimitives);
	}

	Graphics::setCullMode (cullMode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClientGlobalWaterManager2::DynamicShaderPrimitive : public ClientGlobalWaterManager2::LocalShaderPrimitive
{
public:

	DynamicShaderPrimitive(
		const Appearance& appearance, 
		float mapWidthInMeters, 
		const char* shaderTemplateName, 
		float shaderSize, 
		float height, 
		Rectangle2d const * clipRegion
		);

	virtual void prepareToDraw () const;
	virtual void draw () const;

private:

	DynamicShaderPrimitive(const DynamicShaderPrimitive& rhs); // Disabled
	DynamicShaderPrimitive&operator= (const DynamicShaderPrimitive& rhs); // Disabled

	mutable DynamicVertexBuffer m_vertexBuffer;
	mutable DynamicIndexBuffer  m_indexBuffer;
};

ClientGlobalWaterManager2::DynamicShaderPrimitive::DynamicShaderPrimitive(
	const Appearance& appearance, 
	float mapWidthInMeters, 
	const char* shaderTemplateName, 
	float shaderSize, 
	float height, 
	Rectangle2d const * clipRegion
	)
:	LocalShaderPrimitive(appearance, mapWidthInMeters, shaderTemplateName, shaderSize, height, clipRegion),
	m_vertexBuffer(_getVertexFormat()),
	m_indexBuffer()
{
}

void ClientGlobalWaterManager2::DynamicShaderPrimitive::prepareToDraw () const
{
	_reset();

	const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();

	unsigned renderQuadrants = _getPotentiallyVisibleQuadrants(camera);
	if (renderQuadrants==0)
	{
		return;
	}

	const Vector referencePosition_w = camera.getPosition_w();
	const float fieldOfView = camera.getHorizontalFieldOfView();
	const float farPlane = camera.getFarPlane();
	const float resolution = float(camera.getViewportWidth());

	_tesselate(referencePosition_w, fieldOfView, farPlane, resolution, renderQuadrants);

	if (_isEmpty())
	{
		return;
	}

	const bool above = camera.getPosition_w ().y > _height();

	m_vertexBuffer.lock(m_vertexArray.size());
	VertexBufferWriteIterator vv = m_vertexBuffer.begin();

	m_indexBuffer.lock(m_indexArray.size());
	Index *ii = m_indexBuffer.begin();

	if (_usesVertexShader())
	{
		_fillGraphicsBuffersVS(vv, ii);
	}
	else
	{
		const Vector normal = (above) ? Vector(0,1,0) : Vector(0,-1,0);
		_fillGraphicsBuffersFFP(vv, ii, normal);
	}

	m_vertexBuffer.unlock();
	m_indexBuffer.unlock();

	if (_usesVertexShader())
	{
		const Vector normal = (above) ? Vector(0,1,0) : Vector(0,-1,0);
		const float ooShaderSize = 1.0f / _shaderSize();
		const float stime = float(_time() * 2.0);
		const Vector referencePosition_w = camera.getPosition_w();

		//const float xref = float(POS_QUANTIZE(referencePosition_w.x));
		//const float zref = float(POS_QUANTIZE(referencePosition_w.z));

		Graphics::setVertexShaderUserConstants(0, normal.y, stime, ooShaderSize, _heightDelta());
		Graphics::setVertexShaderUserConstants(1, 0.0f, 0.0f, 0.0f, 0.0f);
		Graphics::setVertexShaderUserConstants(2, 0.0f, 0.0f, 0.0f, 0.0f);
		Graphics::setVertexShaderUserConstants(3, _clipRegion().x0, -65536.0f, _clipRegion().y0, -65536.0f);
		Graphics::setVertexShaderUserConstants(4, _clipRegion().x1,  65536.0f, _clipRegion().y1,  65536.0f);
	}

	//-- setup the data to render
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111); 
	//Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer(m_vertexBuffer);
	Graphics::setIndexBuffer(m_indexBuffer);
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::DynamicShaderPrimitive::draw () const
{
	if (_isEmpty())
	{
		return;
	}
	const GlCullMode cullMode = Graphics::getCullMode ();

	if (ShaderPrimitiveSorter::getCurrentCamera().isUnderWater())
		Graphics::setCullMode (GCM_clockwise);
	else
		Graphics::setCullMode (GCM_counterClockwise);


	Graphics::drawIndexedTriangleList ();
	Graphics::setCullMode (cullMode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------

ClientGlobalWaterManager2::LocalShaderPrimitive::LocalShaderPrimitive (const Appearance& appearance, const float mapWidthInMeters, const char* const shaderTemplateName, const float shaderSize, const float height, Rectangle2d const * clipRegion) :
	ShaderPrimitive (),
	m_vertexArray (),
	m_indexArray (),
	m_appearance (appearance),
	m_mapWidthInMeters (mapWidthInMeters),
	m_shader (ShaderTemplateList::fetchModifiableShader (shaderTemplateName)),
	m_usesVertexShader(Graphics::getShaderCapability() >= ShaderCapability(1,1)),
	m_usesClipRegion(clipRegion!=0),
	m_isEmpty(true),
	m_shaderSize (shaderSize),
	m_height (height),
	m_heightDelta(m_usesClipRegion ? 0.0f : 0.03f),
	m_clipRegion((m_usesClipRegion) ? (*clipRegion) : (Rectangle2d(-2*m_mapWidthInMeters, -2*m_mapWidthInMeters, 2*m_mapWidthInMeters, 2*m_mapWidthInMeters))),
	m_time (0.f)
{
	_reset();
}

// ---------------------------------------------------------------------

ClientGlobalWaterManager2::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
	m_shader->release ();
}  //lint !e1740  //-- m_shader not freed or zero'ed

//-------------------------------------------------------------------

float ClientGlobalWaterManager2::LocalShaderPrimitive::alter (const float elapsedTime)
{
	m_time += elapsedTime;
	m_shader->alter (elapsedTime);

	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const Vector ClientGlobalWaterManager2::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ClientGlobalWaterManager2::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return -m_height;
}

// ----------------------------------------------------------------------

VertexBufferFormat ClientGlobalWaterManager2::LocalShaderPrimitive::_getVertexFormat() const
{
	VertexBufferFormat format;
	format.setPosition();
	if (!m_usesVertexShader)
	{
		format.setNormal();
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
	}
	return format;
}

// ----------------------------------------------------------------------

int ClientGlobalWaterManager2::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ClientGlobalWaterManager2::LocalShaderPrimitive::prepareToView () const
{
	return m_shader->prepareToView ();
}

//-------------------------------------------------------------------
#pragma warning (push)
#pragma warning (disable: 4127) // conditional expression is constant
void ClientGlobalWaterManager2::LocalShaderPrimitive::_tesselateStrip(IndexArray &o_indeces, TessVertList vl1, TessVertList vl2) const
{
	int i1, i2;

	DEBUG_FATAL(vl1.num_verts<=0 || vl2.num_verts<=0, ("tesselation error"));
	DEBUG_FATAL(vl1.num_verts<=1 && vl2.num_verts<=1, ("tesselation error"));
	
	i1 = 0;
	i2 = 0;
	do
	{
		int inext1 = i1+1;
		int inext2 = i2+1;

		if (inext1<vl1.num_verts && inext2<vl2.num_verts)
		{
			const Vector &p1     =vl1.verts[i1];
			const Vector &p1next =vl1.verts[inext1];
			const Vector &p2     =vl2.verts[i2];
			const Vector &p2next =vl2.verts[inext2];

			float mag1 = sqr(p2next.x-p1.x) + sqr(p2next.z-p1.z);
			float mag2 = sqr(p1next.x-p2.x) + sqr(p1next.z-p2.z);

			if (mag1<=mag2)
			{
				o_indeces.push_back(i1 + vl1.index_offset);
				o_indeces.push_back(i2 + vl2.index_offset);
				o_indeces.push_back(inext2 + vl2.index_offset);

				// advance second list
				i2++;
			}
			else
			{
				o_indeces.push_back(i1 + vl1.index_offset);
				o_indeces.push_back(i2 + vl2.index_offset);
				o_indeces.push_back(inext1 + vl1.index_offset);

				// advance first list
				i1++;
			}
		}
		else if (inext1<vl1.num_verts)
		{
			o_indeces.push_back(i1 + vl1.index_offset);
			o_indeces.push_back(i2 + vl2.index_offset);
			o_indeces.push_back(inext1 + vl1.index_offset);

			// advance first list
			i1++;
		}
		else if (inext2<vl2.num_verts)
		{
			o_indeces.push_back(i1 + vl1.index_offset);
			o_indeces.push_back(i2 + vl2.index_offset);
			o_indeces.push_back(inext2 + vl2.index_offset);

			// advance second list
			i2++;
		}
		else
		{
			break;
		}

	} while (true);
}
#pragma warning (pop)

void ClientGlobalWaterManager2::LocalShaderPrimitive::_tesselate(const Vector &referencePosition_w, 
																					  const float fieldOfView,
																					  const float farPlane,
																					  const float resolution,
																					  const unsigned quadrants
																					  ) const
{
	double y;
	std::vector<Vector> verts1, verts2;

	const float pixelAngle = fieldOfView / float(resolution);

	/*
	float altitude = abs(referencePosition_w.y - m_height);
	if (altitude<1.0f)
	{
		altitude=1.0f;
	}
	*/
	float altitude=2;
	float altitudeSquared = altitude*altitude;

	float xref = float(POS_QUANTIZE(referencePosition_w.x));
	float zref = float(POS_QUANTIZE(referencePosition_w.z));

	for (int quadrant=0;quadrant<4;quadrant++)
	{
		if (!(quadrants & (1<<quadrant)))
		{
			continue;
		}

		y=0;
		verts1.clear();
		verts1.push_back(Vector(xref, m_height, zref));
		int indexOffset1=-1;
		double last_ystep=WATER_QUANTIZATION/2.0;
		int new_tris=0;

		do
		{
			//////////////////////////////////////////////////////////////////////////////////////////
			// figure out desired step
			// The angle between the view vector and the vertical is defined as atan(y / altitude)
			// Compute the rate at which the view vector moves across the ground as the angle between
			// the view vector and the vertical changes (using the first derivative of tan(x))
			const double cos_angle = double(altitude) / sqrt(double(altitudeSquared) + y*y);
			const double dscreen = altitude * (1.0 / (cos_angle*cos_angle)); // meters per radian
			const double dpixels = dscreen * pixelAngle; //  meters per pixel
			double ystep = MESH_RESOLUTION * dpixels; // meters
			if (y<MAX_DETAIL_RADIUS)
			{
				ystep=WATER_QUANTIZATION;
			}
			else if (ystep<=WATER_QUANTIZATION)
			{
				ystep=WATER_QUANTIZATION;
			}
			// ystep now holds an approximation of the amount to increase the tesselation to maintian
			// a constant step in screen space (MESH_RESOLUTION).
			//////////////////////////////////////////////////////////////////////////////////////////

			if (ystep>=2.0*last_ystep && (new_tris&3)==0)
			{
				// Expansion case
				// ystep is greater than double our last step and the number of triangles created
				// last time is divisible by four.
				ystep=2.0*last_ystep; // only step by a factor of two to keep our mesh aligned.
				new_tris = new_tris/2 + 2;
				y+=ystep;
				last_ystep = ystep;
			}
			else
			{
				// Hold case
				// Just stick with out last step.
				y+=last_ystep;
				new_tris+=2;
			}

			float xwidth = 2.0f*float(y);
			float xstep = xwidth / float(new_tris);

			/////////////////////////////////////////////////////////////////////////
			verts2.clear();
			const float vy = float(y);
			float vx=-xwidth/2;
			switch (quadrant)
			{
			case 0: 
			{
				for (int i=0;i<new_tris;i++, vx+=xstep)
				{
					verts2.push_back(Vector(vx+xref, m_height, vy+zref));
				}
				verts2.push_back(Vector(xwidth/2.0f+xref, m_height, vy+zref));
			} break;
			case 1: 
			{
				for (int i=0;i<new_tris;i++, vx+=xstep)
				{
					verts2.push_back(Vector(vy+xref, m_height, -vx+zref));
				}
				verts2.push_back(Vector(vy+xref, m_height, -xwidth/2.0f+zref));
			} break;
			case 2: 
			{
				for (int i=0;i<new_tris;i++, vx+=xstep)
				{
					verts2.push_back(Vector(-vx+xref, m_height, -vy+zref));
				}
				verts2.push_back(Vector(-(xwidth/2.0f)+xref, m_height, -vy+zref));
			} break;
			case 3: 
			{
				for (int i=0;i<new_tris;i++, vx+=xstep)
				{
					verts2.push_back(Vector(-vy+xref, m_height, vx+zref));
				}
				verts2.push_back(Vector(-vy+xref, m_height, xwidth/2.0f+zref));
			} break;
			}
			/////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////
			if (indexOffset1<0)
			{
				indexOffset1=m_vertexArray.size();
				m_vertexArray.insert(m_vertexArray.end(), verts1.begin(), verts1.end());
			}
			TessVertList l1(verts1.size(), &verts1[0], indexOffset1);
			//////////////////////////////////////////////////////////////////
			const int indexOffset2=m_vertexArray.size();
			m_vertexArray.insert(m_vertexArray.end(), verts2.begin(), verts2.end());
			TessVertList l2(verts2.size(), &verts2[0], indexOffset2);
			//////////////////////////////////////////////////////////////////

			_tesselateStrip(m_indexArray, l1, l2);

			//////////////////////////////////////////////////////////////////

			verts1.swap(verts2);
			indexOffset1=indexOffset2;
		} while (y<farPlane);
	}

	m_isEmpty = m_vertexArray.empty();

	/*
	int num_tris = m_indexArray.size() / 3;
	int num_verts = m_vertexArray.size();
	num_tris=0;
	num_verts=0;
	*/
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::LocalShaderPrimitive::_fillGraphicsBuffersFFP(VertexBufferWriteIterator i_vv, Index *const i_ii, Vector normal) const
{
	const float ooShaderSize = 1.0f / m_shaderSize;
	const float stime = float(m_time * 2.0);
	VertexBufferWriteIterator vv = i_vv;
	Index *ii                    = i_ii;

	const unsigned numVerts = m_vertexArray.size ();
	const Vector *verts = &m_vertexArray[0];
	const Rectangle2d clipRegion = m_clipRegion;

	for (unsigned i = 0; i < numVerts; ++i, ++vv)
	{
		Vector position = verts[i];

		////////////////////////////////////
		// clip to clip region
		if (position.x<m_clipRegion.x0)
		{
			position.x=m_clipRegion.x0;
		}
		else if (position.x>m_clipRegion.x1)
		{
			position.x=m_clipRegion.x1;
		}

		if (position.z<m_clipRegion.y0)
		{
			position.z=m_clipRegion.y0;
		}
		else if (position.z>m_clipRegion.y1)
		{
			position.z=m_clipRegion.y1;
		}
		////////////////////////////////////

		const float turn = position.x + position.z + stime;
		const float du = cos (turn) * m_heightDelta;
		position.y += du;

		vv.setPosition (position);

		vv.setNormal(normal);

		const float u = -position.x * ooShaderSize;
		const float v = position.z * ooShaderSize;
		const float dv = sin(turn) * m_heightDelta;

		vv.setTextureCoordinates (0, u + du, v + dv);
	}
	{
		uint i;
		for (i = 0; i < m_indexArray.size (); ++i, ++ii)
			*ii = static_cast<Index> (m_indexArray [i]);
	}
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::LocalShaderPrimitive::_fillGraphicsBuffersVS(VertexBufferWriteIterator i_vv, Index *const i_ii) const
{
	VertexBufferWriteIterator vv = i_vv;
	Index *ii                    = i_ii;
	const unsigned numVerts = m_vertexArray.size ();
	const Vector *verts = &m_vertexArray[0];

	for (unsigned i = 0; i < numVerts; ++i, ++vv)
	{
		const Vector &position = verts[i];
		vv.setPosition(position);
	}
	{
		for (uint i = 0; i < m_indexArray.size (); ++i, ++ii)
			*ii = static_cast<Index> (m_indexArray [i]);
	}
}

//-------------------------------------------------------------------
static bool _plane_cull(Plane plane, const Vector *const i_vertices, const int numVertices)
{
	const Vector *v = i_vertices;
	const Vector *const vstop = i_vertices + numVertices;
	while (v!=vstop)
	{
		if (plane.computeDistanceTo(*v)<0)
		{
			return false;
		}
		v++;
	}
	return true;
}

unsigned ClientGlobalWaterManager2::LocalShaderPrimitive::_getPotentiallyVisibleQuadrants(const Camera &i_camera) const
{
	const Vector *frustumVertices = i_camera.getFrustumVertices_w();
	const Volume &frustum_w = i_camera.getWorldFrustumVolume();
	const Vector referencePosition_w = i_camera.getPosition_w();
	const float xref = float(POS_QUANTIZE(referencePosition_w.x));
	const float zref = float(POS_QUANTIZE(referencePosition_w.z));

	if (_plane_cull(Plane(Vector(0, 1,0), -(m_height+m_heightDelta)), frustumVertices, Camera::FV_Max))
	{
		// view frustum is entirely above the top water plane
		return 0;
	}

	if (_plane_cull(Plane(Vector(0,-1,0),  (m_height-m_heightDelta)), frustumVertices, Camera::FV_Max))
	{
		// view frustum is entirely below the top water plane
		return 0;
	}

	Vector meshExtents[6];
	meshExtents[0].set(xref, m_height+m_heightDelta, zref);
	meshExtents[1].set(xref, m_height-m_heightDelta, zref);
	const float farPlane = i_camera.getFarPlane();

	const float oosqrt2 = .707106781188f;
	const Vector center(xref, 0, zref);

	unsigned retVal=0;

	/////////////////////////////////////////////////////////////////////////
	// +z wedge
	if (  !_plane_cull(Plane(Vector(oosqrt2,0,-oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(-oosqrt2,0,-oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(0,0,-1), center), frustumVertices, Camera::FV_Max)
		)
	{
		meshExtents[2].set(xref+farPlane, m_height+m_heightDelta, zref+farPlane);
		meshExtents[3].set(xref+farPlane, m_height-m_heightDelta, zref+farPlane); 
		meshExtents[4].set(xref-farPlane, m_height+m_heightDelta, zref+farPlane);
		meshExtents[5].set(xref-farPlane, m_height-m_heightDelta, zref+farPlane); 
		if (!frustum_w.fastConservativeExcludes(meshExtents, 6))
		{
			retVal|=Z_QUADRANT;
		}
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	// +x wedge
	if (  !_plane_cull(Plane(Vector(-oosqrt2,0, oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(-oosqrt2,0,-oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(-1,0,0), center), frustumVertices, Camera::FV_Max)
		)
	{
		meshExtents[2].set(xref+farPlane, m_height+m_heightDelta, zref+farPlane);
		meshExtents[3].set(xref+farPlane, m_height-m_heightDelta, zref+farPlane); 
		meshExtents[4].set(xref+farPlane, m_height+m_heightDelta, zref-farPlane);
		meshExtents[5].set(xref+farPlane, m_height-m_heightDelta, zref-farPlane); 
		if (!frustum_w.fastConservativeExcludes(meshExtents, 6))
		{
			retVal|=X_QUADRANT;
		}
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	// -z wedge
	if (  !_plane_cull(Plane(Vector(-oosqrt2,0, oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector( oosqrt2,0, oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(0,0,1), center), frustumVertices, Camera::FV_Max)
		)
	{
		meshExtents[2].set(xref+farPlane, m_height+m_heightDelta, zref-farPlane);
		meshExtents[3].set(xref+farPlane, m_height-m_heightDelta, zref-farPlane); 
		meshExtents[4].set(xref-farPlane, m_height+m_heightDelta, zref-farPlane);
		meshExtents[5].set(xref-farPlane, m_height-m_heightDelta, zref-farPlane); 
		if (!frustum_w.fastConservativeExcludes(meshExtents, 6))
		{
			retVal|=NZ_QUADRANT;
		}
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	// -x wedge
	if (  !_plane_cull(Plane(Vector( oosqrt2,0,-oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector( oosqrt2,0, oosqrt2), center), frustumVertices, Camera::FV_Max)
		&& !_plane_cull(Plane(Vector(1,0,0), center), frustumVertices, Camera::FV_Max)
		)
	{
		meshExtents[2].set(xref-farPlane, m_height+m_heightDelta, zref+farPlane);
		meshExtents[3].set(xref-farPlane, m_height-m_heightDelta, zref+farPlane); 
		meshExtents[4].set(xref-farPlane, m_height+m_heightDelta, zref-farPlane);
		meshExtents[5].set(xref-farPlane, m_height-m_heightDelta, zref-farPlane); 
		if (!frustum_w.fastConservativeExcludes(meshExtents, 6))
		{
			retVal|=NX_QUADRANT;
		}
	}
	/////////////////////////////////////////////////////////////////////////

	return retVal;
}

//-------------------------------------------------------------------

bool ClientGlobalWaterManager2::LocalShaderPrimitive::isPotentiallyVisible() const
{
	if (!m_usesClipRegion)
	{
		return true;
	}

	Vector meshExtents[8];
	meshExtents[0].set(m_clipRegion.x0, m_height+m_heightDelta, m_clipRegion.y0);
	meshExtents[1].set(m_clipRegion.x0, m_height-m_heightDelta, m_clipRegion.y0);
	meshExtents[2].set(m_clipRegion.x0, m_height+m_heightDelta, m_clipRegion.y1);
	meshExtents[3].set(m_clipRegion.x0, m_height-m_heightDelta, m_clipRegion.y1);
	meshExtents[4].set(m_clipRegion.x1, m_height+m_heightDelta, m_clipRegion.y1);
	meshExtents[5].set(m_clipRegion.x1, m_height-m_heightDelta, m_clipRegion.y1);
	meshExtents[6].set(m_clipRegion.x1, m_height+m_heightDelta, m_clipRegion.y0);
	meshExtents[7].set(m_clipRegion.x1, m_height-m_heightDelta, m_clipRegion.y0);

	const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();
	const Volume &frustum_w = camera.getWorldFrustumVolume();
	return !frustum_w.fastConservativeExcludes(meshExtents, 8);
}

//===================================================================

void ClientGlobalWaterManager2::LocalShaderPrimitive::_reset() const
{
	m_vertexArray.clear ();
	m_indexArray.clear ();
	m_isEmpty = true;
}

//===================================================================

void ClientGlobalWaterManager2::install ()
{
#ifdef _DEBUG
	DebugFlags::registerFlag (ms_noRenderWater, "ClientTerrain", "noRenderGlobalWaterManager2");
	DebugFlags::registerFlag (ms_reportClientGlobalWaterManager2, "ClientTerrain", "reportClientGlobalWaterManager2");
	DebugFlags::registerFlag (ms_renderGlobalWaterManager2Normals, "ClientTerrain", "renderGlobalWaterManager2Normals");
#endif

	ExitChain::add (remove, "ClientGlobalWaterManager2::remove");
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_noRenderWater);
	DebugFlags::unregisterFlag (ms_reportClientGlobalWaterManager2);
	DebugFlags::unregisterFlag (ms_renderGlobalWaterManager2Normals);
#endif
}

//===================================================================

ClientGlobalWaterManager2::ClientGlobalWaterManager2 (const Appearance& appearance, const float mapWidthInMeters, const char* const shaderTemplateName, const float shaderSize, const float height, Rectangle2d const * clipRegion) :
	WaterManager (),
	m_appearance (appearance),
	m_localShaderPrimitive(0)
{
	//UNREF(clipRegion);
	//Rectangle2d testClipRegion(-1988-5, -6399-5, -1988+5, -6399+5);

	bool usesVertexShader = Graphics::getShaderCapability() >= ShaderCapability(1,1);

	if (usesVertexShader)
	{
		m_localShaderPrimitive = new StaticShaderPrimitive (appearance, mapWidthInMeters, shaderTemplateName, shaderSize, height, clipRegion);
	}
	else
	{
		m_localShaderPrimitive = new DynamicShaderPrimitive (appearance, mapWidthInMeters, shaderTemplateName, shaderSize, height, clipRegion);
	}
}

//-------------------------------------------------------------------
	
ClientGlobalWaterManager2::~ClientGlobalWaterManager2 ()
{
	delete m_localShaderPrimitive;
}  //lint !e1740  //-- m_shader not freed or zero'ed

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::alter (const float elapsedTime)
{
	NOT_NULL (m_localShaderPrimitive);
	m_localShaderPrimitive->alter (elapsedTime);
}

//-------------------------------------------------------------------

void ClientGlobalWaterManager2::draw () const
{
#ifdef _DEBUG
	if (ms_noRenderWater)
		return;
#endif

	NOT_NULL (m_localShaderPrimitive);
	if (m_localShaderPrimitive->isPotentiallyVisible())
	{
		ShaderPrimitiveSorter::add (*m_localShaderPrimitive, ShaderPrimitiveSorter::getPhase (TAG_WATR));
	}
}

//===================================================================

