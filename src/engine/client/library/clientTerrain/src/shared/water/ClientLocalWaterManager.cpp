//===================================================================
//
// ClientLocalWaterManager.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientLocalWaterManager.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientTerrain/Triangulate.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Transform2d.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/FileName.h"

#include <algorithm>
#include <functional>
#include <vector>
#include <cstdio>

//===================================================================

namespace
{
	const Tag TAG_MAIN = TAG (M,A,I,N);
	const Tag TAG_WATR = TAG (W,A,T,R);
}

//===================================================================
//== START LocalShaderPrimitiveRibbonDefault
//===================================================================

class ClientLocalWaterManager::LocalShaderPrimitiveDefault : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveDefault (char const * debugName, const Appearance& appearance, const char* shaderTemplateName, float shaderSize, stdvector<Vector>::fwd const & vertices, stdvector<int>::fwd const & indices, const Vector2d& direction, float velocity, const Vector& normal, float maximumLength=0.f);
	LocalShaderPrimitiveDefault (char const * debugName, const Appearance& appearance, const char* shaderTemplateName, float shaderSize, stdvector<Vector>::fwd const & vertices, stdvector<int>::fwd const & indices, const Vector2d& direction, float velocity, const Vector& normal, const Vector& origin, float maximumLength=0.f);
	virtual ~LocalShaderPrimitiveDefault ();

	virtual float               alter (float time);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	const Sphere&               getSphere () const;
	virtual float getRadius() const { return getSphere().getRadius(); };

private:

	LocalShaderPrimitiveDefault ();
	LocalShaderPrimitiveDefault (const LocalShaderPrimitiveDefault& rhs);             
	LocalShaderPrimitiveDefault& operator= (const LocalShaderPrimitiveDefault& rhs);  

private:

	const Appearance&     m_appearance;
	Sphere                m_sphere;

	Shader*               m_shader;
	const bool            m_usesVertexShader;
	StaticVertexBuffer*   m_vertexBuffer;
	StaticIndexBuffer*    m_indexBuffer;
	float                 m_shaderSize;
	Vector2d              m_direction;
	float                 m_velocity;
	float                 m_time;
};

//-------------------------------------------------------------------
ClientLocalWaterManager::LocalShaderPrimitiveDefault::LocalShaderPrimitiveDefault (char const * debugName, Appearance const & appearance, char const * const shaderTemplateName, float const shaderSize, std::vector<Vector> const & vertices, std::vector<int> const & indices, Vector2d const & direction, float const velocity, Vector const & /*normal*/, float const maximumLength) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_sphere (),
	m_shader (0),
	m_usesVertexShader(Graphics::getShaderCapability() >= ShaderCapability(1,1)),
	m_vertexBuffer (0),
	m_indexBuffer (0),
	m_shaderSize (shaderSize),
	m_direction (direction),
	m_velocity (velocity),
	m_time(0.f)
{
	//-- recenter
	BoxExtent extent;
	extent.setMin (Vector::maxXYZ);
	extent.setMax (Vector::negativeMaxXYZ);

	{
		for (uint i = 0; i < vertices.size (); ++i)
			extent.updateMinAndMax(vertices[i]);
	}

	extent.calculateCenterAndRadius ();
	m_sphere = extent.getSphere ();

	bool clockwiseWinding = true;
	if (indices.size () >= 3)
	{
		Vector const & v0 = vertices [0];
		Vector const & v1 = vertices [1];
		Vector const & v2 = vertices [2];
		if ((v0 - v2).cross (v1 - v0).y < 0.f)
			clockwiseWinding = false;
	}

	//-- create vertexbuffer
	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();
	std::vector<Vector> texCoords;
	if (m_usesVertexShader)
	{
		Vector origin = vertices[0];

		uint i;
		for (i = 1; i < vertices.size(); ++i)
		{
			origin += vertices[i];
		}

		origin /= static_cast<float>(vertices.size());

		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 3);

		texCoords.reserve( vertices.size() );

		Vector tc(0,0,0);
		float theta = direction.theta();
		for (i = 0; i < vertices.size(); ++i)
		{
			Vector2d tc2d(vertices[i].x - origin.x, vertices[i].z - origin.z);
			tc2d.rotate(theta);

			tc.x = tc2d.x;
			tc.z = tc2d.y;
			tc.y = 0.07f * origin.magnitudeBetween(vertices[i]);

			texCoords.push_back( tc );
		}
	}
	else
	{
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
	}

	m_vertexBuffer = new StaticVertexBuffer (format, static_cast<int> (vertices.size ()));

		m_vertexBuffer->lock ();

		{
			float const ooShaderSize = m_shaderSize != 0.f ? (1.0f / m_shaderSize) : 1.f;

			VertexBufferWriteIterator vi = m_vertexBuffer->begin();

			uint const numberOfVertices = vertices.size ();
			for (uint i = 0; i < numberOfVertices; ++i, ++vi)
			{
				Vector const & position = vertices [i];

				vi.setPosition (position);
				vi.setNormal (Vector::unitY);
				if (m_usesVertexShader)
				{
					vi.setTextureCoordinates (0, texCoords[i]);
				}
				else
				{

					Vector2d const point (position.x * ooShaderSize, position.z * ooShaderSize);
					vi.setTextureCoordinates (0, point.x - m_sphere.getCenter ().x, point.y - m_sphere.getCenter ().z);
				}
			}
		}

		m_vertexBuffer->unlock ();

	//-- create indexbuffer
	m_indexBuffer = new StaticIndexBuffer (static_cast<int> (indices.size ()));

		m_indexBuffer->lock ();

		{
			Index *ii = m_indexBuffer->begin();

			const int numberOfIndices = static_cast<int> (indices.size ());

			int i;
			for (i = 0; i < numberOfIndices / 3; ++i)
			{
				if (clockwiseWinding)
				{
					//-- add top faces
					ii [i * 3 + 0] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 0)]);
					ii [i * 3 + 1] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 1)]);
					ii [i * 3 + 2] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 2)]);
				}
				else
				{
					//-- add top faces
					ii [i * 3 + 0] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 0)]);
					ii [i * 3 + 1] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 2)]);
					ii [i * 3 + 2] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 1)]);
				}
			}
		}

		m_indexBuffer->unlock ();

	//-- create shader
	m_shader = ShaderTemplateList::fetchModifiableShader (shaderTemplateName);

	//--
	UNREF (debugName);
	UNREF (maximumLength);
	DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportLogPrint (), ("ClientLocalWaterManager: [%s] s=%s ml=%1.2f v=%i t=%i w=%1.2f l=%1.2f c=<%1.2f, %1.2f, %1.2f> r=%1.2f\n", debugName ? debugName : "null", shaderTemplateName, maximumLength, m_vertexBuffer->getNumberOfVertices (), m_indexBuffer->getNumberOfIndices () / 3, extent.getWidth(), extent.getLength(), m_sphere.getCenter ().x, m_sphere.getCenter ().y, m_sphere.getCenter ().z, m_sphere.getRadius ()));
}

//-------------------------------------------------------------------

//-------------------------------------------------------------------
ClientLocalWaterManager::LocalShaderPrimitiveDefault::LocalShaderPrimitiveDefault (char const * debugName, Appearance const & appearance, char const * const shaderTemplateName, float const shaderSize, std::vector<Vector> const & vertices, std::vector<int> const & indices, Vector2d const & direction, float const velocity, Vector const & /*normal*/, Vector const& origin, float const maximumLength) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_sphere (),
	m_shader (0),
	m_usesVertexShader(Graphics::getShaderCapability() >= ShaderCapability(1,1)),
	m_vertexBuffer (0),
	m_indexBuffer (0),
	m_shaderSize (shaderSize),
	m_direction (direction),
	m_velocity (velocity),
	m_time(0.f)
{
	//-- recenter
	BoxExtent extent;
	extent.setMin (Vector::maxXYZ);
	extent.setMax (Vector::negativeMaxXYZ);

	{
		for (uint i = 0; i < vertices.size (); ++i)
			extent.updateMinAndMax(vertices[i]);
	}

	extent.calculateCenterAndRadius ();
	m_sphere = extent.getSphere ();

	bool clockwiseWinding = true;
	if (indices.size () >= 3)
	{
		Vector const & v0 = vertices [0];
		Vector const & v1 = vertices [1];
		Vector const & v2 = vertices [2];
		if ((v0 - v2).cross (v1 - v0).y < 0.f)
			clockwiseWinding = false;
	}

	//-- create vertexbuffer
	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();
	std::vector<Vector> texCoords;
	if (m_usesVertexShader)
	{
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 3);

		texCoords.reserve( vertices.size() );

		Vector tc(0,0,0);
		float theta = direction.theta();
		for (uint i = 0; i < vertices.size(); ++i)
		{
			Vector2d tc2d(vertices[i].x - origin.x, vertices[i].z - origin.z);
			tc2d.rotate(theta);

			tc.x = tc2d.x;
			tc.z = tc2d.y;
			tc.y = 0.07f * tc.z;

			texCoords.push_back( tc );
		}
	}
	else
	{
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
	}

	m_vertexBuffer = new StaticVertexBuffer (format, static_cast<int> (vertices.size ()));

		m_vertexBuffer->lock ();

		{
			float const ooShaderSize = m_shaderSize != 0.f ? (1.0f / m_shaderSize) : 1.f;

			VertexBufferWriteIterator vi = m_vertexBuffer->begin();

			uint const numberOfVertices = vertices.size ();
			for (uint i = 0; i < numberOfVertices; ++i, ++vi)
			{
				Vector const & position = vertices [i];

				vi.setPosition (position);
				vi.setNormal (Vector::unitY);
				if (m_usesVertexShader)
				{
					vi.setTextureCoordinates (0, texCoords[i]);
				}
				else
				{

					Vector2d const point (position.x * ooShaderSize, position.z * ooShaderSize);
					vi.setTextureCoordinates (0, point.x - m_sphere.getCenter ().x, point.y - m_sphere.getCenter ().z);
				}
			}
		}

		m_vertexBuffer->unlock ();

	//-- create indexbuffer
	m_indexBuffer = new StaticIndexBuffer (static_cast<int> (indices.size ()));

		m_indexBuffer->lock ();

		{
			Index *ii = m_indexBuffer->begin();

			const int numberOfIndices = static_cast<int> (indices.size ());

			int i;
			for (i = 0; i < numberOfIndices / 3; ++i)
			{
				if (clockwiseWinding)
				{
					//-- add top faces
					ii [i * 3 + 0] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 0)]);
					ii [i * 3 + 1] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 1)]);
					ii [i * 3 + 2] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 2)]);
				}
				else
				{
					//-- add top faces
					ii [i * 3 + 0] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 0)]);
					ii [i * 3 + 1] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 2)]);
					ii [i * 3 + 2] = static_cast<Index> (indices [static_cast<uint> (i * 3 + 1)]);
				}
			}
		}

		m_indexBuffer->unlock ();

	//-- create shader
	m_shader = ShaderTemplateList::fetchModifiableShader (shaderTemplateName);

	//--
	UNREF (debugName);
	UNREF (maximumLength);
	DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportLogPrint (), ("ClientLocalWaterManager: [%s] s=%s ml=%1.2f v=%i t=%i w=%1.2f l=%1.2f c=<%1.2f, %1.2f, %1.2f> r=%1.2f\n", debugName ? debugName : "null", shaderTemplateName, maximumLength, m_vertexBuffer->getNumberOfVertices (), m_indexBuffer->getNumberOfIndices () / 3, extent.getWidth(), extent.getLength(), m_sphere.getCenter ().x, m_sphere.getCenter ().y, m_sphere.getCenter ().z, m_sphere.getRadius ()));
}

//-------------------------------------------------------------------

ClientLocalWaterManager::LocalShaderPrimitiveDefault::~LocalShaderPrimitiveDefault ()
{
	m_shader->release ();
	m_shader = 0;

	delete m_vertexBuffer;
	m_vertexBuffer = 0;

	delete m_indexBuffer;
	m_indexBuffer = 0;
}

//-------------------------------------------------------------------

const Sphere& ClientLocalWaterManager::LocalShaderPrimitiveDefault::getSphere () const
{
	return m_sphere;
}

//-------------------------------------------------------------------

float ClientLocalWaterManager::LocalShaderPrimitiveDefault::alter (float time)
{
	m_time += time;
	return m_shader->alter (time);
}

// ----------------------------------------------------------------------

const Vector ClientLocalWaterManager::LocalShaderPrimitiveDefault::getPosition_w() const
{
	return m_sphere.getCenter ();
}
	
//-------------------------------------------------------------------

float ClientLocalWaterManager::LocalShaderPrimitiveDefault::getDepthSquaredSortKey() const
{
	return -getPosition_w ().y;
}

// ----------------------------------------------------------------------

int ClientLocalWaterManager::LocalShaderPrimitiveDefault::getVertexBufferSortKey() const
{
	return m_vertexBuffer->getSortKey();
}

//-------------------------------------------------------------------

const StaticShader& ClientLocalWaterManager::LocalShaderPrimitiveDefault::prepareToView () const
{
	return m_shader->prepareToView ();
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::LocalShaderPrimitiveDefault::prepareToDraw () const
{

	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (*m_vertexBuffer);
	Graphics::setIndexBuffer (*m_indexBuffer);
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::LocalShaderPrimitiveDefault::draw () const
{
	if (m_usesVertexShader)
	{
		const float stime = float(m_time * 2.0);

		Graphics::setVertexShaderUserConstants(0, 1.0f, stime, 1.0f / m_shaderSize, 0.0f);
		Graphics::setVertexShaderUserConstants(1, m_sphere.getCenter().x, m_sphere.getCenter().y, m_sphere.getCenter().z, 0.0f);
		Graphics::setVertexShaderUserConstants(2, 0.0f, 0.0f, 0.0f, 0.0f);
		Graphics::setVertexShaderUserConstants(3, -65536.0f, -65536.0f, -65536.0f, -65536.0f);
		Graphics::setVertexShaderUserConstants(4, 65536.0f,  65536.0f, 65536.0f,  65536.0f);
	}

	const GlCullMode cullMode = Graphics::getCullMode ();
	if (ShaderPrimitiveSorter::getCurrentCamera().isUnderWater())
		Graphics::setCullMode (GCM_clockwise);
	else
		Graphics::setCullMode (GCM_counterClockwise);

	Graphics::drawIndexedTriangleList ();
	Graphics::setCullMode (cullMode);
}


//===================================================================

class LocalShaderPrimitiveDefault_DrawFunctor
{
public:

	void operator () (const ClientLocalWaterManager::LocalShaderPrimitiveDefault* localShaderPrimitiveDefault) const
	{
		if (ShaderPrimitiveSorter::getCurrentCamera ().testVisibility_w (localShaderPrimitiveDefault->getSphere ()))
			ShaderPrimitiveSorter::add (*localShaderPrimitiveDefault, ShaderPrimitiveSorter::getPhase (TAG_WATR));
	}
};


//===================================================================
//== END LocalShaderPrimitiveRibbonDefault
//== START LocalShaderPrimitiveRibbonStrip
//===================================================================

class ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveRibbonStrip (char const * debugName, const Appearance& appearance, const char* shaderTemplateName, float shaderSize, stdvector<Vector>::fwd const & vertices, stdvector<int>::fwd const & indices, stdvector<Vector>::fwd const & texCoords, float velocity);
	virtual ~LocalShaderPrimitiveRibbonStrip ();

	virtual float               alter (float time);
	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

	const Sphere&               getSphere () const;
	virtual float getRadius() const { return getSphere().getRadius();}

private:

	LocalShaderPrimitiveRibbonStrip ();
	LocalShaderPrimitiveRibbonStrip (const LocalShaderPrimitiveRibbonStrip& rhs);             
	LocalShaderPrimitiveRibbonStrip& operator= (const LocalShaderPrimitiveRibbonStrip& rhs);  

private:

	const Appearance&     m_appearance;
	Sphere                m_sphere;

	Shader*               m_shader;
	const bool            m_usesVertexShader;
	StaticVertexBuffer*   m_vertexBuffer;
	StaticIndexBuffer*    m_indexBuffer;
	float                 m_shaderSize;
	float                 m_velocity;
	float                 m_time;

};

//-------------------------------------------------------------------

ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::LocalShaderPrimitiveRibbonStrip (char const * debugName, Appearance const & appearance, char const * const shaderTemplateName, float const shaderSize, std::vector<Vector> const & vertices, std::vector<int> const & indices, std::vector<Vector> const & texCoords, float const velocity) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_sphere (),
	m_shader (0),
	m_usesVertexShader(Graphics::getShaderCapability() >= ShaderCapability(1,1)),
	m_vertexBuffer (0),
	m_indexBuffer (0),
	m_shaderSize (shaderSize),
	m_velocity (velocity),
	m_time(0.f)
{
	//-- recenter
	BoxExtent extent;
	extent.setMin (Vector::maxXYZ);
	extent.setMax (Vector::negativeMaxXYZ);

	{
		for (uint i = 0; i < vertices.size (); ++i)
			extent.updateMinAndMax(vertices[i]);
	}

	extent.calculateCenterAndRadius ();
	m_sphere = extent.getSphere ();

	bool clockwiseWinding = true;
	if (indices.size () >= 3)
	{
		Vector const & v0 = vertices [0];
		Vector const & v1 = vertices [1];
		Vector const & v2 = vertices [2];
		if ((v0 - v2).cross (v1 - v0).y < 0.f)
			clockwiseWinding = false;
	}

	//-- create vertexbuffer
	VertexBufferFormat format;
	format.setPosition();
	format.setNormal();

	if (m_usesVertexShader)
	{
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 3);
	}
	else
	{
		format.setNumberOfTextureCoordinateSets(1);
		format.setTextureCoordinateSetDimension(0, 2);
	}

	m_vertexBuffer = new StaticVertexBuffer (format, static_cast<int> (vertices.size ()));

		m_vertexBuffer->lock ();

		{
			float const ooShaderSize = m_shaderSize != 0.f ? (1.0f / m_shaderSize) : 1.f;

			VertexBufferWriteIterator vi = m_vertexBuffer->begin();

			uint const numberOfVertices = vertices.size ();
			for (uint i = 0; i < numberOfVertices; ++i, ++vi)
			{
				Vector const & position = vertices [i];

				vi.setPosition (position);
				vi.setNormal (Vector::unitY);
				if (m_usesVertexShader)
				{
					vi.setTextureCoordinates (0, texCoords[i]);
				}
				else
				{
					Vector2d const point (position.x * ooShaderSize, position.z * ooShaderSize);
					vi.setTextureCoordinates (0, point.x - m_sphere.getCenter ().x, point.y - m_sphere.getCenter ().z);
				}
			}
		}

		m_vertexBuffer->unlock ();

	//-- create indexbuffer
	m_indexBuffer = new StaticIndexBuffer (static_cast<int> (indices.size ()));

		m_indexBuffer->lock ();

		{
			Index *ii = m_indexBuffer->begin();
			const int numberOfIndices = static_cast<int> (indices.size ());	
			int i;
			for(i = 0; i < numberOfIndices; ++i)
			{
				ii[i] = static_cast<Index> (indices[i]);
			}
		}



		m_indexBuffer->unlock ();

	//-- create shader
	m_shader = ShaderTemplateList::fetchModifiableShader (shaderTemplateName);

	//--
	UNREF (debugName);

	DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportLogPrint (), ("ClientLocalWaterManager: [%s] s=%s v=%i t=%i w=%1.2f l=%1.2f c=<%1.2f, %1.2f, %1.2f> r=%1.2f\n", debugName ? debugName : "null", shaderTemplateName, m_vertexBuffer->getNumberOfVertices (), m_indexBuffer->getNumberOfIndices () / 3, extent.getWidth(), extent.getLength(), m_sphere.getCenter ().x, m_sphere.getCenter ().y, m_sphere.getCenter ().z, m_sphere.getRadius ()));
}

//-------------------------------------------------------------------

ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::~LocalShaderPrimitiveRibbonStrip ()
{
	m_shader->release ();
	m_shader = 0;

	delete m_vertexBuffer;
	m_vertexBuffer = 0;

	delete m_indexBuffer;
	m_indexBuffer = 0;
}

//-------------------------------------------------------------------

const Sphere& ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::getSphere () const
{
	return m_sphere;
}

//-------------------------------------------------------------------

float ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::alter (float time)
{
	m_time += time;
	return m_shader->alter (time);
}

// ----------------------------------------------------------------------

const Vector ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::getPosition_w() const
{
	return m_sphere.getCenter ();
}
	
//-------------------------------------------------------------------

float ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::getDepthSquaredSortKey() const
{
	return -getPosition_w ().y;
}

// ----------------------------------------------------------------------

int ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::getVertexBufferSortKey() const
{
	return m_vertexBuffer->getSortKey();
}

//-------------------------------------------------------------------

const StaticShader& ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::prepareToView () const
{
	return m_shader->prepareToView ();
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::prepareToDraw () const
{

	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);
	Graphics::setVertexBuffer (*m_vertexBuffer);
	Graphics::setIndexBuffer (*m_indexBuffer);
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip::draw () const
{
	if (m_usesVertexShader)
	{
		const float stime = float(m_time * 2.0);

		Graphics::setVertexShaderUserConstants(0, 1.0f, stime, 1.0f / m_shaderSize, 0.0f);
		Graphics::setVertexShaderUserConstants(1, m_sphere.getCenter().x, m_sphere.getCenter().y, m_sphere.getCenter().z, 0.0f);
		Graphics::setVertexShaderUserConstants(2, 0.0f, 0.0f, 0.0f, 0.0f);
		Graphics::setVertexShaderUserConstants(3, -65536.0f, -65536.0f, -65536.0f, -65536.0f);
		Graphics::setVertexShaderUserConstants(4, 65536.0f,  65536.0f, 65536.0f,  65536.0f);
	}

	const GlCullMode cullMode = Graphics::getCullMode ();
	if (ShaderPrimitiveSorter::getCurrentCamera().isUnderWater())
		Graphics::setCullMode (GCM_clockwise);
	else
		Graphics::setCullMode (GCM_counterClockwise);

	Graphics::drawIndexedTriangleStrip ();
	Graphics::setCullMode (cullMode);
}


//===================================================================

class LocalShaderPrimitiveRibbonStrip_DrawFunctor
{
public:

	void operator () (const ClientLocalWaterManager::LocalShaderPrimitiveRibbonStrip* localShaderPrimitiveRibbonStrip) const
	{
		if (ShaderPrimitiveSorter::getCurrentCamera ().testVisibility_w (localShaderPrimitiveRibbonStrip->getSphere ()))
			ShaderPrimitiveSorter::add (*localShaderPrimitiveRibbonStrip, ShaderPrimitiveSorter::getPhase (TAG_WATR));
	}
};

//===================================================================
//== END LocalShaderPrimitiveRibbonStrip
//===================================================================

#ifdef _DEBUG
namespace ClientLocalWaterManagerNamespace
{
	static bool ms_noRenderWater;
}

using namespace ClientLocalWaterManagerNamespace;
#endif

//===================================================================

ClientLocalWaterManager::ClientLocalWaterManager (const Appearance& appearance) :
	WaterManager (),
	m_appearance (appearance),
	m_localShaderPrimitiveDefaultList (0),
	m_localShaderPrimitiveRibbonStripList (0)
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_noRenderWater, "ClientTerrain", "noRenderLocalWater");
#endif

	m_localShaderPrimitiveDefaultList = NON_NULL (new LocalShaderPrimitiveDefaultList);
	m_localShaderPrimitiveRibbonStripList = NON_NULL (new LocalShaderPrimitiveRibbonStripList);
}

//-------------------------------------------------------------------

ClientLocalWaterManager::~ClientLocalWaterManager ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag(ms_noRenderWater);
#endif

	std::for_each (m_localShaderPrimitiveDefaultList->begin (), m_localShaderPrimitiveDefaultList->end (), PointerDeleter ());
	m_localShaderPrimitiveDefaultList->clear ();

	delete m_localShaderPrimitiveDefaultList;
	m_localShaderPrimitiveDefaultList = 0;

	std::for_each (m_localShaderPrimitiveRibbonStripList->begin (), m_localShaderPrimitiveRibbonStripList->end (), PointerDeleter ());
	m_localShaderPrimitiveRibbonStripList->clear ();

	delete m_localShaderPrimitiveRibbonStripList;
	m_localShaderPrimitiveRibbonStripList = 0;
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::addWater (char const * debugName, const char* shaderTemplateName, float shaderSize, const std::vector<Vector>& vertices, const Vector2d& direction, float velocity)
{
	static std::vector<int> indices;
	if (indices.empty())
	{
		indices.reserve(6);
		indices.push_back (0);
		indices.push_back (1);
		indices.push_back (2);
		indices.push_back (0);
		indices.push_back (2);
		indices.push_back (3);
	}

	//-- calculate normals
	const Vector normal = (vertices [static_cast<uint> (indices [0])] - vertices [static_cast<uint> (indices [2])]).cross (vertices [static_cast<uint> (indices [1])] - vertices [static_cast<uint> (indices [0])]);

	m_localShaderPrimitiveDefaultList->push_back (new LocalShaderPrimitiveDefault (debugName, m_appearance, shaderTemplateName, shaderSize, vertices, indices, direction, velocity, normal));
}

//-------------------------------------------------------------------

static void retriangulate (std::vector<Vector>& vertices, std::vector<int>& indices, float maximumLength);
static int findOrAdd (std::vector<Vector>& vertices, const Vector& vector);

//----------------------------------------------------------------------

void ClientLocalWaterManager::addWater (char const * debugName, const char* shaderTemplateName, float shaderSize, const std::vector<Vector2d>& pointList, float height, const Vector2d& direction, float velocity)
{
	if (!shaderTemplateName)
	{
		DEBUG_FATAL (true, ("shaderTemplateName == 0"));
		return;  //lint !e527  //-- unreachable
	}

	UNREF (shaderSize);

	if (pointList.size () < 3)
		return;

#if 1
	float maximumLength = 1024.f;

	if (Graphics::getShaderCapability () >= ShaderCapability (1, 1))
	{
		Rectangle2d extent;
		extent.x0 = FLT_MAX;
		extent.y0 = FLT_MAX;
		extent.x1 = -FLT_MAX;
		extent.y1 = -FLT_MAX;

		for (uint i = 0; i < pointList.size (); ++i)
			extent.expand (pointList [i]);

		maximumLength = clamp (2.f, extent.getCenter ().magnitudeBetween (Vector2d (extent.x0, extent.y0)) / 16.f, 64.f);
	}
#else
	float maximumLength = 1024.f;
	
	if (Graphics::getShaderCapability () >= ShaderCapability (2, 0))
		maximumLength = 32.f;
	else
		if (Graphics::getShaderCapability () >= ShaderCapability (1, 1))
			maximumLength = 64.f;
#endif

	//-- triangulate
	std::vector<int> indices;
	IGNORE_RETURN (Triangulate::Process (pointList, indices));

	//-- re-trianglulate again to ensure no polygons have edges larger than 512m
	std::vector<Vector> vertices;
	{
		//-- create initial vertices
		{
			const uint n = pointList.size ();
			uint i;
			for (i = 0; i < n; ++i)
				vertices.push_back (Vector (pointList [i].x, height, pointList [i].y));
		}

		//-- go through all triangles and make sure no sides are > 64m
		retriangulate (vertices, indices, maximumLength);
	}

	m_localShaderPrimitiveDefaultList->push_back (new LocalShaderPrimitiveDefault (debugName, m_appearance, shaderTemplateName, shaderSize, vertices, indices, direction, velocity, Vector::xyz111, maximumLength));
}

//----------------------------------------------------------------------

void ClientLocalWaterManager::addRibbonEndCap (char const * debugName, const char* shaderTemplateName, float shaderSize, const std::vector<Vector2d>& pointList, float height, const Vector2d& direction, float velocity, const Vector& origin)
{
	if (!shaderTemplateName)
	{
		DEBUG_FATAL (true, ("shaderTemplateName == 0"));
		return;  //lint !e527  //-- unreachable
	}

	UNREF (shaderSize);

	if (pointList.size () < 3)
		return;

	float maximumLength = 1024.f;

	if (Graphics::getShaderCapability () >= ShaderCapability (1, 1))
	{
		Rectangle2d extent;
		extent.x0 = FLT_MAX;
		extent.y0 = FLT_MAX;
		extent.x1 = -FLT_MAX;
		extent.y1 = -FLT_MAX;

		for (uint i = 0; i < pointList.size (); ++i)
			extent.expand (pointList [i]);

		maximumLength = clamp (2.f, extent.getCenter ().magnitudeBetween (Vector2d (extent.x0, extent.y0)) / 16.f, 64.f);
	}

	//-- triangulate
	std::vector<int> indices;
	IGNORE_RETURN (Triangulate::Process (pointList, indices));

	//-- re-trianglulate again to ensure no polygons have edges larger than 512m
	std::vector<Vector> vertices;
	{
		//-- create initial vertices
		{
			const uint n = pointList.size ();
			uint i;
			for (i = 0; i < n; ++i)
				vertices.push_back (Vector (pointList [i].x, height, pointList [i].y));
		}

		//-- go through all triangles and make sure no sides are > 64m
		retriangulate (vertices, indices, maximumLength);
	}

	m_localShaderPrimitiveDefaultList->push_back (new LocalShaderPrimitiveDefault (debugName, m_appearance, shaderTemplateName, shaderSize, vertices, indices, direction, velocity, Vector::xyz111, origin, maximumLength));
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::addRibbonStrip (char const * debugName, const char* shaderTemplateName, float shaderSize, const std::vector<Vector>& pointList,  const std::vector<Vector>& texCoords, float velocity)
{
	const uint n = pointList.size ();
	std::vector<int> indices(n);
	uint i;
	for (i = 0; i < n; ++i)
	{
		indices[i] = i;
	}

	m_localShaderPrimitiveRibbonStripList->push_back (new LocalShaderPrimitiveRibbonStrip (debugName, m_appearance, shaderTemplateName, shaderSize, pointList, indices, texCoords, velocity));
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::alter (float time)
{
	NOT_NULL (m_localShaderPrimitiveDefaultList);
	std::for_each (m_localShaderPrimitiveDefaultList->begin (), m_localShaderPrimitiveDefaultList->end (), std::bind2nd (std::mem_fun (&LocalShaderPrimitiveDefault::alter), time));

	NOT_NULL (m_localShaderPrimitiveRibbonStripList);
	std::for_each (m_localShaderPrimitiveRibbonStripList->begin (), m_localShaderPrimitiveRibbonStripList->end (), std::bind2nd (std::mem_fun (&LocalShaderPrimitiveRibbonStrip::alter), time));
}

//-------------------------------------------------------------------

void ClientLocalWaterManager::draw () const
{
#ifdef _DEBUG
	if (ms_noRenderWater)
		return;
#endif

	NOT_NULL (m_localShaderPrimitiveDefaultList);
	std::for_each (m_localShaderPrimitiveDefaultList->begin (), m_localShaderPrimitiveDefaultList->end (), LocalShaderPrimitiveDefault_DrawFunctor ());

	NOT_NULL (m_localShaderPrimitiveRibbonStripList);
	std::for_each (m_localShaderPrimitiveRibbonStripList->begin (), m_localShaderPrimitiveRibbonStripList->end (), LocalShaderPrimitiveRibbonStrip_DrawFunctor ());
}

//===================================================================

static int findOrAdd (std::vector<Vector>& vertices, const Vector& vector)
{
	uint const numberOfVertices = vertices.size ();
	for (uint i = 0; i < numberOfVertices; i++)
		if (vertices [i] == vector)
			return static_cast<int> (i);

	vertices.push_back (vector);

	return static_cast<int> (numberOfVertices);
}

//-------------------------------------------------------------------

static void retriangulate (std::vector<Vector>& vertices, std::vector<int>& indices, float maximumLength)
{
	enum Split
	{
		S_none,
		S_0_1,
		S_1_2,
		S_2_0
	};

	std::vector<bool> faceList;
	{
		faceList.reserve (indices.size () / 3);

		uint i;
		for (i = 0; i < indices.size () / 3; ++i)
			faceList.push_back (true);
	}

	uint faceIndex = 0;
	while (faceIndex < (indices.size () / 3))
	{
		//-- check the 3 sides
		const int i0 = indices [faceIndex * 3 + 0];
		const int i1 = indices [faceIndex * 3 + 1];
		const int i2 = indices [faceIndex * 3 + 2];

		Vector const & v0 = vertices [static_cast<unsigned int> (i0)];
		Vector const & v1 = vertices [static_cast<unsigned int> (i1)];
		Vector const & v2 = vertices [static_cast<unsigned int> (i2)];

		Split split = S_none;

		float maximumLengthSquared = sqr (maximumLength);

		const float magnitudeBetweenSquared_01 = v0.magnitudeBetweenSquared (v1);
		if (magnitudeBetweenSquared_01 > maximumLengthSquared)
		{
			maximumLengthSquared = magnitudeBetweenSquared_01;
			split = S_0_1;
		}

		const float magnitudeBetweenSquared_12 = v1.magnitudeBetweenSquared (v2);
		if (magnitudeBetweenSquared_12 > maximumLengthSquared)
		{
			maximumLengthSquared = magnitudeBetweenSquared_12;
			split = S_1_2;
		}

		const float magnitudeBetweenSquared_20 = v2.magnitudeBetweenSquared (v0);
		if (magnitudeBetweenSquared_20 > maximumLengthSquared)
		{
			maximumLengthSquared = magnitudeBetweenSquared_20;
			split = S_2_0;
		}

		switch (split)
		{
		case S_0_1:
			{
				//
				//-- split between v0 and v1
				//

				//-- add new vertex
				const int newIndex = static_cast<int> (vertices.size ());

				vertices.push_back (Vector::midpoint (v0, v1));

				//-- add new face (0, new, 2)
				indices.push_back (i0);
				indices.push_back (newIndex);
				indices.push_back (i2);
				faceList.push_back (true);

				//-- add new face (new, 1, 2)
				indices.push_back (newIndex);
				indices.push_back (i1);
				indices.push_back (i2);
				faceList.push_back (true);

				//-- remove 0, 1, 2 from indices
				faceList [faceIndex] = false;
			}
			break;

		case S_1_2:
			{
				//
				//-- split between v1 and v2
				//

				//-- add new vertex
				const int newIndex = static_cast<int> (vertices.size ());

				vertices.push_back (Vector::midpoint (v1, v2));

				//-- add new face (0, 1, new)
				indices.push_back (i0);
				indices.push_back (i1);
				indices.push_back (newIndex);
				faceList.push_back (true);

				//-- add new face (0, new, 2)
				indices.push_back (i0);
				indices.push_back (newIndex);
				indices.push_back (i2);
				faceList.push_back (true);

				//-- remove 0, 1, 2 from indices
				faceList [faceIndex] = false;
			}
			break;

		case S_2_0:
			{
				//
				//-- split between v2 and v0
				//

				//-- add new vertex
				const int newIndex = static_cast<int> (vertices.size ());

				vertices.push_back (Vector::midpoint (v2, v0));

				//-- add new face (0, 1, new)
				indices.push_back (i0);
				indices.push_back (i1);
				indices.push_back (newIndex);
				faceList.push_back (true);

				//-- add new face (new, 1, 2)
				indices.push_back (newIndex);
				indices.push_back (i1);
				indices.push_back (i2);
				faceList.push_back (true);

				//-- remove 0, 1, 2 from indices
				faceList [faceIndex] = false;
			}
			break;

		case S_none:
			break;
		}

		++faceIndex;
	}

	//-- crush size
	{
		std::vector<int>    compactIndices;
		std::vector<Vector> compactVertices;

		unsigned int i;
		for (i = 0; i < faceList.size (); ++i)
		{
			if (faceList [i])
			{
				compactIndices.push_back (findOrAdd (compactVertices, vertices [static_cast<unsigned int> (indices [i * 3 + 0])]));
				compactIndices.push_back (findOrAdd (compactVertices, vertices [static_cast<unsigned int> (indices [i * 3 + 1])]));
				compactIndices.push_back (findOrAdd (compactVertices, vertices [static_cast<unsigned int> (indices [i * 3 + 2])]));
			}
		}

		vertices.swap (compactVertices);
		indices.swap (compactIndices);
	}
}

//===================================================================

